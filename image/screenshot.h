#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QImage>
#include <QRect>
#include <QByteArray>

void captureDesktop(QImage &dest, QImage::Format format = QImage::Format_RGB32);
void captureRect(const QRect &, QImage &dest, QImage::Format format = QImage::Format_RGB32);

inline QImage captureDesktop(QImage::Format format = QImage::Format_RGB32)
{ QImage dest; captureDesktop(dest, format); return dest; }

inline QImage captureRect(const QRect &rect, QImage::Format format = QImage::Format_RGB32)
{ QImage dest; captureRect(rect, dest, format); return dest; }

#endif // SCREENSHOT_H
