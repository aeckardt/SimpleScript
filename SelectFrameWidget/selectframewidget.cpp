#include <QtWidgets>

#include "selectframewidget.h"

FocusWidget::FocusWidget(SelectFrameWidget *_parent)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint),
    parent(_parent)
{
    setWindowState(Qt::WindowMaximized);
    setWindowOpacity(0.05);
}

void FocusWidget::mousePressEvent(QMouseEvent *event)
{
    parent->parentMousePressEvent(event);
}
void FocusWidget::mouseMoveEvent(QMouseEvent *event)
{
    parent->parentMouseMoveEvent(event);
}

void FocusWidget::mouseReleaseEvent(QMouseEvent *event)
{
    parent->parentMouseReleaseEvent(event);
}

SelectFrameWidget::SelectFrameWidget()
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
              Qt::WindowTransparentForInput | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint),
      focus_widget(this)
{
    focus_widget.show();

    setWindowState(Qt::WindowMaximized);

    menuPosition = QPoint(0, 22);
    windowRect = QRect(frameGeometry());
    windowRect.translate(menuPosition);
    frame_width = 2;
}

void SelectFrameWidget::parentMousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!ellipseRect[0].isNull() && ellipseRect[0].contains(event->globalPos() - menuPosition, false))
        {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
            startPosition = topLeft;
            endPosition = bottomRight;
        }
        else if (!ellipseRect[1].isNull() && ellipseRect[1].contains(event->globalPos() - menuPosition, false))
        {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
            startPosition = QPoint(topLeft.x(), bottomRight.y());
            endPosition = QPoint(bottomRight.x(), topLeft.y());
        }
        else if (!ellipseRect[2].isNull() && ellipseRect[2].contains(event->globalPos() - menuPosition, false))
        {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
            startPosition = QPoint(bottomRight.x(), topLeft.y());
            endPosition = QPoint(topLeft.x(), bottomRight.y());
        }
        else if (!ellipseRect[3].isNull() && ellipseRect[3].contains(event->globalPos() - menuPosition, false))
        {
            dragMode = ResizeFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
            startPosition = bottomRight;
            endPosition = topLeft;
        }
        else if (!frameRect.isNull() && frameRect.contains(event->globalPos() - menuPosition, false))
        {
            dragMode = MoveFrame;
            dragPosition = event->globalPos() - startPosition;
            QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        }
        else
        {
            dragMode = CreateFrame;
            startPosition = event->globalPos() - menuPosition;
        }
        event->accept();
    }
}

void SelectFrameWidget::parentMouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (dragMode == CreateFrame)
        {
            endPosition = event->globalPos() - menuPosition;
        }
        else if (dragMode == MoveFrame)
        {
            QPoint size = endPosition - startPosition;
            QPoint diff = QPoint(event->globalPos() - dragPosition);
            startPosition = diff;
            endPosition   = diff + size;
        }
        else if (dragMode == ResizeFrame)
        {
            startPosition = event->globalPos() - menuPosition;
            if (startPosition.y() < 0)
            {
                startPosition.setY(0);
            }
            else if (startPosition.y() > frameGeometry().height())
            {
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
    if (dragMode == MoveFrame || dragMode == ResizeFrame)
    {
        QApplication::restoreOverrideCursor();

        startPosition = topLeft;
        endPosition = bottomRight;
    }
    event->accept();
}

void SelectFrameWidget::paintEvent(QPaintEvent *)
{
    QColor mainColor(255, 0, 0);
    QColor ellipseColor(0, 0, 0, 191);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(mainColor);

    painter.save();
    painter.drawConvexPolygon(mainFramePts);
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

    QRect screen = frameGeometry();
    if (screen.width() < bottomRight.x()) {
        QPoint diff = QPoint(bottomRight.x() - screen.width(), 0);
        topLeft -= diff;
        bottomRight -= diff;
    }
    if (screen.height() < bottomRight.y()) {
        QPoint diff = QPoint(0, bottomRight.y() - screen.height());
        topLeft -= diff;
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

    ellipseRect[0] = QRect(ellipseCenter[0].x(), ellipseCenter[0].y(), 8 + frame_width, 8 + frame_width);
    ellipseRect[1] = QRect(ellipseCenter[1].x(), ellipseCenter[1].y(), 8 + frame_width, 8 + frame_width);
    ellipseRect[2] = QRect(ellipseCenter[2].x(), ellipseCenter[2].y(), 8 + frame_width, 8 + frame_width);
    ellipseRect[3] = QRect(ellipseCenter[3].x(), ellipseCenter[3].y(), 8 + frame_width, 8 + frame_width);

    ellipseFrame[0] = QRegion(ellipseRect[0], QRegion::Ellipse);
    ellipseFrame[1] = QRegion(ellipseRect[1], QRegion::Ellipse);
    ellipseFrame[2] = QRegion(ellipseRect[2], QRegion::Ellipse);
    ellipseFrame[3] = QRegion(ellipseRect[3], QRegion::Ellipse);
}

void SelectFrameWidget::resizeEvent(QResizeEvent * /* event */)
{
    recalcDim();

    QRegion maskedRegion = mainFrame + ellipseFrame[0] + ellipseFrame[1] + ellipseFrame[2] + ellipseFrame[3];
    setMask(maskedRegion);
}
