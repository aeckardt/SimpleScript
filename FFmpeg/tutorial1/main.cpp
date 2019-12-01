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

#define current_file_path \
    video_file_path

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
    // Remark: Not sure if align needs to be 1 or 32, see
    // https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
                                        pCodecCtx->height, 1);
    buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(numBytes) * sizeof(uint8_t)));

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
