#include <QApplication>
#include <QSettings>

#include <iostream>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include "image/image.h"
#include "tests/createimage.h"

static const AVPixelFormat pix_fmt = AV_PIX_FMT_BGR0;

int createVideoFile(const char *file_name, int width, int height, int framerate)
{
    av_log_set_level(AV_LOG_DEBUG);

    AVOutputFormat *output_fmt = av_guess_format("mp4", nullptr, nullptr);
    AVFormatContext *format_ctx = nullptr;

    int av_error = avformat_alloc_output_context2(&format_ctx, output_fmt, nullptr, file_name);
    if (av_error < 0) {
        fprintf(stderr, "Could not open video file format context.\n");
        return av_error;
    }

    AVStream *stream = avformat_new_stream(format_ctx, nullptr);

    stream->id = format_ctx->nb_streams - 1;
    stream->nb_frames = 0;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->width = width;
    stream->codecpar->height = height;
    stream->codecpar->codec_id = AV_CODEC_ID_H264;
    stream->codecpar->format = pix_fmt;
    stream->time_base = AVRational{1, framerate};
    stream->r_frame_rate = AVRational{framerate, 1};

    avio_open(&format_ctx->pb, file_name, AVIO_FLAG_WRITE);

    av_error = avformat_write_header(format_ctx, nullptr);
    if (av_error < 0) {
        fprintf(stderr, "Could not write format header.\n");
        return av_error;
    }

    AVFrame *frame = av_frame_alloc();
    if (frame == nullptr) {
        fprintf(stderr, "Could not allocate frame.\n");
        return -1;
    }

    size_t num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 1);

    uint8_t *buffer = static_cast<uint8_t*>(av_malloc(num_bytes));
    if (buffer == nullptr) {
        fprintf(stderr, "Could not allocate frame buffer.\n");
        return -1;
    }

    av_image_fill_arrays(frame->data, frame->linesize, buffer, AV_PIX_FMT_RGB32,
                         width, height, 1);

    Image image;
    image.assign(frame->data[0], width, height);
    fillImage(image, 0);

    // Find the decoder for the video stream
    AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (codec == nullptr) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    // Copy context
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codec_ctx, stream->codecpar) < 0) {
        fprintf(stderr, "Could not copy codec parameters to context.\n");
        return -1;
    }

    // Open codec
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        fprintf(stderr, "Could not open codec.\n");
        return -1;
    }

    AVPacket *pkt = av_packet_alloc();
    if (pkt == nullptr) {
        fprintf(stderr, "Could not allocate packet.\n");
        return -1;
    }

    av_error = avcodec_send_frame(codec_ctx, frame);
    if (av_error < 0) {
        fprintf(stderr, "Could not send frame.\n");
        return av_error;
    }
    av_error = avcodec_receive_packet(codec_ctx, pkt);
    if (av_error < 0) {
        fprintf(stderr, "Could not receive packet.\n");
        return av_error;
    }

    av_error = av_write_frame(format_ctx, pkt);
    if (av_error < 0) {
        fprintf(stderr, "Could not write frame.\n");
        return av_error;
    }

    av_packet_unref(pkt);
    av_packet_free(&pkt);
    av_frame_free(&frame);

    av_write_trailer(format_ctx);
    avio_close(format_ctx->pb);
    avformat_free_context(format_ctx);

    return av_error;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QSettings settings;
//    settings.setValue("video_file_header", "...");
//    settings.setValue("video_file_noheader", "...");
//    settings.setValue("video_file_createheader", "...");

    QString video_file_createheader = settings.value("video_file_createheader").toString();

    std::string file_name_str = video_file_createheader.toStdString();
    const char *file_name = file_name_str.c_str();

    int av_error = createVideoFile(file_name, 540, 405, 25);
    if (av_error < 0) {
        fprintf(stderr, "Error creating video file.\n");
        return -1;
    }

    std::string video_file_path = video_file_createheader.toStdString();
    fprintf(stdout, "The video file path is: %s\n", video_file_path.c_str());

    AVFormatContext *format_ctx = nullptr;
    av_error = avformat_open_input(&format_ctx, video_file_path.c_str(), nullptr, nullptr);
    if (av_error < 0) {
        fprintf(stderr, "Could not open video file.\n");
        return -1;
    }

    av_dump_format(format_ctx, 0, file_name, 0);

    format_ctx->probesize = 40000000;
    fprintf(stdout, "Probesize is changed to: %lld\n", format_ctx->probesize);

    av_error = avformat_find_stream_info(format_ctx, nullptr);
    if (av_error < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }

    fprintf(stdout, "Going through streams...\n");

    // Find the first video stream
    size_t nb_stream;
    int video_stream = -1;
    for (nb_stream = 0; nb_stream < format_ctx->nb_streams; nb_stream++) {
        const AVStream *stream = format_ctx->streams[nb_stream];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = nb_stream;
            break;
        }
    }

    if (video_stream == -1) {
        fprintf(stderr, "Could not find a video stream\n");
        return -1;
    }

    AVStream *stream = format_ctx->streams[video_stream];
    fprintf(stdout, "- id: %d\n", stream->id);
    fprintf(stdout, "- index: %d\n", stream->index);
    fprintf(stdout, "- cur_dts: %lld\n", stream->cur_dts);
    fprintf(stdout, "- duration: %lld\n", stream->duration);
    fprintf(stdout, "- first_dts: %lld\n", stream->first_dts);
    fprintf(stdout, "- nb_frames: %lld\n", stream->nb_frames);
    fprintf(stdout, "- time_base: (%d, %d)\n", stream->time_base.num, stream->time_base.den);
    fprintf(stdout, "- start_time: %lld\n", stream->start_time);
    fprintf(stdout, "- dispositon: %d\n", stream->disposition);
    fprintf(stdout, "- dts_ordered: %d\n", stream->dts_ordered);
    fprintf(stdout, "- r_frame_rate: (%d, %d)\n", stream->r_frame_rate.num, stream->r_frame_rate.den);

    int64_t divisor = int64_t(AV_TIME_BASE) * stream->r_frame_rate.den;
    int framecount = ((format_ctx->duration - 1) * stream->r_frame_rate.num + divisor - 1) / divisor;

    fprintf(stdout, "Calculated framcount is: %d\n", framecount);

    avformat_close_input(&format_ctx);

    // ------------------------------------------------------------------------------------------------------------ //

    return 0;
}
