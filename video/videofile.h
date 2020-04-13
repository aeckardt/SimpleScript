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

    const QString &fileName() const { return file_path; }
    void save(const QString &file_name) const { QFile::copy(file_path, file_name); }

    VideoFile &operator=(const VideoFile &src) { file_path = src.file_path; return *this; }

private:
    QTemporaryFile temp_file;
    QString file_path;

    friend VideoFile createTemporaryVideoFile();
};

inline VideoFile createTemporaryVideoFile()
{
    VideoFile video_file;
    video_file.temp_file.open();
    video_file.file_path = video_file.temp_file.fileName();
    return video_file;
}

#endif // VIDEOFILE_H
