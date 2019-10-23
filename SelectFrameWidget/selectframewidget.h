#ifndef SELECTFRAMEWIDGET_H
#define SELECTFRAMEWIDGET_H

#include <QWidget>

class SelectFrameWidget;

class FocusWidget : public QWidget
{
    Q_OBJECT
public:
    FocusWidget(SelectFrameWidget *_parent);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    SelectFrameWidget *parent;
};

class SelectFrameWidget : public QWidget
{
    Q_OBJECT
public:
    SelectFrameWidget();

    void parentMousePressEvent(QMouseEvent *);
    void parentMouseMoveEvent(QMouseEvent *);
    void parentMouseReleaseEvent(QMouseEvent *);

    QRect frameRect;

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    FocusWidget focus_widget;
    QPoint startPosition;
    QPoint endPosition;
    QPoint menuPosition;
    QRect windowRect;

    int frame_width;
    QPoint topLeft;
    QPoint bottomRight;

    enum DragMode
    {
        CreateFrame,
        MoveFrame,
        ResizeFrame
    };

    QPoint dragPosition;
    DragMode dragMode;

    QVector<QPoint> mainFramePts;
    QPoint ellipseCenter[4];
    QRect ellipseRect[4];

    QRegion mainFrame;
    QRegion ellipseFrame[4];

    void recalcDim();
};
// QWindow interface

#endif // SELECTFRAMEWIDGET_H
