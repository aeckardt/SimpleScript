#ifndef MEMORYUSAGE_H
#define MEMORYUSAGE_H

class MemoryUsage
{
public:
    MemoryUsage();

    void retrieveInfo();

    int used() const { return _used; }
    int unused() const { return _unused; }
    int total() const { return _total; }

    int wired() const { return _wired; }

private:
    int _used;
    int _unused;
    int _total;
    int _wired;

    void clear();
};

#endif // MEMORYUSAGE_H
