#include <QtWidgets>

#include "SelectFrameWidget.h"

FocusDialog::FocusDialog(SelectFrameWidget *_parent)
    : QDialog(_parent, Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
    parent(_parent)
{
    setWindowState(Qt::WindowMaximized);
    setWindowOpacity(0.05);
}

void FocusDialog::mousePressEvent(QMouseEvent *event)
{
    parent->parentMousePressEvent(event);
}

void FocusDialog::mouseMoveEvent(QMouseEvent *event)
{
    parent->parentMouseMoveEvent(event);
}

void FocusDialog::mouseReleaseEvent(QMouseEvent *event)
{
    parent->parentMouseReleaseEvent(event);
}

SelectFrameWidget::SelectFrameWidget()
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
              Qt::WindowTransparentForInput | Qt::NoDropShadowWindowHint),
      focus_dialog(this), dragMode(NoDrag)
{
    setWindowState(Qt::WindowMaximized);
    setAttribute(Qt::WA_DeleteOnClose);

    menuPosition = QPoint(0, 23);
    windowRect = QRect(frameGeometry());
    windowRect.translate(menuPosition);
    frame_width = 1;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(5);
}

SelectFrameWidget::~SelectFrameWidget()
{
    delete timer;
}

QRect SelectFrameWidget::selectRect()
{
    mode = SelectRect;

    show();
    focus_dialog.exec();

    return QRect(
        frameRect.topLeft()     + menuPosition,
        frameRect.bottomRight() + menuPosition - QPoint(1, 1));
}

void SelectFrameWidget::parentMousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!ellipseRect[0].isNull() && ellipseRect[0].contains(event->globalPos() - menuPosition, false)) {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
            startPosition = topLeft;
            endPosition = bottomRight;
        } else if (!ellipseRect[1].isNull() && ellipseRect[1].contains(event->globalPos() - menuPosition, false)) {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
            startPosition = QPoint(topLeft.x(), bottomRight.y());
            endPosition = QPoint(bottomRight.x(), topLeft.y());
        } else if (!ellipseRect[2].isNull() && ellipseRect[2].contains(event->globalPos() - menuPosition, false)) {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
            startPosition = QPoint(bottomRight.x(), topLeft.y());
            endPosition = QPoint(topLeft.x(), bottomRight.y());
        } else if (!ellipseRect[3].isNull() && ellipseRect[3].contains(event->globalPos() - menuPosition, false)) {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
            startPosition = bottomRight;
            endPosition = topLeft;
        } else if (!frameRect.isNull() && frameRect.contains(event->globalPos() - menuPosition, false)) {
            dragMode = MoveFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        } else {
            dragMode = CreateFrame;
            startPosition = event->globalPos() - menuPosition;
        }
        event->accept();
    }
}

void SelectFrameWidget::parentMouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (dragMode == CreateFrame) {
            endPosition = event->globalPos() - menuPosition;
            if (endPosition.y() < 0) {
                endPosition.setY(0);
            } else if (endPosition.y() > frameGeometry().height()) {
                endPosition.setY(frameGeometry().height());
            }
        } else if (dragMode == MoveFrame) {
            QPoint size = endPosition - startPosition;
            QPoint diff = QPoint(event->globalPos() - dragPosition);
            startPosition = diff;
            endPosition   = diff + size;
        } else if (dragMode == ResizeFrame) {
            startPosition = event->globalPos() - menuPosition;
            if (startPosition.y() < 0) {
                startPosition.setY(0);
            } else if (startPosition.y() > frameGeometry().height()) {
                startPosition.setY(frameGeometry().height());
            }
        }
        event->accept();
        resizeEvent(nullptr);
        update();
    }
}

void SelectFrameWidget::parentMouseReleaseEvent(QMouseEvent *event)
{
    if (dragMode == MoveFrame || dragMode == ResizeFrame) {
        QApplication::restoreOverrideCursor();

        startPosition = topLeft;
        endPosition = bottomRight;
    }
    dragMode = NoDrag;
    event->accept();

    if (mode == SelectRect) {
        focus_dialog.close();
        close();
    }
}

