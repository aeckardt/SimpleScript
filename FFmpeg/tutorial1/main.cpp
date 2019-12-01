extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <iostream>

#define video_file_path \
    "/Users/albrecht/Documents/Code/Tasc/FFmpeg/muxingtest.mp4"

#define mp3_file_path \
    "/Users/albrecht/Downloads/Amanda Jenssen - Illusionist.mp3"

#define guwu_file_path \
    "/Users/albrecht/Downloads/Episode 1 - A Gurls Wurld Full Episode - Totes Amaze ❤️ - Teen TV Shows.mp4"

#define current_file_path \
    video_file_path

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[200];
    int  y;

    // Open file
    sprintf(szFilename, "/Users/albrecht/Documents/Code/Tasc/FFmpeg/build/frames/frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == nullptr)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for(y = 0; y < height; y++)
    fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, static_cast<size_t>(width * 3), pFile);

    // Close file
    fclose(pFile);
}

int main(int /*argc*/, char **/*argv*/)
{
    // av_register_all(); // -> deprecated
    // see https://github.com/leandromoreira/ffmpeg-libav-tutorial/issues/29

    AVFormatContext *pFormatCtx = nullptr;
    AVDictionary *opt = nullptr;
    int videoStream;

    // Open video file
    if (avformat_open_input(&pFormatCtx, current_file_path, nullptr, &opt) != 0) {
        std::cerr << "Error: Could not open file" << std::endl;
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        std::cerr << "Error: Could not find stream information" << std::endl;
        return -1; // Couldn't find stream information
    }

    // Dump information about file onto standard error
//    av_dump_format(pFormatCtx, 0, current_file_path, 0);

    int i;
    AVCodecParameters *pCodecPar = nullptr;

    // Find the first video stream
    videoStream = -1;
    for (i = 0; i < static_cast<int>(pFormatCtx->nb_streams); i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }

    if (videoStream == -1) {
        std::cerr << "Error: Did not find a video stream" << std::endl;
        return -1; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecPar = pFormatCtx->streams[videoStream]->codecpar;

    AVCodec *pCodec = nullptr;
    AVCodecContext *pCodecCtx = nullptr;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecPar->codec_id);
    if (pCodec == nullptr) {
        std::cerr << "Error: Unsupported codec!" << std::endl;
        return -1; // Codec not found
    }

    // Replace avcodec_copy_context -> deprecated
    // with avcodec_parameters to context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (avcodec_parameters_to_context(pCodecCtx, pCodecPar) < 0) {
        std::cerr << "Error: Could not copy context" << std::endl;
        return -1; // Couldn't copy context
    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, &opt) < 0) {
        std::cerr << "Error: Could not open codec" << std::endl;
        return -1; // Could not open codec
    }

    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;

    // Allocate video frame
    pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == nullptr) {
        std::cerr << "Could not allocate frame" << std::endl;
        return -1;
    }

    uint8_t *buffer = nullptr;
    int numBytes;
    // Determine required buffer size and allocate buffer
    // Replace avpicture_get_size -> deprecated
    // with av_image_get_buffer_size

    // Remark: Not sure if align needs to be 1 or 32, see
    // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
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
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24,
                         pCodecCtx->width, pCodecCtx->height, 1);

    struct SwsContext *sws_ctx = nullptr;
    int frameFinished;
    AVPacket packet;

    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
        );

    int averror;

    i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            frameFinished = 0;

            // Replace avcodec_decode_video2 -> deprecated
            // with avcodec_send_packet and avcodec_receive_frame
            // see https://github.com/pesintta/vdr-plugin-vaapidevice/issues/31
             if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO ||
                 pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
                 averror = avcodec_send_packet(pCodecCtx, &packet);
                 if (averror < 0 && averror != AVERROR(EAGAIN) && averror != AVERROR_EOF) {
                } else {
                     if (averror >= 0) {
                         packet.size = 0;
                     }
                     averror = avcodec_receive_frame(pCodecCtx, pFrame);
                     if (averror >= 0)
                         frameFinished = 1;
                 }
             }

            // Did we get a video frame?
            if (frameFinished) {
                // Convert the image from its native format to RGB
                sws_scale(sws_ctx, static_cast<uint8_t const * const *>(pFrame->data),
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGB->data, pFrameRGB->linesize);

                // Save the frame to disk
                if (++i <= 1000) {
                    std::cerr << "Save frame..." << std::endl;
                    SaveFrame(pFrameRGB, pCodecCtx->width,
                              pCodecCtx->height, i);
                    std::cerr << "Frame saved!" << std::endl;
                }
            }
        }
    }

    // Unref the packet that was allocated by av_read_frame
    av_packet_unref(&packet);

    // Free buffer
    av_free(buffer);

    // Free video frame
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);

    // Close the codecs
    avcodec_close(pCodecCtx);

    // Free the codecs
    avcodec_free_context(&pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return 0;
}
