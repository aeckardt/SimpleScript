#include "engine.h"

using namespace std;
using namespace tw;

#include "selectFrame/SelectFrameWidget.h"
#include "image/screenshot.h"
#include "image/video.h"
#include "image/recorder.h"
#include "imageView/ImageView.h"
#include "videoView/VideoView.h"

static ScriptEngine *engine;

enum : ObjectReference
{
    ImageRef,
    VideoRef
};

template<> ObjectReference ParameterObjectBase<QImage>::ref = ImageRef;
template<> ObjectReference ParameterObjectBase<Video>::ref  = VideoRef;

bool cmdCapture(const ParameterList &params, Parameter &param)
{
    if (params.empty()) {
        param.createObject<QImage>(captureDesktop());
    } else {
        const QRect &rect = params[0].asRect();
        param.createObject<QImage>(captureRect(rect));
    }
    return param.asObject<QImage>().size() != QSize(0, 0);
}

bool cmdMsecsBetween(const ParameterList &params, Parameter &param)
{
    const QDateTime &dt1 = params[0].asDateTime();
    const QDateTime &dt2 = params[1].asDateTime();

    param.assign(static_cast<int>(dt1.msecsTo(dt2)));

    return true;
}

bool cmdNow(const ParameterList &, Parameter &param)
{
    param.assign(QDateTime::currentDateTime());
    return true;
}

bool cmdPrint(const ParameterList &params, Parameter &)
{
    if (params.empty()) {
        engine->print(Parameter());
        return true;
    }

    engine->print(params[0]);
    return true;
}

bool cmdRecord(const ParameterList &params, Parameter &param)
{
    const QRect &rect = params[0].asRect();
    const int frame_rate = params[1].asInt();

    if (frame_rate < 1 || frame_rate > 30) {
        engine->printError("Frame rate needs to be between 1 and 30");
        return false;
    }

    engine->mainWindow->hide();

    Recorder recorder;
    recorder.exec(rect, param.createObject<Video>(), frame_rate);

    engine->mainWindow->show();

    return true;
}

bool cmdSelect(const ParameterList &, Parameter &param)
{
    engine->mainWindow->hide();

    param.assign(SelectFrameWidget().selectRect());

    engine->mainWindow->show();

    return true;
}

#include <unistd.h>

bool cmdSleep(const ParameterList &params, Parameter &)
{
    useconds_t msec;
    switch (params[0].type()) {
    case Int:
        msec = static_cast<useconds_t>(params[0].asInt());
        break;
    case Float:
        msec = static_cast<useconds_t>(params[0].asFloat());
        break;
    default:
        msec = 0;
        break;
    }

    QTimer timer;
    QEventLoop loop;

    timer.singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();

    return true;
}

bool cmdStr(const ParameterList &params, Parameter &param)
{
    std::stringstream ss;
    ss << params[0];
    param.assign(ss.str());

    return true;
}

bool cmdView(const ParameterList &params, Parameter &)
{
    switch (params[0].objectRef()) {
    case ImageRef: {
        const QImage &image = params[0].asObject<QImage>();

        ImageView imageView;
        imageView.showImage(image);

        return true;
    }
    case VideoRef: {
        const Video &video = params[0].asObject<Video>();

        VideoView videoView;
        videoView.showVideo(video);

        return true;
    }
    default:
        return false;
    }
}

ScriptEngine::ScriptEngine(QMainWindow *parent)
    : mainWindow(parent)
{
    tw.registerObject<QImage>("Image");
    tw.registerObject<Video>("Video");

    tw.registerCommand("capture", cmdCapture,
        {{Empty, Rect}}, ImageRef);

    tw.registerCommand("msecsbetween", cmdMsecsBetween,
        {{DateTime}, {DateTime}}, Int);

    tw.registerCommand("now", cmdNow,
        {}, DateTime);

    tw.registerCommand("print", cmdPrint,
        {{Empty, String, Int, Float, Boolean, Point, Rect, DateTime}}, Empty);

    tw.registerCommand("record", cmdRecord,
        {{Empty, Rect}, {Int}}, VideoRef);

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
