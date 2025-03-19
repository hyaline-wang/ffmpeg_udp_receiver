#include <iostream>
#include <string>
#include <fstream>  // Include this for std::ofstream
#include <opencv2/opencv.hpp> // Include OpenCV

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_video> <output_image_prefix>" << std::endl;
        return 1;
    }

    const char *in_filename = argv[1];
    const char *out_prefix = argv[2];

    av_register_all(); // Register all formats and codecs

    AVFormatContext *in_ctx = nullptr;
    if (avformat_open_input(&in_ctx, in_filename, nullptr, nullptr) < 0) {
        std::cerr << "Could not open input file '" << in_filename << "'" << std::endl;
        return 1;
    }

    if (avformat_find_stream_info(in_ctx, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        return 1;
    }

    AVCodec *codec = nullptr;
    int video_stream_index = -1;
    for (unsigned int i = 0; i < in_ctx->nb_streams; i++) {
        if (in_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            codec = avcodec_find_decoder(in_ctx->streams[i]->codecpar->codec_id);
            if (!codec) {
                std::cerr << "Could not find decoder for video stream" << std::endl;
                return 1;
            }else {
                std::cout << "Detected codec: " << codec->name << std::endl; // Add this line
            }
            break;
        }
    }

    if (video_stream_index == -1) {
        std::cerr << "Could not find video stream in input file" << std::endl;
        return 1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "Could not allocate codec context" << std::endl;
        return 1;
    }

    if (avcodec_parameters_to_context(codec_ctx, in_ctx->streams[video_stream_index]->codecpar) < 0) {
        std::cerr << "Could not copy codec parameters to decoder context" << std::endl;
        return 1;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return 1;
    }

    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Could not allocate video frame" << std::endl;
        return 1;
    }

    SwsContext *sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws_ctx) {
        std::cerr << "Could not create software scaling context" << std::endl;
        return 1;
    }

    AVFrame *rgb_frame = av_frame_alloc();
    if (!rgb_frame) {
        std::cerr << "Could not allocate RGB frame" << std::endl;
        return 1;
    }

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);
    uint8_t *buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height, 1);

    AVPacket packet;
    int frame_count = 0;
    while (av_read_frame(in_ctx, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) {
            int ret = avcodec_send_packet(codec_ctx, &packet);
            if (ret < 0) {
                std::cerr << "Error sending packet for decoding" << std::endl;
                continue; // Skip to the next packet
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break; // Need more data or decoder finished
                else if (ret < 0) {
                    std::cerr << "Error receiving frame from decoder" << std::endl;
                    break;
                }
                sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, rgb_frame->data, rgb_frame->linesize);
                // OpenCV display
                cv::Mat image(codec_ctx->height, codec_ctx->width, CV_8UC3, rgb_frame->data[0]); // Create OpenCV Mat
                cv::resize(image, image, cv::Size(640, 480)); // Resize the image
                cv::imshow("Decoded Frame", image); // Display the image
                cv::waitKey(1); // Wait for a short time (adjust as needed)
                // std::string out_filename = out_prefix + std::to_string(frame_count) + ".ppm";
                // std::ofstream outfile(out_filename, std::ios::binary);
                // outfile << "P6\n" << codec_ctx->width << " " << codec_ctx->height << "\n255\n";
                // for (int y = 0; y < codec_ctx->height; y++) {
                //     for (int x = 0; x < codec_ctx->width; x++) {
                //         outfile << (int)rgb_frame->data[0][y * rgb_frame->linesize[0] + x] << " "
                //                 << (int)rgb_frame->data[0][y * rgb_frame->linesize[0] + x + 1] << " "
                //                 << (int)rgb_frame->data[0][y * rgb_frame->linesize[0] + x + 2] << " ";
                //     }
                //     outfile << "\n";
                // }
                // outfile.close();

                frame_count++;
            }
        }
        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    av_free(buffer);
    sws_freeContext(sws_ctx);
    avcodec_close(codec_ctx);
    avformat_close_input(&in_ctx);

    return 0;
}