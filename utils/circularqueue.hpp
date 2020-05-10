#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H

#include <QMutex>
#include <vector>

template<typename T>
class CircularQueue
{
public:
    CircularQueue(size_t obj_size);

    void resize(size_t val);

    void push(T &val);
    void pop(T &val);

    inline bool empty() { QMutexLocker locker(&mutex); return total_size == 0; }
    inline bool full() { QMutexLocker locker(&mutex); return total_size == max_size; }
    inline size_t size() { QMutexLocker locker(&mutex); return total_size; }

protected:
    size_t obj_size;

private:
    // Make two-dimensional array in order to deal with problems,
    // that arise, when large vectors are used
    std::vector<std::vector<T>> data;

    size_t vector_size;
    size_t total_size;
    size_t max_size;

    size_t front_vector;
    size_t front_index;
    size_t back_vector;
    size_t back_index;

    QMutex mutex;
};

template<typename T>
CircularQueue<T>::CircularQueue(size_t obj_size)
    : obj_size(obj_size),
      total_size(0),
      max_size(0),
      front_vector(0),
      front_index(0),
      back_vector(0),
      back_index(0)
{
    // max vector size is to store elements of 64 MB size
    vector_size = (1 << 26) / obj_size;
}

template<typename T>
void CircularQueue<T>::resize(size_t val)
{
    QMutexLocker locker(&mutex);

    max_size = val;
    total_size = 0;
    back_vector = 0;
    back_index = 0;
    front_vector = 0;
    front_index = 0;

    data.clear();
    data.resize((max_size + vector_size - 1) / vector_size, std::vector<T>());

    for (std::vector<T> &vec : data)
        vec.resize(vector_size, T());
}

template<typename T>
void CircularQueue<T>::push(T &val)
{
    QMutexLocker locker(&mutex);

    if (total_size == max_size)
        return;

    data[back_vector][back_index] = std::move(val);

    back_index++;
    if (back_index == vector_size) {
        back_vector = (back_vector + 1) % data.size();
        back_index = 0;
    }

    total_size++;
}

template<typename T>
void CircularQueue<T>::pop(T &val)
{
    QMutexLocker locker(&mutex);

    if (total_size == 0)
        return;

    val = std::move(data[front_vector][front_index]);

    front_index++;
    if (front_index == vector_size) {
        front_vector = (front_vector + 1) % data.size();
        front_index = 0;
    }

    total_size--;
}

#endif // CIRCULARQUEUE_H
