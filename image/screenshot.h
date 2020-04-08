#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QImage>
#include <QRect>
#include <QByteArray>

void captureDesktop(QImage &dest, int linesize_align = 0, QImage::Format format = QImage::Format_RGB32);
void captureRect(const QRect &, QImage &dest, int linesize_align = 0, QImage::Format format = QImage::Format_RGB32);

inline QImage captureDesktop(QImage::Format format = QImage::Format_RGB32)
{ QImage dest; captureDesktop(dest, 0, format); return dest; }

inline QImage captureRect(const QRect &rect, QImage::Format format = QImage::Format_RGB32)
{ QImage dest; captureRect(rect, dest, 0, format); return dest; }

#endif // SCREENSHOT_H
