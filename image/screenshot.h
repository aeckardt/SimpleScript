#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QImage>
#include <QRect>
#include <QByteArray>

QImage captureDesktop(QImage::Format format = QImage::Format_RGB32);
QImage captureRect(const QRect &, QImage::Format format = QImage::Format_RGB32);

QByteArray captureCompressed(const QRect &, QImage::Format = QImage::Format_RGB32);

#endif // SCREENSHOT_H
