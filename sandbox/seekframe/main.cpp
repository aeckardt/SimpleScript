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
static const AVCodecID codec_id = AV_CODEC_ID_H264;
static const char *codec_name = "libx264rgb";

int createVideoFile(const char *file_name, int width, int height, int framecount, int framerate)
{
    av_log_set_level(AV_LOG_DEBUG);

    AVOutputFormat *output_fmt = av_guess_format("avi", nullptr, nullptr);
    if (output_fmt == nullptr) {
        fprintf(stderr, "Could not guess format.\n");
        return -1;
    }

    fflush(stderr);

    AVFormatContext *format_ctx = nullptr;

    int av_error = avformat_alloc_output_context2(&format_ctx, output_fmt, nullptr, file_name);
    if (av_error < 0) {
        fprintf(stderr, "Could not open video file format context.\n");
        return av_error;
    }

    fflush(stderr);

    AVStream *stream = avformat_new_stream(format_ctx, nullptr);

    stream->id = format_ctx->nb_streams - 1;
    stream->nb_frames = framecount;
    stream->codecpar->codec_id = codec_id;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->width = width;
    stream->codecpar->height = height;
    stream->codecpar->format = pix_fmt;
    stream->time_base = AVRational{1, framerate};

    fflush(stderr);

    // Find the decoder for the video stream
    AVCodec *codec = avcodec_find_encoder_by_name(codec_name);
    if (codec == nullptr) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    fflush(stderr);

    // Copy context
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codec_ctx, stream->codecpar) < 0) {
        fprintf(stderr, "Could not copy codec parameters to context.\n");
        return -1;
    }
    codec_ctx->time_base = AVRational{1, framerate};
    codec_ctx->max_b_frames = 2;
    codec_ctx->gop_size = 12;
    if (codec_id == AV_CODEC_ID_H264) {
        av_opt_set(codec_ctx->priv_data, "preset", "fast", 0);
        av_opt_set(codec_ctx->priv_data, "crf",    "15",   0);
    }
    if (format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
         codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    fflush(stderr);

    av_error = avcodec_parameters_from_context(stream->codecpar, codec_ctx);
    if (av_error < 0) {
        fprintf(stderr, "Could not copy codec context back to codec parameters.\n");
        return av_error;
    }

    fflush(stderr);

    // Open codec
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        fprintf(stderr, "Could not open codec.\n");
        return -1;
    }

    fflush(stderr);

    if (!(output_fmt->flags & AVFMT_NOFILE)) {
        av_error = avio_open(&format_ctx->pb, file_name, AVIO_FLAG_WRITE);
        if (av_error < 0) {
            fprintf(stderr, "Could not open output format.\n");
            return av_error;
        }
    } else
        fprintf(stderr, "Warning: Output format is not a file.\n");

    fflush(stderr);

    av_error = avformat_write_header(format_ctx, nullptr);
    if (av_error < 0) {
        fprintf(stderr, "Could not write format header.\n");
        return av_error;
    }

    fprintf(stderr, "Stream timebase is: (%d, %d).\n", stream->time_base.num, stream->time_base.den);

    av_dump_format(format_ctx, 0, file_name, 1);

    fflush(stderr);

    AVFrame *frame = av_frame_alloc();
    if (frame == nullptr) {
        fprintf(stderr, "Could not allocate frame.\n");
        return -1;
    }
    frame->format = pix_fmt;
    frame->width = width;
    frame->height = height;

    int linesize_alignment = 1;

    size_t num_bytes = av_image_get_buffer_size(pix_fmt, width, height, linesize_alignment);

    fflush(stderr);

    uint8_t *buffer = static_cast<uint8_t*>(av_malloc(num_bytes));
    if (buffer == nullptr) {
        fprintf(stderr, "Could not allocate frame buffer.\n");
        return -1;
    }

    av_image_fill_arrays(frame->data, frame->linesize, buffer, pix_fmt,
                         width, height, linesize_alignment);

    Image image(linesize_alignment);
    image.assign(frame->data[0], width, height);

    AVPacket pkt;

    fflush(stderr);

    int index;
    for (index = 0; index < framecount; index++) {
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;

        fillImage(image, index);

        // The time_base may be changed by avformat_write_header
        frame->pts = static_cast<int64_t>(static_cast<double>(index) /
                                         (static_cast<double>(framerate) * av_q2d(stream->time_base)));
        av_error = avcodec_send_frame(codec_ctx, frame);
        if (av_error < 0) {
            fprintf(stderr, "Could not send frame.\n");
            return av_error;
        }

        fflush(stderr);

        av_error = avcodec_receive_packet(codec_ctx, &pkt);
        if (av_error < 0) {
            if (av_error != AVERROR(EAGAIN)) {
                fprintf(stderr, "Could not receive packet.\n");
                return av_error;
            }
        }

        if (av_error == 0) {
            pkt.stream_index = stream->index;

            av_error = av_write_frame(format_ctx, &pkt);
            if (av_error < 0) {
                fprintf(stderr, "Could not write frame.\n");
                return av_error;
            }

            av_packet_unref(&pkt);
        }
    }

    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    // Flushing...
    for (;;) {
        pkt.pts++;

        av_error = avcodec_send_frame(codec_ctx, nullptr);
        if (av_error < 0) {
            if (av_error != AVERROR(EAGAIN) && av_error != AVERROR_EOF) {
                fprintf(stderr, "Could not send frame (flushing).\n");
                return av_error;
            }
        }

        fflush(stderr);

        if (avcodec_receive_packet(codec_ctx, &pkt) == 0) {
            av_error = av_write_frame(format_ctx, &pkt);
            if (av_error < 0) {
                fprintf(stderr, "Could not write frame (flushing).\n");
                return av_error;
            }
            av_packet_unref(&pkt);
        }
        else {
            break;
        }

        fflush(stderr);
    }

    av_write_trailer(format_ctx);
    avio_close(format_ctx->pb);

    fflush(stderr);

    av_frame_free(&frame);
    av_freep(&buffer);
    avcodec_free_context(&codec_ctx);
    avformat_free_context(format_ctx);

    fflush(stderr);

    if (av_error == AVERROR_EOF)
        return 0;
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

    int av_error = createVideoFile(file_name, 352, 288, 600, 25);
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
    fprintf(stderr, ">>>> Probesize is changed to: %lld, trying to find stream information...\n", format_ctx->probesize);

    fflush(stderr);

    av_error = avformat_find_stream_info(format_ctx, nullptr);
    if (av_error < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }

    fflush(stderr);

    fprintf(stderr, ">>>> Going through streams...\n");

    fflush(stderr);

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

    fflush(stderr);

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
    fprintf(stdout, "- avg_frame_rate: (%d, %d)\n", stream->avg_frame_rate.num, stream->avg_frame_rate.den);
    fprintf(stdout, "- codec -> codec_type: %d\n", stream->codecpar->codec_type);
    fprintf(stdout, "- codec -> codec_id: %d\n", stream->codecpar->codec_id);
    fprintf(stdout, "- codec -> pix_fmt: %d\n", stream->codecpar->format);
    fprintf(stdout, "- codec -> width: %d\n", stream->codecpar->width);
    fprintf(stdout, "- codec -> height: %d\n", stream->codecpar->height);

    avformat_close_input(&format_ctx);

    // ------------------------------------------------------------------------------------------------------------ //

    return 0;
}
