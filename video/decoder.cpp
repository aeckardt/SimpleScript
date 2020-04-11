#include "decoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoDecoder::VideoDecoder() :
    av_error(0),
    format_ctx(nullptr),
    frame_counter(0),
    codec_par(nullptr),
    codec_ctx(nullptr),
    codec(nullptr),
    frame_(nullptr),
    frame_rgb(nullptr),
    _eof(true),
    pkt(nullptr),
    buffer(nullptr)
{
}

void VideoDecoder::cleanUp()
{
    if (buffer != nullptr)
        av_freep(&buffer);
    if (pkt != nullptr)
        av_packet_free(&pkt);
    if (frame_ != nullptr)
        av_frame_free(&frame_);
    if (codec_ctx != nullptr)
        avcodec_close(codec_ctx);
    if (format_ctx != nullptr)
        avformat_close_input(&format_ctx);
    frame_counter = 0;
    _eof = true;
}

void VideoDecoder::errorMsg(const char *msg)
{
    last_error = msg;
}

void VideoDecoder::open(const VideoFile &video_file)
{
    cleanUp();

    // Open video file
    if (avformat_open_input(&format_ctx, video_file.fileName().toStdString().c_str(), nullptr, nullptr) < 0) {
        errorMsg("Could not open file");
        return;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        errorMsg("Error: Could not find stream information");
        return;
    }

    // Find the first video stream
    video_stream = -1;
    for (frame_counter = 0; frame_counter < static_cast<int>(format_ctx->nb_streams); frame_counter++) {
        const AVStream *stream = format_ctx->streams[frame_counter];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = frame_counter;
            if (stream->avg_frame_rate.den != 0)
                frame_rate =  static_cast<int>(av_q2d(stream->avg_frame_rate) + 0.5);
            else {
                errorMsg("Error: Could not determine framerate");
                return;
            }
            break;
        }
    }

    if (video_stream == -1) {
        errorMsg("Error: Did not find a video stream");
        return;
    }

    // Get a pointer to the codec context for the video stream
    codec_par = format_ctx->streams[video_stream]->codecpar;

    // Find the decoder for the video stream
    codec = avcodec_find_decoder(codec_par->codec_id);
    if (codec == nullptr) {
        errorMsg("Error: Unsupported codec!");
        return;
    }

    // Copy context
    codec_ctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codec_ctx, codec_par) < 0) {
        errorMsg("Error: Could not copy codec parameters to context");
        return;
    }

    // Open codec
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        errorMsg("Error: Could not open codec");
        return;
    }

    // Allocate video frame
    frame_ = av_frame_alloc();
    if (frame_ == nullptr) {
        errorMsg("Could not allocate frame");
        return;
    }

    // Allocate an AVFrame structure
    frame_rgb = av_frame_alloc();
    if (frame_rgb == nullptr) {
        errorMsg("Could not allocate frame");
        return;
    }

    pkt = av_packet_alloc();

    // Determine required buffer size and allocate buffer
    // Replace avpicture_get_size -> deprecated
    // with av_image_get_buffer_size

    // Remark: Not sure if align needs to be 1 or 32, see
    // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
    num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, codec_ctx->width,
                                        codec_ctx->height, 32);
    buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(num_bytes)));

    // Assign appropriate parts of buffer to image planes in frame_rgb
    // Note that frame_rgb is an AVFrame, but AVFrame is a superset
    // of AVPicture

    // Replace avpicture_fill -> deprecated
    // with av_image_fill_arrays

    // Examples of av_image_fill_arrays from
    // https://mail.gnome.org/archives/commits-list/2016-February/msg05531.html
    // https://github.com/bernhardu/dvbcut-deb/blob/master/src/avframe.cpp
    av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, buffer, AV_PIX_FMT_RGB32,
                         codec_ctx->width, codec_ctx->height, 1);

    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(codec_ctx->width,
                             codec_ctx->height,
                             codec_ctx->pix_fmt,
                             codec_ctx->width,
                             codec_ctx->height,
                             AV_PIX_FMT_RGB32,
                             SWS_BILINEAR,
                             nullptr,
                             nullptr,
                             nullptr
                             );

    _eof = false;
}

bool VideoDecoder::readFrame()
{
    while (!_eof && (av_error = av_read_frame(format_ctx, pkt)) >= 0) {
        // Is this a packet from the video stream?
        if (pkt->stream_index == video_stream) {
            // Decode video frame
            frame_finished = 0;

            // Replace avcodec_decode_video2 -> deprecated
            // with avcodec_send_packet and avcodec_receive_frame
            // see https://github.com/pesintta/vdr-plugin-vaapidevice/issues/31
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO ||
                codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
                av_error = avcodec_send_packet(codec_ctx, pkt);
                if (av_error < 0 && av_error != AVERROR(EAGAIN) && av_error != AVERROR_EOF) {
                } else {
                    if (av_error >= 0)
                        pkt->size = 0;
                    av_error = avcodec_receive_frame(codec_ctx, frame_);
                    if (av_error >= 0)
                        frame_finished = 1;
                    else if (av_error == AVERROR_EOF) {
                        _eof = true;
                        break;
                    }
                }
            }
            if (frame_finished)
                break;
        } else
            continue;
    }

    if (av_error == AVERROR_EOF)
        _eof = true;

    return !_eof;
}

void VideoDecoder::scaleFrame()
{
    // Convert the image from its native format to RGB
    sws_scale(sws_ctx, static_cast<uint8_t const * const *>(frame_->data),
              frame_->linesize, 0, codec_ctx->height,
              frame_rgb->data, frame_rgb->linesize);

    image.assign(frame_rgb->data[0], codec_ctx->width, codec_ctx->height);

    frame_counter++;

    av_packet_unref(pkt);
}

DecoderThread::DecoderThread(QObject *parent) :
    QThread(parent),
    video(nullptr),
    continue_reading(true),
    quit(false)
{
}

DecoderThread::~DecoderThread()
{
    stop();
    wait();
}

void DecoderThread::setFile(const VideoFile &video)
{
    QMutexLocker locker(&mutex);
    this->video = &video;
}

void DecoderThread::run()
{
    if (video == nullptr) {
        emit error("No video file specified");
        return;
    }

    decoder.open(*video);

    if (decoder.last_error != "") {
        emit error(decoder.last_error);
        return;
    }

    while (!quit && decoder.readFrame()) {
        mutex.lock();
        if (!continue_reading && !quit)
            condition.wait(&mutex);
        mutex.unlock();

        if (!quit) {
            decoder.scaleFrame();

            continue_reading = false;

            emit newFrame(&decoder.frame());
        }
    }

    emit finished();
}

void DecoderThread::next()
{
    QMutexLocker locker(&mutex);

    continue_reading = true;
    condition.wakeOne();
}

void DecoderThread::stop()
{
    QMutexLocker locker(&mutex);

    quit = true;
    condition.wakeOne();
}
