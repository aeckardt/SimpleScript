#include "video.h"

using std::vector;

void Video::clear()
{
    vector<VideoFrame *>::reverse_iterator it;
    for (it = frames.rbegin(); it != frames.rend(); ++it)
    {
        delete static_cast<VideoFrame *>(*it);
    }

    frames.clear();
}

bool Video::save(const QString &fileName) const
{
    return true;
}
