#include "decoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

static const int frame_cycles = 2;
static const AVPixelFormat pix_fmt = AV_PIX_FMT_RGB32;

DecoderFrame::DecoderFrame(int width, int height) :
    width(width),
    height(height),
    current(0)
{
    int index;
    for (index = 0; index < frame_cycles; index++)
        cycles.push_back({Image(), nullptr, nullptr, false, false});

    if (width != 0 && height != 0)
        allocAll();
}

bool DecoderFrame::isValid() const
{
    size_t index;
    for (index = 0; index < cycles.size(); index++) {
        if (cycles[index].has_errors ||
                cycles[index].frame == nullptr ||
                cycles[index].buffer == nullptr)
            return false;
    }
    return true;
}

void DecoderFrame::resize(int width, int height)
{
    if (this->width != width || this->height != height) {
        this->width = width;
        this->height = height;

        if (width > 0 && height > 0)
            // Linesize alignment for scaled RGB32 frame should be 1 (or 4)
            num_bytes = av_image_get_buffer_size(pix_fmt, width, height, 1);
        else
            num_bytes = 0;

        size_t index;
        for (index = 0; index < cycles.size(); index++)
            cycles[index].need_resize = true;
    }
}

void DecoderFrame::shift()
{
    current = (current + 1) % cycles.size();
    if (cycles[current].need_resize) {
        cleanUp(current);
        alloc(current);
    }
}

void DecoderFrame::reset()
{
    size_t index;
    for (index = 0; index < cycles.size(); index++)
        shift();
}

void DecoderFrame::alloc(size_t index)
{
    Cycle &cycle = cycles[index];

    if (num_bytes > 0) {
        // Allocate an AVFrame structure
        cycle.frame = av_frame_alloc();
        if (cycle.frame == nullptr) {
            cycle.has_errors = true;
            errorMsg("Could not allocate frame");
            return;
        }

        cycle.frame->width = width;
        cycle.frame->height = height;
        cycle.frame->format = pix_fmt;

        cycle.buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(num_bytes)));
        if (cycle.buffer == nullptr) {
            cycle.has_errors = true;
            errorMsg("Could not allocate frame buffer");
            return;
        }

        av_image_fill_arrays(cycle.frame->data, cycle.frame->linesize, cycle.buffer, pix_fmt,
                             width, height, 1);

        cycle.image.assign(cycle.frame->data[0], width, height);
    } else {
        cycle.buffer = nullptr;
        cycle.frame = nullptr;
        cycle.image.clear();
    }

    cycle.need_resize = false;
}

void DecoderFrame::allocAll()
{
    num_bytes = av_image_get_buffer_size(pix_fmt, width, height, 1);

    size_t index;
    for (index = 0; index < cycles.size(); index++)
        alloc(index);
}

void DecoderFrame::cleanUp(size_t index)
{
    Cycle &cycle = cycles[index];

    // Clean up
    if (cycle.frame != nullptr)
        av_frame_free(&cycle.frame);
    if (cycle.buffer != nullptr)
        av_freep(&cycle.buffer);
    cycle.has_errors = false;
}

void DecoderFrame::cleanUpAll()
{
    size_t index;
    for (index = 0; index < cycles.size(); index++)
        cleanUp(index);
}

void DecoderFrame::errorMsg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

VideoDecoder::VideoDecoder() :
    av_error(0),
    format_ctx(nullptr),
    video_stream(nullptr),
    frame_index(0),
    codec_par(nullptr),
    codec_ctx(nullptr),
    codec(nullptr),
    _info({0, 0, 0, 0}),
    frame_(nullptr),
    frame_rgb(0, 0),
    pkt(nullptr),
    sws_ctx(nullptr),
    _eof(true),
    initial_size(QSize())
{
    av_log_set_level(AV_LOG_ERROR);
}

void VideoDecoder::cleanUp()
{
    if (format_ctx != nullptr)
        avformat_close_input(&format_ctx);
    if (codec_ctx != nullptr)
        avcodec_close(codec_ctx);
    _info = {0, 0, 0, 0};
    if (frame_ != nullptr)
        av_frame_free(&frame_);
    frame_rgb.resizeHard(0, 0);
    if (pkt != nullptr)
        av_packet_free(&pkt);
    if (sws_ctx != nullptr) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    video_stream = nullptr;
    frame_index = 0;
    _eof = true;
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

    int stream_index;

    // Find the first video stream
    for (stream_index = 0; stream_index < static_cast<int>(format_ctx->nb_streams); stream_index++) {
        const AVStream *stream = format_ctx->streams[stream_index];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = stream;
            _info.width = stream->codecpar->width;
            _info.height = stream->codecpar->height;
            _info.framecount = stream->nb_frames;
            if (stream->avg_frame_rate.den != 0)
                _info.framerate =  static_cast<int>(av_q2d(stream->avg_frame_rate) + 0.1);
            else {
                errorMsg("Error: Could not determine framerate");
                return;
            }
            break;
        }
    }

    if (video_stream == nullptr) {
        errorMsg("Error: Did not find a video stream");
        return;
    }

    // Get a pointer to the codec context for the video stream
    codec_par = video_stream->codecpar;

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


    // Set width and height for scaling depending on
    // if resize was called before open
    // -> in that case, set the scaling size to that size
    // -> otherwise, use the width and height of the video
    QSize scaling_size;

    if (initial_size == QSize())
        scaling_size = QSize(codec_ctx->width, codec_ctx->height);
    else
        scaling_size = initial_size;

    // Allocate structure for scaled RGB frame
    // -> in RGB32 format with specified width and height
    frame_rgb.resizeHard(scaling_size.width(), scaling_size.height());
    if (!frame_rgb.isValid()) {
        errorMsg("Could not initialize RGB frame");
        return;
    }

    // Clean up initial_size in case 'open' is called again
    initial_size = QSize();

    // Also initialize scaling context
    sws_ctx = sws_getContext(codec_ctx->width,
                             codec_ctx->height,
                             codec_ctx->pix_fmt,
                             scaling_size.width(),
                             scaling_size.height(),
                             AV_PIX_FMT_RGB32,
                             SWS_BILINEAR,
                             nullptr,
                             nullptr,
                             nullptr
                             );

    if (sws_ctx == nullptr) {
        errorMsg("Failed to get scaling context");
        return;
    }

    pkt = av_packet_alloc();
    if (pkt == nullptr) {
        errorMsg("Could not allocate packet");
        return;
    }

    _eof = false;
}

