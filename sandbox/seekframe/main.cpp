#include <QApplication>
#include <QSettings>

#include <iostream>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QSettings settings;
//    settings.setValue("video_file_header", "...");
//    settings.setValue("video_file_noheader", "...");

    std::string video_file_path = settings.value("video_file_noheader").toString().toStdString();
    fprintf(stdout, "The video file path is: %s\n", video_file_path.c_str());

    int av_error;

    AVFormatContext *format_ctx = nullptr;
    av_error = avformat_open_input(&format_ctx, video_file_path.c_str(), nullptr, nullptr);
    if (av_error < 0) {
        fprintf(stderr, "Could not open video file.\n");
        return -1;
    }


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

    return 0;
}
