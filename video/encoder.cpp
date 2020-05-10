#include "encoder.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}

static const char* format_container = "avi";
static const AVCodecID codec_id     = AV_CODEC_ID_H264;
static const char* codec_name       = "libx264rgb";
static const AVPixelFormat pix_fmt  = AV_PIX_FMT_BGR0;
static const int linesize_alignment = 1;

// void(0) is used to enforce semicolon after the macro
#define errorMsgf(format, ...) \
{ char *buffer = new char[strlen(format) * 2 + 50]; sprintf(buffer, format, __VA_ARGS__); errorMsg(buffer); } (void)0

VideoEncoder::VideoEncoder()
    : av_error(0),
      width(0),
      height(0),
      frame_counter(0),
      frame_rate(0),
      output_fmt(nullptr),
      format_ctx(nullptr),
      video_stream(nullptr),
      codec(nullptr),
      codec_ctx(nullptr),
      frame_rgb(nullptr),
      pkt(nullptr),
      image(linesize_alignment), // set x byte linesize alignment for image
      video_file(nullptr)
{
    av_log_set_level(AV_LOG_ERROR);
}

void VideoEncoder::open(const VideoFile &video_file, int width, int height, int frame_rate)
{
    this->video_file = &video_file;
    this->width = width;
    this->height = height;
    this->frame_rate = frame_rate;

    initialize();
}

void VideoEncoder::allocFormatContext()
{
    output_fmt = av_guess_format(format_container, nullptr, nullptr);
    if (output_fmt == nullptr)
        return errorMsg("Could not guess format.");

    format_ctx = nullptr;

    int av_error = avformat_alloc_output_context2(&format_ctx, output_fmt, nullptr, video_file->fileName().toStdString().c_str());
    if (av_error < 0)
        return errorMsg("Could not open video file format context.");

    video_stream = avformat_new_stream(format_ctx, nullptr);

    video_stream->id = format_ctx->nb_streams - 1;
    video_stream->codecpar->codec_id = codec_id;
    video_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    video_stream->codecpar->width = width;
    video_stream->codecpar->height = height;
    video_stream->codecpar->format = pix_fmt;
    video_stream->time_base = AVRational{1, frame_rate};
}

void VideoEncoder::allocCodecContext()
{
    codec = avcodec_find_encoder_by_name(codec_name);
    if (codec == nullptr) {
        errorMsgf("Codec '%s' not found", codec_name);
        return;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (codec_ctx == nullptr)
        return errorMsg("Could not allocate video codec context");

    // Copy codec context from stream parameters initially
    av_error = avcodec_parameters_to_context(codec_ctx, video_stream->codecpar);
    if (av_error < 0)
        return errorMsg("Could not copy codec parameters to context.");

    codec_ctx->gop_size = 12;
    codec_ctx->max_b_frames = 2;
    codec_ctx->time_base = AVRational{1, frame_rate};

    // Use optimal number of threads
    // see https://superuser.com/questions/155305/how-many-threads-does-ffmpeg-use-by-default
    codec_ctx->thread_count = 0;

    if (codec_id == AV_CODEC_ID_H264) {
        // Set optimal compression / speed ratio for this use-case
        av_opt_set(codec_ctx->priv_data, "preset", "fast", 0);

        // Set constant rate factor to lossless
        // see https://trac.ffmpeg.org/wiki/Encode/H.264
        av_opt_set(codec_ctx->priv_data, "crf",    "0",    0);
    }

    // Use global header only if format container is not mp4
    // see https://stackoverflow.com/questions/46444474/c-ffmpeg-create-mp4-file
    if (format_ctx->oformat->flags & AVFMT_GLOBALHEADER && strcmp(format_container, "mp4") != 0)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // Copy codec context back to stream parameters
    av_error = avcodec_parameters_from_context(video_stream->codecpar, codec_ctx);
    if (av_error < 0)
        return errorMsg("Could not copy codec context back to parameters.");

    av_error = avcodec_open2(codec_ctx, codec, nullptr);
    if (av_error < 0) {
        char ch[AV_ERROR_MAX_STRING_SIZE] = {0};
        errorMsgf("Could not open codec: '%s' -> %s", codec_name, av_make_error_string(ch, AV_ERROR_MAX_STRING_SIZE, av_error));
        avcodec_free_context(&codec_ctx);
        return;
    }
}

void VideoEncoder::writeHeader()
{
    if (!(format_ctx->oformat->flags & AVFMT_NOFILE)) {
        av_error = avio_open(&format_ctx->pb, video_file->fileName().toStdString().c_str(), AVIO_FLAG_WRITE);
        if (av_error < 0)
            return errorMsg("Failed to open file.");
    }

    av_error = avformat_write_header(format_ctx, NULL);
    if (av_error < 0)
        return errorMsg("Failed to write header.");
}

void VideoEncoder::allocFrame()
{
    frame_rgb = av_frame_alloc();
    if (frame_rgb == nullptr)
        return errorMsg("Could not allocate video frame");

    frame_rgb->format = pix_fmt;
    frame_rgb->width  = width;
    frame_rgb->height = height;
}

void VideoEncoder::allocFrameBuffer(uint8_t *bits)
{
    // Write frame into external buffer to avoid to make an extra copy, if possible!
    av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, bits, pix_fmt,
                         width, height, linesize_alignment);
}

