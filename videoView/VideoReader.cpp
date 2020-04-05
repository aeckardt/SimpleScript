#include "VideoReader.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoReader::VideoReader(QObject *parent) : QThread(parent)
{
    pFormatCtx = nullptr;
    pCodecPar = nullptr;

    pCodecCtx = nullptr;
    pCodec = nullptr;

    pFrame = nullptr;
    pFrameRGB = nullptr;

    buffer = nullptr;
    sws_ctx = nullptr;

    frame = 0;

    quit = false;
    continueReading = true;
}

VideoReader::~VideoReader()
{
    stop();
    wait();
}

void VideoReader::setFileName(const QString &fileName)
{
    QMutexLocker locker(&mutex);

    this->fileName = fileName;
}

#ifdef PRINT_DEBUG_LOG
#include <QDebug>
#endif

void VideoReader::run()
{
    // av_register_all(); // -> deprecated
    // see https://github.com/leandromoreira/ffmpeg-libav-tutorial/issues/29

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avformat_open_input");
#endif

    // Open video file
    if (avformat_open_input(&pFormatCtx, fileName.toStdString().c_str(), nullptr, nullptr) < 0) {
        emit error("Could not open file");
        return;
    }

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avformat_find_stream_info");
#endif

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        emit error("Error: Could not find stream information");
        return; // Couldn't find stream information
    }

    // Dump information about file onto standard error
//    av_dump_format(pFormatCtx, 0, current_file_path, 0);

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> 'find first video stream'");
#endif

    // Find the first video stream
    videoStream = -1;
    for (frame = 0; frame < static_cast<int>(pFormatCtx->nb_streams); frame++)
        if (pFormatCtx->streams[frame]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = frame;
            break;
        }

    if (videoStream == -1) {
        emit error("Error: Did not find a video stream");
        return; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecPar = pFormatCtx->streams[videoStream]->codecpar;

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avcodec_find_decoder");
#endif

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecPar->codec_id);
//    pCodec = avcodec_find_decoder_by_name("ffv1");
//    codec = avcodec_find_encoder_by_name(codec_name);
    if (pCodec == nullptr) {
        emit error("Error: Unsupported codec!");
        return; // Codec not found
    }

#ifdef PRINT_DEBUG_LOG
    qDebug() << ("-> avcodec_alloc_context3");
#endif

    // Copy context
    // -> Replace avcodec_copy_context -> deprecated
    //    with avcodec_parameters to context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (avcodec_parameters_to_context(pCodecCtx, pCodecPar) < 0) {
        emit error("Error: Could not copy codec parameters to context");
        return; // Couldn't copy parameters
    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        emit error("Error: Could not open codec");
        return; // Could not open codec
    }

    // Allocate video frame
    pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == nullptr) {
        emit error("Could not allocate frame");
        return;
    }

    packet = av_packet_alloc();

    // Determine required buffer size and allocate buffer
    // Replace avpicture_get_size -> deprecated
    // with av_image_get_buffer_size

    // Remark: Not sure if align needs to be 1 or 32, see
    // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, pCodecCtx->width,
                                        pCodecCtx->height, 1);
    buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(numBytes) * sizeof(uint8_t)));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture

    // Replace avpicture_fill -> deprecated
    // with av_image_fill_arrays

    // Examples of av_image_fill_arrays from
    // https://mail.gnome.org/archives/commits-list/2016-February/msg05531.html
    // https://github.com/bernhardu/dvbcut-deb/blob/master/src/avframe.cpp
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB32,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecCtx->width,
                             pCodecCtx->height,
                             pCodecCtx->pix_fmt,
                             pCodecCtx->width,
                             pCodecCtx->height,
                             AV_PIX_FMT_RGB32,
                             SWS_BILINEAR,
                             nullptr,
                             nullptr,
                             nullptr
                             );

    while (av_read_frame(pFormatCtx, packet) >= 0 && !quit) {
        // Is this a packet from the video stream?
        if (packet->stream_index == videoStream) {
            // Decode video frame
            frameFinished = 0;

            // Replace avcodec_decode_video2 -> deprecated
            // with avcodec_send_packet and avcodec_receive_frame
            // see https://github.com/pesintta/vdr-plugin-vaapidevice/issues/31
            if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO ||
                pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
                av_error = avcodec_send_packet(pCodecCtx, packet);
                if (av_error < 0 && av_error != AVERROR(EAGAIN) && av_error != AVERROR_EOF) {
                } else {
                    if (av_error >= 0)
                        packet->size = 0;
                    av_error = avcodec_receive_frame(pCodecCtx, pFrame);
                    if (av_error >= 0)
                        frameFinished = 1;
                }
            }

            // Did we get a video frame?
            if (frameFinished) {
                // Wait for VideoWidget to be ready
                // (this is necessary, since the image pointer is shared with VideoWidget!)
                mutex.lock();
                if (!continueReading && !quit) {
                    condition.wait(&mutex);
                }
                mutex.unlock();

                if (!quit) {
                    // Convert the image from its native format to RGB
                    sws_scale(sws_ctx, static_cast<uint8_t const * const *>(pFrame->data),
                              pFrame->linesize, 0, pCodecCtx->height,
                              pFrameRGB->data, pFrameRGB->linesize);

                    // At this point, the widget has already finished the update
                    // and can not interfere with the flow in this section
                    continueReading = false;

                    image = QImage((uchar*)pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                    emit newFrame(&image);

                    frame++;
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
    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    emit finished();
}

void VideoReader::next()
{
    QMutexLocker locker(&mutex);

    continueReading = true;
    condition.wakeOne();
}

void VideoReader::stop()
{
    QMutexLocker locker(&mutex);

    quit = true;
    condition.wakeOne();
}
