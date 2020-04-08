#include "videofile.h"

VideoFile::VideoFile()
{
    temp_file.open();
    file_path = temp_file.fileName();
}