void VideoEncoder::initialize()
{
    cleanUp();

    allocFormatContext();
    if (format_ctx == nullptr)
        return;

    allocCodecContext();
    if (codec_ctx == nullptr)
        return;

    writeHeader();

    allocFrame();
    if (frame_rgb == nullptr)
        return;

    pkt = av_packet_alloc();

    // The frame buffer is allocated externally to avoid copy
    connect(&image, &Image::reallocate, this, &VideoEncoder::allocFrameBuffer, Qt::DirectConnection);
}

void VideoEncoder::cleanUp()
{
    if (frame_rgb != nullptr)
        av_frame_free(&frame_rgb);
    image.clear();
    if (pkt != nullptr)
        av_packet_free(&pkt);
    if (codec_ctx != nullptr)
        avcodec_free_context(&codec_ctx);
    if (format_ctx != nullptr && !(format_ctx->oformat->flags & AVFMT_NOFILE))
        avio_close(format_ctx->pb);
    if (format_ctx != nullptr) {
        avformat_free_context(format_ctx);
        format_ctx = nullptr;
    }
    frame_counter = 0;
}

void VideoEncoder::addFrame()
{
    if (format_ctx == nullptr || codec_ctx == nullptr || frame_rgb == nullptr) {
        errorMsg("Error initializing encoder");
        return;
    }

    pkt->data = nullptr;
    pkt->size = 0;

    // The time_base may be changed by avformat_write_header
    frame_rgb->pts = static_cast<int64_t>(static_cast<double>(frame_counter++) /
                                         (static_cast<double>(frame_rate) * av_q2d(video_stream->time_base)));

    // Send the frame to the encoder
    av_error = avcodec_send_frame(codec_ctx, frame_rgb);
    if (av_error < 0)
        return errorMsg("Error sending a frame for encoding.");

    av_error = avcodec_receive_packet(codec_ctx, pkt);
    if (av_error == AVERROR(EAGAIN))
        return;
    else if (av_error < 0)
        return errorMsg("Error during encoding.");

    av_error = av_write_frame(format_ctx, pkt);
    if (av_error < 0)
        return errorMsg("Error writing frame.");

    av_packet_unref(pkt);
}

void VideoEncoder::finish()
{
    if (format_ctx == nullptr || codec_ctx == nullptr || frame_rgb == nullptr) {
        errorMsg("Error initializing encoder");
        return;
    }

    pkt->data = nullptr;
    pkt->size = 0;

    while (true) {
        // Enter draining mode by sending empty buffer
        av_error = avcodec_send_frame(codec_ctx, nullptr);
        if (av_error < 0 && av_error != AVERROR_EOF)
            return errorMsg("Error sending a frame encoding (flushing).");

        av_error = avcodec_receive_packet(codec_ctx, pkt);
        if (av_error < 0) {
            if (av_error == AVERROR_EOF)
                break;
            else
                return errorMsg("Error during encoding (flushing).");
        }

        av_error = av_write_frame(format_ctx, pkt);
        if (av_error < 0)
            return errorMsg("Error writing frame (flushing).");
        av_packet_unref(pkt);
    }

    av_write_trailer(format_ctx);

    cleanUp();
}

void VideoEncoder::errorMsg(const char *msg)
{
    last_error = msg;
    fprintf(stderr, "%s\n", msg);
}
