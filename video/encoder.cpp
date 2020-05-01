#include "encoder.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}

static const char* codec_name      = "libx264rgb";
static const AVPixelFormat pix_fmt = AV_PIX_FMT_BGR0;

// void(0) is used to enforce semicolon after the macro
#define errorMsgf(format, ...) \
{ char *buffer = new char[strlen(format) * 2 + 50]; sprintf(buffer, format, __VA_ARGS__); errorMsg(buffer); } (void)0

VideoEncoder::VideoEncoder()
    : av_error(0),
      width(0),
      height(0),
      frame_rate(0),
      ctx(nullptr),
      codec(nullptr),
      frame_(nullptr),
      pkt(nullptr),
      pts(0),
      image(32), // set 32 byte linesize alignment for image
      video_file(nullptr),
      file(nullptr)
{
    av_log_set_level(AV_LOG_ERROR);
}

void VideoEncoder::allocContext()
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

    ctx->width = width;
    ctx->height = height;
    ctx->time_base = {1, frame_rate};
    ctx->gop_size = 10;
    ctx->max_b_frames = 1;
    ctx->pix_fmt = pix_fmt;

    // Use optimal number of threads
    // see https://superuser.com/questions/155305/how-many-threads-does-ffmpeg-use-by-default
    ctx->thread_count = 0;

    // Set optimal compression / speed ratio for this use-case
    av_opt_set(ctx->priv_data, "preset", "fast", 0);

    // Set constant rate factor to lossless
    // see https://trac.ffmpeg.org/wiki/Encode/H.264
    av_opt_set(ctx->priv_data, "crf",    "0",    0);

    av_error = avcodec_open2(ctx, codec, nullptr);
    if (av_error < 0) {
        char ch[AV_ERROR_MAX_STRING_SIZE] = {0};
        errorMsgf("Could not open codec: '%s' -> %s", codec_name, av_make_error_string(ch, AV_ERROR_MAX_STRING_SIZE, av_error));
        avcodec_free_context(&ctx);
        return;
    }
}

void VideoEncoder::allocFrame()
{
    frame_ = av_frame_alloc();
    if (frame_ == nullptr)
        return errorMsg("Could not allocate video frame");

    frame_->format = pix_fmt;
    frame_->width  = width;
    frame_->height = height;
}

void VideoEncoder::allocFrameBuffer(uint8_t *bits)
{
    // Write frame into external buffer to avoid to make an extra copy, if possible!
    av_image_fill_arrays(frame_->data, frame_->linesize, bits, AV_PIX_FMT_RGB32, width, height, 1);
}

void VideoEncoder::cleanUp()
{
    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }
    if (ctx != nullptr)
        avcodec_free_context(&ctx);
    if (frame_ != nullptr)
        av_frame_free(&frame_);
    image.clear();
    if (pkt != nullptr)
        av_packet_free(&pkt);
    pts = 0;
}

void VideoEncoder::open(const VideoFile &video_file, int width, int height, int frame_rate)
{
    this->video_file = &video_file;
    this->width = width;
    this->height = height;
    this->frame_rate = frame_rate;

    initialize();

    file = fopen(this->video_file->fileName().toStdString().c_str(), "wb");
    if (file == nullptr)
        return errorMsg("Could not open file");
}

void VideoEncoder::encodeFrame(bool flush)
{
    if (ctx == nullptr || frame_ == nullptr || file == nullptr) {
        errorMsg("Error initializing encoder");
        return;
    }

    if (!flush) {
        // Send the frame to the encoder
        av_error = avcodec_send_frame(ctx, frame_);
        if (av_error < 0)
            return errorMsg("Error sending a frame for encoding");
    } else {
        // Enter draining mode by sending empty buffer
        av_error = avcodec_send_frame(ctx, nullptr);
        if (av_error < 0)
            return errorMsg("Error flushing");
    }

    while (av_error >= 0) {
        av_error = avcodec_receive_packet(ctx, pkt);
        if (av_error == AVERROR(EAGAIN) || av_error == AVERROR_EOF) {
            if (flush) {
                fclose(file);
                file = nullptr;
            }
            return;
        }

        else if (av_error < 0)
            return errorMsg("Error during encoding");

        frame_->pts = pts++;

        fwrite(pkt->data, 1, static_cast<size_t>(pkt->size), file);
        av_packet_unref(pkt);
    }
}

void VideoEncoder::errorMsg(const char *msg)
{
    last_error = msg;
    fprintf(stderr, "%s\n", msg);
}

void VideoEncoder::initialize()
{
    cleanUp();

    allocContext();
    if (ctx == nullptr)
        return;

    codec = ctx->codec;

    allocFrame();
    if (frame_ == nullptr)
        return;

    // The frame buffer is allocated externally
    connect(&image, &Image::reallocate, this, &VideoEncoder::allocFrameBuffer, Qt::DirectConnection);

    pkt = av_packet_alloc();
    if (pkt == nullptr) {
        errorMsg("Could not allocate packet");
        return;
    }
}
