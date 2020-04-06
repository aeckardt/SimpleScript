#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QImage>
#include <QRect>
#include <QByteArray>

void captureDesktop(QImage &dest, int linesize_align = 0, QImage::Format format = QImage::Format_RGB32);
void captureRect(const QRect &, QImage &dest, int linesize_align = 0, QImage::Format format = QImage::Format_RGB32);

inline QImage captureDesktop(int linesize_align = 0, QImage::Format format = QImage::Format_RGB32)
{ QImage dest; captureDesktop(dest, linesize_align, format); return dest; }

inline QImage captureRect(const QRect &rect, int linesize_align = 0, QImage::Format format = QImage::Format_RGB32)
{ QImage dest; captureRect(rect, dest, linesize_align, format); return dest; }

#endif // SCREENSHOT_H
