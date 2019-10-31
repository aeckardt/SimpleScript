#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QImage>
#include <QRect>

QImage captureDesktop();
QImage captureRect(const QRect &);

#endif // SCREENSHOT_H
