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
typedef ParameterObjectBase<Video> VideoObject;

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
//    ImageObject *imgObj;
//    if (params.empty())
//    {
//        imgObj = new ImageObject(captureDesktop());
//        return param.asObject<ImageObject>().image.size() != QSize(0, 0);
//    }
//    else
//    {
//        const QRect &rect = params[0].asRect();
//        param.asObject<ImageObject>().image = captureRect(rect);
//        return captureRect(rect);
//    }
//    param.assignObject<ImageObject>(imgObj);
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

    param.assignObject(move(video));

    return true;
}

bool cmdShowVid(const ParameterList &params, Parameter &)
{
    const Video &video = params[0].asObject<VideoObject>().obj;

    ImageView imageView;
    imageView.showImage(video.frame(0).image());

    return true;
}

ScriptEngine::ScriptEngine(QMainWindow *parent)
    : mainWindow(parent)
{
    tw.registerCommand("print",
        {{Empty, String, Int, Float, Boolean, Point, Rect, DateTime}}, Empty,
        cmdPrint);

    tw.registerCommand("select",
        {}, Rect,
        cmdSelect);

    tw.registerCommand("capture",
        {{Empty, Rect}}, Object,
        cmdCapture);

    tw.registerCommand("view",
        {{Object}}, Empty,
        cmdView);

    tw.registerCommand("now",
        {}, DateTime,
        cmdNow);

    tw.registerCommand("msecsbetween",
        {{DateTime}, {DateTime}}, Int,
        cmdMsecsBetween);

    tw.registerCommand("str",
        {{String, Int, Float, Boolean, Point, Rect, DateTime}}, String,
        cmdStr);

    tw.registerCommand("record",
        {{Empty, Rect}, {Int}}, Object,
        cmdRecord);

    tw.registerCommand("showvid",
        {{Object}}, Empty,
        cmdShowVid);

    pengine = this;
}
