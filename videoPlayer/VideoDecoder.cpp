#include "VideoDecoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoDecoder::VideoDecoder(QObject *parent) : QThread(parent)
{
    format_ctx = nullptr;
    codec_par = nullptr;

    codec_ctx = nullptr;
    codec = nullptr;

    frame = nullptr;
    frame_rgb = nullptr;

    buffer = nullptr;
    sws_ctx = nullptr;

    frame_counter = 0;

    quit = false;
    continue_reading = true;
}

VideoDecoder::~VideoDecoder()
{
    stop();
    wait();
}

void VideoDecoder::setFileName(const QString &fileName)
{
    QMutexLocker locker(&mutex);

    this->fileName = fileName;
}

#ifdef PRINT_DEBUG_LOG
#include <QDebug>
#endif

void VideoDecoder::run()
{
    // av_register_all(); // -> deprecated
    // see https://github.com/leandromoreira/ffmpeg-libav-tutorial/issues/29

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avformat_open_input");
#endif

    // Open video file
    if (avformat_open_input(&format_ctx, fileName.toStdString().c_str(), nullptr, nullptr) < 0) {
        emit error("Could not open file");
        return;
    }

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avformat_find_stream_info");
#endif

    // Retrieve stream information
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        emit error("Error: Could not find stream information");
        return; // Couldn't find stream information
    }

    // Dump information about file onto standard error
//    av_dump_format(format_ctx, 0, fileName.toStdString().c_str(), 0);

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> 'find first video stream'");
#endif

    // Find the first video stream
    video_stream = -1;
    for (frame_counter = 0; frame_counter < static_cast<int>(format_ctx->nb_streams); frame_counter++) {
        const AVStream *stream = format_ctx->streams[frame_counter];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = frame_counter;
            if (stream->avg_frame_rate.den != 0)
                frame_rate =  static_cast<int>(av_q2d(stream->avg_frame_rate) + 0.5);
            else {
                emit error("Error: Could not determine framerate");
                return;
            }
            break;
        }
    }

    if (video_stream == -1) {
        emit error("Error: Did not find a video stream");
        return; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    codec_par = format_ctx->streams[video_stream]->codecpar;

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avcodec_find_decoder");
#endif

    // Find the decoder for the video stream
    codec = avcodec_find_decoder(codec_par->codec_id);
//    codec = avcodec_find_decoder_by_name("ffv1");
//    codec = avcodec_find_encoder_by_name(codec_name);
    if (codec == nullptr) {
        emit error("Error: Unsupported codec!");
        return; // Codec not found
    }

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avcodec_alloc_context3");
#endif

    // Copy context
    // -> Replace avcodec_copy_context -> deprecated
    //    with avcodec_parameters to context
    codec_ctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codec_ctx, codec_par) < 0) {
        emit error("Error: Could not copy codec parameters to context");
        return; // Couldn't copy parameters
    }

    // Open codec
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        emit error("Error: Could not open codec");
        return; // Could not open codec
    }

    // Allocate video frame
    frame = av_frame_alloc();

    // Allocate an AVFrame structure
    frame_rgb = av_frame_alloc();
    if (frame_rgb == nullptr) {
        emit error("Could not allocate frame");
        return;
    }

    packet = av_packet_alloc();

    // Determine required buffer size and allocate buffer
    // Replace avpicture_get_size -> deprecated
    // with av_image_get_buffer_size

    // Remark: Not sure if align needs to be 1 or 32, see
    // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
    num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, codec_ctx->width,
                                        codec_ctx->height, 1);
    buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(num_bytes) * sizeof(uint8_t)));

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

    while (av_read_frame(format_ctx, packet) >= 0 && !quit) {
        // Is this a packet from the video stream?
        if (packet->stream_index == video_stream) {
            // Decode video frame
            frame_finished = 0;

            // Replace avcodec_decode_video2 -> deprecated
            // with avcodec_send_packet and avcodec_receive_frame
            // see https://github.com/pesintta/vdr-plugin-vaapidevice/issues/31
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO ||
                codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
                av_error = avcodec_send_packet(codec_ctx, packet);
                if (av_error < 0 && av_error != AVERROR(EAGAIN) && av_error != AVERROR_EOF) {
                } else {
                    if (av_error >= 0)
                        packet->size = 0;
                    av_error = avcodec_receive_frame(codec_ctx, frame);
                    if (av_error >= 0)
                        frame_finished = 1;
                }
            }

            // Did we get a video frame?
            if (frame_finished) {
                // Wait for VideoPlayer to be ready
                // (this is necessary, since the image pointer is shared with VideoPlayer!)
                mutex.lock();
                if (!continue_reading && !quit) {
                    condition.wait(&mutex);
                }
                mutex.unlock();

                if (!quit) {
                    // Convert the image from its native format to RGB
                    sws_scale(sws_ctx, static_cast<uint8_t const * const *>(frame->data),
                              frame->linesize, 0, codec_ctx->height,
                              frame_rgb->data, frame_rgb->linesize);

                    // At this point, the player has already finished the update
                    // and can not interfere with the flow in this section
                    continue_reading = false;

                    image = QImage((uchar*)frame_rgb->data[0], codec_ctx->width, codec_ctx->height, QImage::Format_RGB32);
                    emit newFrame(&image);

                    frame_counter++;
                }
            }
        }

        // Unref the packet that was allocated by av_read_frame
        av_packet_unref(packet);
    }

    // Unref the packet that was allocated by av_read_frame
    av_packet_unref(packet);

    av_packet_free(&packet);

    // Free the original frame(s)
    av_frame_free(&frame);
    av_frame_free(&frame_rgb);

    // Close the codec
    avcodec_close(codec_ctx);

    // Close the video file
    avformat_close_input(&format_ctx);

    emit finished();
}

void VideoDecoder::next()
{
    QMutexLocker locker(&mutex);

    continue_reading = true;
    condition.wakeOne();
}

void VideoDecoder::stop()
{
    QMutexLocker locker(&mutex);

    quit = true;
    condition.wakeOne();
}
