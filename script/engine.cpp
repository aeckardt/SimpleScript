#include "engine.h"

using namespace std;
using namespace tw;

#include "selectFrame/SelectFrameWidget.h"
#include "image/screenshot.h"
#include "image/video.h"
#include "imageView/ImageView.h"

#include <sys/time.h>

#include <QLabel>
#include <QScrollArea>

static ScriptEngine *pengine;

typedef ParameterObjectBase<QImage> ImageObject;
template<> ObjectReference ImageObject::ref = -1;
typedef ParameterObjectBase<Video> VideoObject;
template<> ObjectReference VideoObject::ref = -1;

bool cmdPrint(const ParameterList &params, Parameter &)
{
    if (params.empty())
    {
        pengine->print(Parameter());
        return true;
    }

    pengine->print(params[0]);
    return true;
}

bool cmdSelect(const ParameterList &, Parameter &param)
{
    pengine->mainWindow->hide();
    param.assign(SelectFrameWidget().selectRect());
    pengine->mainWindow->show();

    return true;
}

bool cmdCapture(const ParameterList &params, Parameter &param)
{
    if (params.empty())
    {
        param.assign(ImageObject(captureDesktop()));
    }
    else
    {
        const QRect &rect = params[0].asRect();
        param.assign(ImageObject(captureRect(rect)));
    }
    return param.asObject<ImageObject>().obj.size() != QSize(0, 0);
}

bool cmdView(const ParameterList &params, Parameter &)
{
    const QImage &image = params[0].asObject<ImageObject>().obj;

    ImageView imageView;
    imageView.showImage(image);

    return true;
}

bool cmdNow(const ParameterList &, Parameter &param)
{
    timeval tv;
    gettimeofday(&tv, nullptr);

    qint64 msecs = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    QDateTime dt = QDateTime::fromMSecsSinceEpoch(msecs);
    param.assign(dt);

    return true;
}

bool cmdMsecsBetween(const ParameterList &params, Parameter &param)
{
    const QDateTime &dt1 = params[0].asDateTime();
    const QDateTime &dt2 = params[1].asDateTime();

    param.assign(static_cast<int>(dt1.msecsTo(dt2)));

    return true;
}

bool cmdStr(const ParameterList &params, Parameter &param)
{
    std::stringstream ss;
    ss << params[0];
    param.assign(ss.str());

    return true;
}

bool cmdRecord(const ParameterList &params, Parameter &param)
{
    const QRect &rect    = params[0].asRect();
//    const int frame_rate = params[1].asInt();
//    if (frame_rate < 1)
//    {
//        return false;
//    }

//    int usec_interval = static_cast<int>(10000000.0 * (1.0 / static_cast<double>(frame_rate)) + 0.5);

//    timeval start, last;
//    gettimeofday(&start, nullptr);

    VideoObject video;
    video.obj.addFrame(VideoFrame(captureRect(rect)));

    param.assign(move(video));

    return true;
}

ScriptEngine::ScriptEngine(QMainWindow *parent)
    : mainWindow(parent)
{
//    ImageObject::ref = tw.createObjectReference();
//    VideoObject::ref = tw.createObjectReference();

    tw.registerCommand("print", cmdPrint,
        {{Empty, String, Int, Float, Boolean, Point, Rect, DateTime}}, Empty);

    tw.registerCommand("select", cmdSelect,
        {}, Rect);

    tw.registerCommand("capture", cmdCapture,
        {{Empty, Rect}}, {Object, ImageObject::ref});

    tw.registerCommand("view", cmdView,
        {{{Object, ImageObject::ref}}}, Empty);

    tw.registerCommand("now", cmdNow,
        {}, DateTime);

    tw.registerCommand("msecsbetween", cmdMsecsBetween,
        {{DateTime}, {DateTime}}, Int);

    tw.registerCommand("str", cmdStr,
        {{String, Int, Float, Boolean, Point, Rect, DateTime}}, String);

    tw.registerCommand("record", cmdRecord,
        {{Empty, Rect}, {Int}}, {Object, VideoObject::ref});

    pengine = this;
}
