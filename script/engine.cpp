#include "engine.h"

#include "selectFrame/SelectFrameWidget.h"
#include "image/screenshot.h"
#include "imageViewer/ImageViewer.h"
#include "video/recorder.h"
#include "video/video.h"
#include "videoPlayer/VideoPlayer.h"

#include <QEventLoop>
#include <QTimer>

using namespace tw;

static ScriptEngine *engine;

enum : ObjectReference
{
    ImageRef,
    VideoRef
};

template<> ObjectReference ParameterObjectBase<QImage>::ref = ImageRef;
template<> ObjectReference ParameterObjectBase<Video>::ref  = VideoRef;

bool cmdCapture(const ParameterList &in_params, Parameter &out_param)
{
    if (in_params.empty())
        out_param.createObject<QImage>(captureDesktop());
    else {
        const QRect &rect = in_params[0].asRect();
        out_param.createObject<QImage>(captureRect(rect));
    }
    return out_param.asObject<QImage>().size() != QSize(0, 0);
}

bool cmdMsecsBetween(const ParameterList &in_params, Parameter &out_param)
{
    const QDateTime &dt1 = in_params[0].asDateTime();
    const QDateTime &dt2 = in_params[1].asDateTime();

    out_param.assign(static_cast<int>(dt1.msecsTo(dt2)));

    return true;
}

bool cmdNow(const ParameterList &, Parameter &out_param)
{
    out_param.assign(QDateTime::currentDateTime());
    return true;
}

bool cmdPrint(const ParameterList &in_params, Parameter &)
{
    if (in_params.empty()) {
        engine->print(Parameter());
        return true;
    }

    engine->print(in_params[0]);
    return true;
}

bool cmdRecord(const ParameterList &in_params, Parameter &out_param)
{
    const QRect &rect = in_params[0].asRect();
    int frame_rate    = in_params[1].asInt();

    if (frame_rate < 1 || frame_rate > 30) {
        engine->printError("Frame rate needs to be between 1 and 30");
        return false;
    }

    engine->mainWindow->hide();

    ScreenRecorder recorder;
    recorder.exec(rect, out_param.createObject<Video>(), frame_rate);

    engine->mainWindow->show();

    return true;
}

bool cmdSelect(const ParameterList &, Parameter &out_param)
{
    engine->mainWindow->hide();

    out_param.assign(SelectFrameWidget().selectRect());

    engine->mainWindow->show();

    return true;
}

bool cmdSleep(const ParameterList &in_params, Parameter &)
{
    useconds_t msec;
    switch (in_params[0].type()) {
    case Int:
        msec = static_cast<useconds_t>(in_params[0].asInt());
        break;
    case Float:
        msec = static_cast<useconds_t>(in_params[0].asFloat());
        break;
    default:
        msec = 0;
        break;
    }

    QEventLoop loop;

    // Not very accurate, usually off by 1-6 milliseconds
    // TODO: Find a better way of implementing 'sleep'
    QTimer::singleShot(msec, Qt::PreciseTimer, &loop, &QEventLoop::quit);
    loop.exec();

    return true;
}

bool cmdStr(const ParameterList &in_params, Parameter &out_param)
{
    std::stringstream ss;
    ss << in_params[0];
    out_param.assign(ss.str());

    return true;
}

bool cmdView(const ParameterList &in_params, Parameter &)
{
    switch (in_params[0].objectRef()) {
    case ImageRef: {
        const QImage &image = in_params[0].asObject<QImage>();

        ImageViewer image_viewer;
        image_viewer.showImage(image);

        return true;
    }
    case VideoRef: {
        const Video &video = in_params[0].asObject<Video>();

        VideoPlayer video_player;
        video_player.runVideo(video.path());

        return true;
    }
    default:
        return false;
    }
}

ScriptEngine::ScriptEngine(QMainWindow *parent)
    : mainWindow(parent)
{
    tw.registerObject<QImage>("Image", true, false);
    tw.registerObject<Video>("Video", true, false);

    tw.registerCommand("capture", cmdCapture,
        {{Empty, Rect}}, ImageRef);

    tw.registerCommand("msecsbetween", cmdMsecsBetween,
        {{DateTime}, {DateTime}}, Int);

    tw.registerCommand("now", cmdNow,
        {}, DateTime);

    tw.registerCommand("print", cmdPrint,
        {{Empty, String, Int, Float, Boolean, Point, Rect, DateTime}}, Empty);

    tw.registerCommand("record", cmdRecord,
        {{Rect}, {Int}}, VideoRef);

    tw.registerCommand("select", cmdSelect,
        {}, Rect);

    tw.registerCommand("sleep", cmdSleep,
        {{Int, Float}}, Empty);

    tw.registerCommand("str", cmdStr,
        {{String, Int, Float, Boolean, Point, Rect, DateTime}}, String);

    tw.registerCommand("view", cmdView,
        {{ImageRef, VideoRef}}, Empty);

    engine = this;
}
