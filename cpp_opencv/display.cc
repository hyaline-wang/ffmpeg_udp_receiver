#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_video>" << std::endl;
        return 1;
    }

    const char *in_filename = argv[1];

    av_register_all();

    AVFormatContext *in_ctx = nullptr;
    if (avformat_open_input(&in_ctx, in_filename, nullptr, nullptr) < 0) {
        // ... (error handling as before)
    }

    // ... (find video stream and codec as before)

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    // ... (error handling and codec parameters to context as before)

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        // ... (error handling as before)
    }

    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        // ... (error handling as before)
    }

    SwsContext *sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, // Change to BGR24 for OpenCV
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws_ctx) {
        // ... (error handling as before)
    }

    AVFrame *bgr_frame = av_frame_alloc(); // Allocate BGR frame
    if (!bgr_frame) {
        // ... (error handling as before)
    }

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1); // BGR24 size
    uint8_t *buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(bgr_frame->data, bgr_frame->linesize, buffer, AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1); // Fill BGR frame

    AVPacket packet;
    while (av_read_frame(in_ctx, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) {
            // ... (packet sending and frame receiving as before)

            sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, bgr_frame->data, bgr_frame->linesize);

            // OpenCV display
            cv::Mat image(codec_ctx->height, codec_ctx->width, CV_8UC3, bgr_frame->data[0]); // Create OpenCV Mat
            cv::imshow("Decoded Frame", image); // Display the image
            cv::waitKey(1); // Wait for a short time (adjust as needed)

        }
        av_packet_unref(&packet);
    }

    // ... (free resources as before)

    return 0;
}