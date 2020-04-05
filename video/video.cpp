#include "video.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
}

static const char* codec_name      = "libx264rgb";
static const AVPixelFormat pix_fmt = AV_PIX_FMT_BGR0;

// void(0) is used to enforce semicolon after the macro
#define errorMsgf(format, ...) \
{ char *buffer = new char[strlen(format) * 2 + 50]; sprintf(buffer, format, __VA_ARGS__); errorMsg(buffer); } (void)0

Video::Video()
    : av_error(0),
      width(0),
      height(0),
      frame_rate(0),
      ctx(nullptr),
      codec(nullptr),
      frame(nullptr),
      pkt(nullptr),
      pts(0),
      file(nullptr)
{
    av_log_set_level(AV_LOG_ERROR);
}

void Video::allocContext()
{
    const AVCodec *codec;

    codec = avcodec_find_encoder_by_name(codec_name);
    if (codec == nullptr) {
        errorMsgf("Codec '%s' not found", codec_name);
        return;
    }

    ctx = avcodec_alloc_context3(codec);
    if (ctx == nullptr)
        return errorMsg("Could not allocate video codec context");

    ctx->bit_rate = 400000;
    ctx->width = width;
    ctx->height = height;
    ctx->time_base = {1, frame_rate};
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
        errorMsgf("Could not open codec: '%s' -> %s", codec_name, av_make_error_string(ch, AV_ERROR_MAX_STRING_SIZE, av_error));
        avcodec_free_context(&ctx);
        return;
    }
}

void Video::allocFrame()
{
    frame = av_frame_alloc();
    if (frame == nullptr)
        return errorMsg("Could not allocate video frame");

    frame->format = pix_fmt;
    frame->width  = width;
    frame->height = height;

    av_error = av_frame_get_buffer(frame, 32);
    if (av_error < 0) {
        av_frame_free(&frame);
        return errorMsg("Could not allocate the video frame data");
    }
}

void Video::cleanUp()
{
    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }
    if (ctx != nullptr)
        avcodec_free_context(&ctx);
    if (frame != nullptr)
        av_frame_free(&frame);
    image = QImage();
    if (pkt != nullptr)
        av_packet_free(&pkt);
    pts = 0;
}

void Video::create(int width, int height, int frame_rate)
{
    this->width = width;
    this->height = height;
    this->frame_rate = frame_rate;

    initialize();

    temp_file.open();

    file = fopen(temp_file.fileName().toStdString().c_str(), "wb");
    if (file == nullptr)
        return errorMsg("Could not open file");
}

void Video::encodeFrame()
{
    if (ctx == nullptr || frame == nullptr || file == nullptr) {
        errorMsg("Error initializing encoder");
        return;
    }

    // Send the frame to the encoder
    av_error = avcodec_send_frame(ctx, frame);
    if (av_error < 0)
        return errorMsg("Error sending a frame for encoding");

    while (av_error >= 0) {
        av_error = avcodec_receive_packet(ctx, pkt);
        if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF)
            return;

        else if (av_error < 0)
            return errorMsg("Error during encoding");

        frame->pts = pts++;

        fwrite(pkt->data, 1, pkt->size, file);
        av_packet_unref(pkt);
    }
}

void Video::flush()
{
    if (ctx == nullptr || file == nullptr) {
        errorMsg("Error initializing encoder");
        return;
    }

    // Enter draining mode by sending empty buffer
    av_error = avcodec_send_frame(ctx, nullptr);
    if (av_error < 0)
        return errorMsg("Error flushing");

    while (av_error >= 0) {
        av_error = avcodec_receive_packet(ctx, pkt);
        if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF)
            return;
        else if (av_error < 0)
            return errorMsg("Error during encoding");

        frame->pts = pts++;

        fwrite(pkt->data, 1, pkt->size, file);
        av_packet_unref(pkt);
    }

    cleanUp();
}

void Video::errorMsg(const char *msg)
{
    last_error = msg;
}

void Video::initialize()
{
    cleanUp();

    allocContext();
    if (ctx == nullptr)
        return;

    codec = ctx->codec;

    allocFrame();
    if (frame == nullptr)
        return;

    av_error = av_frame_make_writable(frame);
    if (av_error < 0)
        return errorMsg("Could not make frame writable");

    image = QImage(frame->data[0], width, height, QImage::Format_RGB32);

    pkt = av_packet_alloc();
    if (pkt == nullptr) {
        errorMsg("Could not allocate packet");
        return;
    }
}

Video &Video::operator=(Video &&src)
{
    cleanUp();

    width = src.width;
    height = src.height;
    frame_rate = src.frame_rate;

    ctx = src.ctx;
    codec = src.codec;
    frame = src.frame;
    pkt = src.pkt;
    pts = src.pts;

    if (frame != nullptr) {
        image = QImage(frame->data[0], width, height, QImage::Format_RGB32);
    }

    file = src.file;

    src.ctx = nullptr;
    src.codec = nullptr;
    src.frame = nullptr;
    src.pkt = nullptr;
    src.pts = 0;
    src.image = QImage();
    src.file = nullptr;

    return *this;
}
