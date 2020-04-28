#ifdef _WIN32

#include "memoryusage.h"

MemoryUsage::MemoryUsage() :
    _used(0),
    _unused(0),
    _total(0),
    _wired(0)
{
}

void MemoryUsage::retrieveInfo()
{
}

void MemoryUsage::clear()
{
    _total = 0;
    _used = 0;
    _wired = 0;
    _unused = 0;
}


#endif // _WIN32
