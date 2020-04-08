#include "engine.h"

#include "frameSelector/selectframewidget.h"
#include "image/screenshot.h"
#include "image/imageviewer.h"
#include "video/player.h"
#include "video/recorder.h"
#include "video/videofile.h"

#include <QEventLoop>
#include <QFileDialog>
#include <QTimer>

using namespace tw;

static ScriptEngine *engine;

enum : ObjectReference
{
    ImageRef,
    VideoRef
};

template<> ObjectReference ParameterObjectBase<QImage>::ref = ImageRef;
template<> ObjectReference ParameterObjectBase<VideoFile>::ref  = VideoRef;

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

bool cmdLoadImage(const ParameterList &in_params, Parameter &out_param)
{
    QString file_name;
    if (!in_params.empty())
        file_name = in_params[0].asString().c_str();
    else
        file_name = QFileDialog::getOpenFileName(nullptr,
            QObject::tr("Load image"), "",
            QObject::tr("Portable Network Graphics (*.png);;All files (*)"));

    if (!QFileInfo::exists(file_name)) {
        engine->printError("File does not exist");
        return false;
    }

    out_param.createObject<QImage>(file_name);

    return true;
}

bool cmdLoadVideo(const ParameterList &in_params, Parameter &out_param)
{
    QString file_name;
    if (!in_params.empty())
        file_name = in_params[0].asString().c_str();
    else
        file_name = QFileDialog::getOpenFileName(nullptr,
            QObject::tr("Load video"), "",
            QObject::tr("H264 video file (*.h264);;All files (*)"));

    if (!QFileInfo::exists(file_name)) {
        engine->printError("File does not exist");
        return false;
    }

    out_param.createObject<VideoFile>(file_name);

    return true;
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

    ScreenRecorder recorder(out_param.createObject<VideoFile>());
    recorder.exec(rect, frame_rate);

    engine->mainWindow->show();

    return true;
}

bool cmdSave(const ParameterList &in_params, Parameter &)
{
    switch (in_params[0].objectRef()) {
    case ImageRef: {
        const QImage &image = in_params[0].asObject<QImage>();

        QString fileName;
        if (in_params.size() == 2 && in_params[1].type() == String)
            fileName = in_params[1].asString().c_str();
        else
            fileName = QFileDialog::getSaveFileName(nullptr,
                QObject::tr("Save image"), "",
                QObject::tr("Portable Network Graphics (*.png);;All files (*)"));

        if (!fileName.isEmpty())
            image.save(fileName, "PNG");

        return true;
    }
    case VideoRef: {
        const VideoFile &video = in_params[0].asObject<VideoFile>();

        QString fileName;
        if (in_params.size() == 2 && in_params[1].type() == String)
            fileName = in_params[1].asString().c_str();
        else
            fileName = QFileDialog::getSaveFileName(nullptr,
                QObject::tr("Save video"), "",
                QObject::tr("H264 video file (*.h264);;All files (*)"));

        if (!fileName.isEmpty())
            video.save(fileName);

        return true;
    }
    default:
        return false;
    }
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
        const VideoFile &video = in_params[0].asObject<VideoFile>();

        VideoPlayer video_player;
        video_player.runVideo(video);

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
    tw.registerObject<VideoFile>("Video", true, false);

    tw.registerCommand("capture", cmdCapture,
        {{Empty, Rect}}, ImageRef);

    tw.registerCommand("loadImage", cmdLoadImage,
        {{Empty, String}}, ImageRef);

    tw.registerCommand("loadVideo", cmdLoadVideo,
        {{Empty, String}}, VideoRef);

    tw.registerCommand("msecsbetween", cmdMsecsBetween,
        {{DateTime}, {DateTime}}, Int);

    tw.registerCommand("now", cmdNow,
        {}, DateTime);

    tw.registerCommand("print", cmdPrint,
        {{Empty, String, Int, Float, Boolean, Point, Rect, DateTime}}, Empty);

    tw.registerCommand("record", cmdRecord,
        {{Rect}, {Int}}, VideoRef);

    tw.registerCommand("save", cmdSave,
        {{ImageRef, VideoRef}, {Empty, String}}, Empty);

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