void SelectFrameWidget::paintEvent(QPaintEvent *)
{
    QColor ellipseColor(63, 63, 63);

    QTime time = QTime::currentTime();
    int xinc = ((time.second() * 1000 + time.msec()) / 100) % 6;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int xpos, ypos;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));

    painter.save();
    for (xpos = topLeft.x() - xinc; xpos < bottomRight.x(); xpos += 6) {
        painter.drawRect(xpos, topLeft.y(), 3, frame_width);
    }
    for (xpos = topLeft.x() + xinc; xpos < bottomRight.x(); xpos += 6) {
        painter.drawRect(xpos, bottomRight.y() - frame_width, 3, frame_width);
    }
    for (ypos = topLeft.y() + xinc; ypos < bottomRight.y(); ypos += 6) {
        painter.drawRect(topLeft.x(), ypos, frame_width, 3);
    }
    for (ypos = topLeft.y() - xinc; ypos < bottomRight.y(); ypos += 6) {
        painter.drawRect(bottomRight.x() - frame_width, ypos, frame_width, 3);
    }
    painter.restore();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(63, 63, 63));

    painter.save();
    for (xpos = topLeft.x() - xinc; xpos + xinc < bottomRight.x(); xpos += 6) {
        painter.drawRect(xpos + 2, topLeft.y(), 3, frame_width);
    }
    for (xpos = topLeft.x() + xinc; xpos + xinc < bottomRight.x(); xpos += 6) {
        painter.drawRect(xpos + 2, bottomRight.y() - frame_width, 3, frame_width);
    }
    for (ypos = topLeft.y() + xinc; ypos < bottomRight.y(); ypos += 6) {
        painter.drawRect(topLeft.x(), ypos + 2, frame_width, 3);
    }
    for (ypos = topLeft.y() - xinc; ypos < bottomRight.y(); ypos += 6) {
        painter.drawRect(bottomRight.x() - frame_width, ypos + 2, frame_width, 3);
    }
    painter.restore();

    painter.setPen(Qt::NoPen);
    painter.setBrush(ellipseColor);

    painter.save();
    painter.drawEllipse(ellipseRect[0]);
    painter.drawEllipse(ellipseRect[1]);
    painter.drawEllipse(ellipseRect[2]);
    painter.drawEllipse(ellipseRect[3]);
    painter.restore();
}

void SelectFrameWidget::recalcDim()
{
    QPoint size = QPoint(
                std::abs(startPosition.x() - endPosition.x()),
                std::abs(startPosition.y() - endPosition.y()));

    topLeft = QPoint(
                std::max(0, std::min(startPosition.x(), endPosition.x())),
                std::max(0, std::min(startPosition.y(), endPosition.y())));

    bottomRight = topLeft + size;

    QRect frame = frameGeometry();
    if (frame.width() < bottomRight.x()) {
        QPoint diff = QPoint(bottomRight.x() - frame.width(), 0);
        topLeft = QPoint(
                    std::max(0, topLeft.x() - diff.x()),
                    std::max(0, topLeft.y() - diff.y()));
        bottomRight -= diff;
    }
    if (frame.height() < bottomRight.y()) {
        QPoint diff = QPoint(0, bottomRight.y() - frame.height());
        topLeft = QPoint(
                    std::max(0, topLeft.x() - diff.x()),
                    std::max(0, topLeft.y() - diff.y()));
        bottomRight -= diff;
    }

    frameRect = QRect(topLeft, bottomRight);

    mainFramePts = {
        QPoint(topLeft.x(), topLeft.y()),
        QPoint(topLeft.x(), bottomRight.y()),
        QPoint(topLeft.x() + frame_width, bottomRight.y()),
        QPoint(topLeft.x() + frame_width, topLeft.y() + frame_width),
        QPoint(bottomRight.x() - frame_width, topLeft.y() + frame_width),
        QPoint(bottomRight.x() - frame_width, bottomRight.y() - frame_width),
        QPoint(topLeft.x() + frame_width, bottomRight.y() - frame_width),
        QPoint(topLeft.x() + frame_width, bottomRight.y()),
        QPoint(bottomRight.x(), bottomRight.y()),
        QPoint(bottomRight.x(), topLeft.y())
    };

    mainFrame = QRegion(QPolygon(mainFramePts), Qt::OddEvenFill);

    ellipseCenter[0] = QPoint(topLeft.x() - 4, topLeft.y() - 4);
    ellipseCenter[1] = QPoint(topLeft.x() - 4, bottomRight.y() - 4 - frame_width);
    ellipseCenter[2] = QPoint(bottomRight.x() - 4 - frame_width, topLeft.y() - 4);
    ellipseCenter[3] = QPoint(bottomRight.x() - 4 - frame_width, bottomRight.y() - 4 - frame_width);

    ellipseRect[0] = QRect(ellipseCenter[0].x(), ellipseCenter[0].y(), 10, 10);
    ellipseRect[1] = QRect(ellipseCenter[1].x(), ellipseCenter[1].y(), 10, 10);
    ellipseRect[2] = QRect(ellipseCenter[2].x(), ellipseCenter[2].y(), 10, 10);
    ellipseRect[3] = QRect(ellipseCenter[3].x(), ellipseCenter[3].y(), 10, 10);

    ellipseFrame[0] = QRegion(ellipseRect[0], QRegion::Ellipse);
    ellipseFrame[1] = QRegion(ellipseRect[1], QRegion::Ellipse);
    ellipseFrame[2] = QRegion(ellipseRect[2], QRegion::Ellipse);
    ellipseFrame[3] = QRegion(ellipseRect[3], QRegion::Ellipse);
}

void SelectFrameWidget::resizeEvent(QResizeEvent *)
{
    recalcDim();

    QRegion maskedRegion = mainFrame + ellipseFrame[0] + ellipseFrame[1] + ellipseFrame[2] + ellipseFrame[3];
    setMask(maskedRegion);
}
