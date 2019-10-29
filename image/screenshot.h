#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QImage>
#include <QRect>

bool captureDesktop(QImage &);
bool captureRect(QImage &, const QRect &);

#endif // SCREENSHOT_H
