#ifndef SELECTFRAMEWIDGET_H
#define SELECTFRAMEWIDGET_H

#include <QWidget>
#include <QDialog>

class SelectFrameWidget;

class FocusDialog : public QDialog
{
    Q_OBJECT
public:
    FocusDialog(SelectFrameWidget *_parent);

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
    ~SelectFrameWidget() override;

    QRect selectRect();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    FocusDialog focus_dialog;

    void parentMousePressEvent(QMouseEvent *);
    void parentMouseMoveEvent(QMouseEvent *);
    void parentMouseReleaseEvent(QMouseEvent *);

    QRect frameRect;

    enum SelectMode
    {
        SelectRect
    };

    SelectMode mode;

    QPoint startPosition;
    QPoint endPosition;
    QPoint menuPosition;
    QRect windowRect;

    QTimer *timer;

    int frame_width;
    QPoint topLeft;
    QPoint bottomRight;

    enum DragMode
    {
        NoDrag,
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

    friend FocusDialog;
};

#endif // SELECTFRAMEWIDGET_H
