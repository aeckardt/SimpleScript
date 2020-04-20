#ifndef VIDEOFILE_H
#define VIDEOFILE_H

#include <QString>
#include <QTemporaryFile>

class VideoFile
{
public:
    VideoFile() {}
    VideoFile(const QString &file_name) { file_path = file_name; }

    void createTemporary() { temp_file.open(); file_path = temp_file.fileName(); }

    const QString &fileName() const { return file_path; }
    void save(const QString &file_name) const { QFile::copy(file_path, file_name); }

private:
    QTemporaryFile temp_file;
    QString file_path;
};

#endif // VIDEOFILE_H

