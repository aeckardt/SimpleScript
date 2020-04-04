#include "engine.h"

#include "selectFrame/SelectFrameWidget.h"
#include "image/screenshot.h"
#include "imageView/ImageView.h"
#include "video/video.h"

#include <QDir>
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

bool cmdRecord(const ParameterList &, Parameter &)
{
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

    QTimer timer;
    QEventLoop loop;

    // Not very accurate, usually off by 3-10 milliseconds
    // TODO: Find a better way of implementing 'sleep'
    timer.singleShot(msec, &loop, &QEventLoop::quit);
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

        ImageView imageView;
        imageView.showImage(image);

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
        {{Empty, Rect}}, VideoRef);

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
