#ifdef _WIN32

#include "memoryusage.h"

#include <QString>
#include <QDebug>

#include <sysinfoapi.h>

MemoryUsage::MemoryUsage() :
    _used(0),
    _unused(0),
    _total(0),
    _wired(0)
{
}

void MemoryUsage::retrieveInfo()
{
    QString system_info;

    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status)) {
        _total = static_cast<int>(memory_status.ullTotalPhys / (1024 * 1024));
        _unused = static_cast<int>(memory_status.ullAvailPhys / (1024 * 1024));
        _used = _total - _unused;
    }
}

void MemoryUsage::clear()
{
    _total = 0;
    _used = 0;
    _wired = 0;
    _unused = 0;
}

#endif // _WIN32