bool VideoDecoder::readFrame()
{
    while (!_eof && (av_error = av_read_frame(format_ctx, pkt)) >= 0) {
        // Is this a packet from the video stream?
        if (pkt->stream_index == video_stream->index) {
            // Decode video frame
            frame_finished = 0;

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

void VideoDecoder::swsScale()
{
    frame_rgb.shift();

    if (sws_ctx != nullptr) {
        // Convert the image from its native format to RGB
        sws_scale(sws_ctx, static_cast<uint8_t const * const *>(frame_->data),
                  frame_->linesize, 0, codec_ctx->height,
                  frame_rgb.frame()->data, frame_rgb.frame()->linesize);
    }

    frame_index++;

    // This finishes readFrame()
    av_packet_unref(pkt);
}

void VideoDecoder::resize(const QSize &size)
{
    if (!isOpen()) {
        initial_size = size;
        return;
    }

    if (size == QSize(frame_rgb.width, frame_rgb.height))
        return;

    if (sws_ctx != nullptr)
        sws_freeContext(sws_ctx);

    if (size.width() < 1 || size.height() < 1) {
        frame_rgb.resize(0, 0);
        sws_ctx = nullptr;
    } else {
        frame_rgb.resize(size.width(), size.height());
        // Also initialize scaling context
        sws_ctx = sws_getContext(codec_ctx->width,
                                 codec_ctx->height,
                                 codec_ctx->pix_fmt,
                                 size.width(),
                                 size.height(),
                                 AV_PIX_FMT_RGB32,
                                 SWS_BILINEAR,
                                 nullptr,
                                 nullptr,
                                 nullptr
                                 );
    }
}

void VideoDecoder::seek(int n_frame)
{
    _eof = n_frame >= _info.framecount;

    if (_eof)
        return;

    int64_t t_frame = av_q2d({n_frame * video_stream->time_base.den, _info.framerate * video_stream->time_base.num});

    av_error = avformat_seek_file(format_ctx, video_stream->index, t_frame, t_frame, t_frame, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
    if (av_error < 0)
        return errorMsg("Error seeking frame");

    frame_index = n_frame;
}

void VideoDecoder::errorMsg(const char *msg)
{
    last_error = msg;
}

DecoderThread::DecoderThread(QObject *parent) :
    QThread(parent)
  , video(nullptr)
  , initial_size(QSize(0, 0))
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

void DecoderThread::next()
{
    QMutexLocker locker(&mutex);

    // Continue only, when end is not yet reached!
    // In order to continue playing, use seekFrame!
    if (!eof) {
        continue_reading = true;
        condition.wakeOne();
    }
}

void DecoderThread::stop()
{
    QMutexLocker locker(&mutex);

    quit = true;
    condition.wakeOne();
}

void DecoderThread::seekFrame(int index)
{
    QMutexLocker locker(&mutex);

    seek_frame = index;
    eof = false;
    condition.wakeOne();
}

void DecoderThread::resize(const QSize &size)
{
    QMutexLocker locker(&mutex);

    decoder.resize(size);
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

    // Initialize values
    continue_reading = true;
    eof = false;
    quit = false;
    seek_frame = -1;

    while (!quit) {

        while (!quit && !eof) {
            if (seek_frame != -1) {
                decoder.seek(seek_frame);
                seek_frame = -1;
            }
            eof = !decoder.readFrame();
            if (eof)
                break;
            mutex.lock();
            if (!continue_reading && !quit)
                condition.wait(&mutex);

            if (!quit) {
                decoder.swsScale();
                mutex.unlock();

                // Continue reading only when next() is called
                // to avoid a race-condition for decoder.frame()
                continue_reading = false;

                emit newFrame(&decoder.frame());
            } else
                mutex.unlock();
        }

        emit finished();

        if (!quit) {
            mutex.lock();
            condition.wait(&mutex);
            mutex.unlock();
        }
    }
}
