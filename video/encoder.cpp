#include "encoder.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
}

static const char* codec_name      = "libx264rgb";
static const AVPixelFormat pix_fmt = AV_PIX_FMT_BGR0;

AVCodecContext *VideoEncoder::allocContext(int width, int height, int framerate)
{
    const AVCodec *codec;
    AVCodecContext *ctx = nullptr;

    codec = avcodec_find_encoder_by_name(codec_name);
    if (codec == nullptr) {
        last_error = "Codec '" + std::string(codec_name) + "' not found";
        return nullptr;
    }

    ctx = avcodec_alloc_context3(codec);
    if (ctx == nullptr) {
        last_error = "Could not allocate video codec context";
        return nullptr;
    }

    ctx->bit_rate = 400000;
    ctx->width = width;
    ctx->height = height;
    ctx->time_base = {1, framerate};
    ctx->gop_size = 10;
    ctx->max_b_frames = 1;
    ctx->pix_fmt = pix_fmt;

    // Set optimal compression / speed ratio for this use-case
    av_opt_set(ctx->priv_data, "preset", "fast", 0);

    // Set codec to lossless
    av_opt_set(ctx->priv_data, "crf",    "0",    0);

    av_error = avcodec_open2(ctx, codec, nullptr);
    if (av_error < 0) {
        char ch[AV_ERROR_MAX_STRING_SIZE] = {0};
        last_error = "Could not open codec: '" + std::string(codec_name) + "' -> " +
                  av_make_error_string(ch, AV_ERROR_MAX_STRING_SIZE, av_error);
        avcodec_free_context(&ctx);
        return nullptr;
    }

    return ctx;
}

AVFrame *VideoEncoder::allocFrame()
{
    AVFrame *frame = av_frame_alloc();
    if (frame == nullptr) {
        last_error = "Could not allocate video frame";
        return nullptr;
    }

    frame->format = ctx->pix_fmt;
    frame->width  = ctx->width;
    frame->height = ctx->height;

    av_error = av_frame_get_buffer(frame, 32);
    if (av_error < 0) {
        last_error = "Could not allocate the video frame data";
        av_frame_free(&frame);
        return nullptr;
    }

    return frame;
}

void VideoEncoder::create(int width, int height, int frameRate, const char *fileName)
{
    av_log_set_level(AV_LOG_ERROR);

    clear();

    ctx = allocContext(width, height, frameRate);
    if (ctx == nullptr)
        // Error message is created in allocContext
        return;

    codec = ctx->codec;

    frame = allocFrame();
    if (frame == nullptr)
        // Error message is created in allocFrame
        return;

    // Image bits are stored in same place as AVFrame data
    image = QImage(frame->data[0], ctx->width, ctx->height, QImage::Format_RGB32);

    pkt = av_packet_alloc();
    if (pkt == nullptr) {
        last_error = "Could not allocate packet";
        return;
    }

    file = fopen(fileName, "wb");
    if (file == nullptr) {
        last_error = "Could not open file";
        return;
    }
}

void VideoEncoder::clear()
{
    if (ctx != nullptr)
        avcodec_free_context(&ctx);
    if (frame != nullptr)
        av_frame_free(&frame);
    if (pkt != nullptr)
        av_packet_free(&pkt);
    pts = 0;
    image = QImage();
    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }
}

void VideoEncoder::encodeFrame()
{
    if (ctx == nullptr || frame == nullptr || file == nullptr) {
        last_error = "Error initializing encoder";
        return;
    }

    frame->pts = pts++;

    // Send the frame to the encoder
    av_error = avcodec_send_frame(ctx, frame);
    if (av_error < 0) {
        last_error = "Error sending a frame for encoding";
        return;
    }

    while (av_error >= 0) {
        av_error = avcodec_receive_packet(ctx, pkt);
        if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF) {
            av_packet_unref(pkt);
            return;
        } else if (av_error < 0) {
            last_error = "Error during encoding";
            return;
        }

        fwrite(pkt->data, 1, pkt->size, file);
        av_packet_unref(pkt);
    }
}
