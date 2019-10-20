#ifndef CV_H
#define CV_H

#include <iostream>

namespace cv
{

struct Point
{
    int32_t x;
    int32_t y;
};

inline std::ostream &operator <<(std::ostream &os, const Point &pt) { return os << "(" << pt.x << ", " << pt.y << ")"; }
inline Point operator +(const Point &pt1, const Point &pt2) { return {pt1.x + pt2.x, pt1.y + pt2.y}; }
inline Point operator -(const Point &pt1, const Point &pt2) { return {pt1.x - pt2.x, pt1.y - pt2.y}; }
inline Point operator *(const Point &pt, const double &d) { return {static_cast<int32_t>(static_cast<double>(pt.x) * d), static_cast<int32_t>(static_cast<double>(pt.y) * d)}; }
inline Point operator *(const double &d, const Point &pt) { return {static_cast<int32_t>(static_cast<double>(pt.x) * d), static_cast<int32_t>(static_cast<double>(pt.y) * d)}; }
inline Point operator *(const Point &pt, const int32_t &i) { return {pt.x * i, pt.y * i}; }
inline Point operator *(const int32_t &i, const Point &pt) { return {pt.x * i, pt.y * i}; }
inline Point operator /(const Point &pt, const double &d) { return {static_cast<int32_t>(static_cast<double>(pt.x) / d), static_cast<int32_t>(static_cast<double>(pt.y) / d)}; }
inline Point operator /(const Point &pt, const int32_t &i) { return {pt.x / i, pt.y / i}; }
inline bool operator ==(const Point &pt1, const Point &pt2) { return pt1.x == pt2.x && pt1.y == pt2.y; }
inline bool operator !=(const Point &pt1, const Point &pt2) { return pt1.x != pt2.x || pt1.y != pt2.y; }

}

#endif // CV_H
