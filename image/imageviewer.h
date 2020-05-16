#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QDialog>
#include <QIcon>

#include "image.h"

QT_BEGIN_NAMESPACE
class QImage;
class QScrollArea;
class QScrollBar;
class QVBoxLayout;
class QToolBar;
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

class ScalableImage;
class LabelEventFilter;
class ZoomToolButtons;

class ImageViewer : public QDialog
{
    Q_OBJECT

public:
    ImageViewer(QDialog *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void showImage(const Image &image);

protected:
    void keyPressEvent(QKeyEvent *) override;

private slots:
    void copy();
    void zoomIn();
    void zoomOut();
    void fitToWindow();
    void actualSize();
    void lineEditReturn();
    void comboBoxChange();

private:
    void scaleImage(double factor, int zoomInOut);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    QImage image;
    QVBoxLayout *mainLayout;
    LabelEventFilter *labelEventFilter;
    QToolBar *toolBar;
    ZoomToolButtons *zoomToolButtons;
    QComboBox *scaleComboBox;
    QLineEdit *scaleEdit;
    QScrollArea *scrollArea;
    ScalableImage *scalableImage;

    double originalFactor;
    int zoomInOut;

    double scaleFactor;
    QString scaleStr;

    friend LabelEventFilter;
    friend ZoomToolButtons;
};

class ScalableImage : public QWidget
{
    Q_OBJECT

public:
    ScalableImage(QWidget *parent = nullptr) : QWidget(parent) {}

    void setPixmap(QPixmap &&pixmap) { _pixmap = std::move(pixmap); resize(_pixmap.size()); }
    const QPixmap *pixmap() const { return &_pixmap; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap _pixmap;
};

class LabelEventFilter : public QObject
{
    Q_OBJECT

public:
    LabelEventFilter(ImageViewer *parent) : QObject(parent) { this->parent = parent; }

protected:
    bool eventFilter(QObject *, QEvent *) override;

private:
    ImageViewer *parent;

    QPoint dragPosition;
};

class ZoomToolButtons : public QWidget
{
    Q_OBJECT

public:
    ZoomToolButtons(ImageViewer *parent);

    void setZoomInEnabled(bool enabled) { zoomIn.enabled = enabled; }
    void setZoomOutEnabled(bool enabled) { zoomOut.enabled = enabled; }
    void setZoomToFitEnabled(bool enabled) { zoomToFit.enabled = enabled; }
    void setZoomActualSizeEnabled(bool enabled) { zoomActualSize.enabled = enabled; }

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    QSize sizeHint() const override;

private:
    struct ToolButton
    {
        QIcon icon;
        QRect rect;
        bool pressed;
        bool enabled;
    };

    void drawButton(QPainter &painter, const ToolButton &toolButton) const;
    bool buttonPressed(const QPoint &pos, ToolButton &toolButton);

    ImageViewer *parent;

    ToolButton zoomIn;
    ToolButton zoomOut;
    ToolButton zoomToFit;
    ToolButton zoomActualSize;
};

#endif // IMAGEVIEWER_H
