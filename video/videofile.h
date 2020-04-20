#ifndef VIDEOFILE_H
#define VIDEOFILE_H

#include <QString>
#include <QTemporaryFile>

class VideoFile
{
public:
    VideoFile() {}
    VideoFile(const VideoFile& src) { *this = src; }
    VideoFile(const QString &file_name) { file_path = file_name; }

    void createTemporary() { temp_file.open(); file_path = temp_file.fileName(); }

    const QString &fileName() const { return file_path; }
    void save(const QString &file_name) const { QFile::copy(file_path, file_name); }

    VideoFile &operator=(const VideoFile &src) { file_path = src.file_path; return *this; }

private:
    QTemporaryFile temp_file;
    QString file_path;

    friend VideoFile temporaryVideo();
};

inline VideoFile temporaryVideo()
{
    VideoFile video_file;
    video_file.createTemporary();
    return video_file;
}

#endif // VIDEOFILE_H

