#ifndef VIDEOFILE_H
#define VIDEOFILE_H

#include <QString>
#include <QTemporaryFile>

class VideoFile
{
public:
    VideoFile();
    VideoFile(const QString &file_name)
    { file_path = file_name; }

    const QString &fileName() const
    { return file_path; }

    void save(const QString &file_name) const
    { QFile::copy(file_path, file_name); }

    VideoFile &operator=(const VideoFile &src)
    { file_path = src.file_path; return *this; }

private:
    QTemporaryFile temp_file;
    QString file_path;
};

#endif // VIDEOFILE_H
