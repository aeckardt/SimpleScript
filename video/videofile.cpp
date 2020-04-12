#include "videofile.h"

VideoFile::VideoFile(bool create_temp_file)
{
    if (create_temp_file) {
        temp_file.open();
        file_path = temp_file.fileName();
    }
}
