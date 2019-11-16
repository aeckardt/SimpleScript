#include <QtWidgets>

#include "ImageView.h"

ImageView::ImageView()
   : scalableImage(new ScalableImage)
   , scrollArea(new QScrollArea)
   , toolBar(new QToolBar)
   , mainLayout(new QVBoxLayout(this))
   , zoomToolButtons(new ZoomToolButtons(this))
   , scaleEdit(new QLineEdit)
   , scaleComboBox(new QComboBox)
   , labelEventFilter(new LabelEventFilter(this))
   , originalFactor(1.0)
   , zoomInOut(0)
   , scaleFactor(1.0)
{
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->horizontalScrollBar()->setSingleStep(15);
    scrollArea->verticalScrollBar()->setSingleStep(15);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(toolBar);
    mainLayout->setAlignment(toolBar, Qt::AlignTop);
    mainLayout->addWidget(scrollArea);

    scalableImage->installEventFilter(labelEventFilter);

    scaleComboBox->addItems(QStringList({"12.5%", "25%", "50%", "100%", "200%", "400%", "800%"}));
    scaleComboBox->setCurrentIndex(3);
    scaleComboBox->setLineEdit(scaleEdit);
    scaleComboBox->setInsertPolicy(QComboBox::NoInsert);

    connect(scaleComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(comboBoxChange()));
    connect(scaleEdit, SIGNAL(returnPressed()), this, SLOT(lineEditReturn()));

    toolBar->addWidget(zoomToolButtons);
    toolBar->addWidget(scaleComboBox);

    setWindowTitle("Image Viewer");
}

void ImageView::showImage(const QImage &image)
{
    this->image = &image;

    scalableImage->setPixmap(QPixmap::fromImage(image));

    scrollArea->setWidget(scalableImage);

    const QSize &availableSize = QGuiApplication::primaryScreen()->availableSize();
    const QSize &unusuableSize = QSize(2, 55 + 4);
    const QSize &maxImageSize  = availableSize - unusuableSize;
    const QSize &imageSize     = image.size();

    if (imageSize.width() <= maxImageSize.width() && imageSize.height() <= maxImageSize.height()) {
        resize(imageSize + unusuableSize);
    } else if (imageSize.width() <= maxImageSize.width()) {
        // Vertical scrollbar needed (less space for image)
        resize(imageSize.width() + unusuableSize.width(), availableSize.height());
    } else if (imageSize.height() <= maxImageSize.height()) {
        // Horizontal scrollbar needed (less space for image)
        resize(availableSize.width(), imageSize.height() + unusuableSize.height());
    } else {
        resize(availableSize.width(), availableSize.height());
    }

    show();

    exec();
}

void ImageView::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_W)
            close(); // Close when Ctrl+W is pressed
        else if (event->key() == Qt::Key_C)
            copy();
        else if (event->key() == Qt::Key_Plus  && scaleFactor < 8.0)
            zoomIn();
        else if (event->key() == Qt::Key_Minus && scaleFactor > 0.126)
            zoomOut();
        else if (event->key() == Qt::Key_F)
            fitToWindow();
        else if (event->key() == Qt::Key_A)
            actualSize();
    }
}

void ImageView::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(*image);
#endif // !QT_NO_CLIPBOARD
}

void ImageView::zoomIn()
{
    scaleImage(originalFactor, ++zoomInOut);
}

void ImageView::zoomOut()
{
    scaleImage(originalFactor, --zoomInOut);
}

void ImageView::fitToWindow()
{
    QSize totalArea = size() - QSize(2, toolBar->size().height() + 2);
    originalFactor = std::min(
                static_cast<double>(totalArea.width())  / static_cast<double>(scalableImage->pixmap()->size().width()),
                static_cast<double>(totalArea.height()) / static_cast<double>(scalableImage->pixmap()->size().height()));
    scaleImage(originalFactor, 0);
}

void ImageView::actualSize()
{
    scaleImage(1.0, 0);
}

void ImageView::comboBoxChange()
{
    if (scaleStr == scaleEdit->text())
        return;

    if (scaleComboBox->currentIndex() != -1)
        scaleImage(1.0 * pow(2, scaleComboBox->currentIndex() - 3), 0);
}

void ImageView::lineEditReturn()
{
    double newScaleFactor;
    sscanf(scaleEdit->text().toUtf8(), "%lf%%", &newScaleFactor);
    newScaleFactor /= 100.0;
    if (newScaleFactor >= 0.01 && newScaleFactor <= 100.0)
        scaleImage(newScaleFactor, 0);
}

void ImageView::scaleImage(double factor, int zoomInOut)
{
    Q_ASSERT(scalableImage->pixmap());

    originalFactor = factor;
    this->zoomInOut = zoomInOut;

    // 1.259922 is ~ sqrt3(2)
    // -> it takes three times zooming in to half the zoom factor
    double newScaleFactor = factor * pow(1.259922, zoomInOut);

    if (fabs(scaleFactor - newScaleFactor) < 0.001)
        return;

    double factorQuot = newScaleFactor / scaleFactor;
    scaleFactor = newScaleFactor;

    scalableImage->resize(scaleFactor * scalableImage->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factorQuot);
    adjustScrollBar(scrollArea->verticalScrollBar(), factorQuot);

    zoomToolButtons->setZoomInEnabled(scaleFactor < 8.0);
    zoomToolButtons->setZoomOutEnabled(scaleFactor > 0.126);

    if (fabs(scaleFactor * 100.0 - static_cast<double>(static_cast<int>(scaleFactor * 100.0 + 0.5))) < 0.0999)
        scaleStr.sprintf("%d%%", static_cast<int>(scaleFactor * 100.0 + 0.1));
    else
        scaleStr.sprintf("%.1f%%", scaleFactor * 100.0);
    scaleEdit->setText(scaleStr);

    if (scaleStr == "12.5%")
        scaleComboBox->setCurrentIndex(0);
    else if (scaleStr == "25%")
        scaleComboBox->setCurrentIndex(1);
    else if (scaleStr == "50%")
        scaleComboBox->setCurrentIndex(2);
    else if (scaleStr == "100%")
        scaleComboBox->setCurrentIndex(3);
    else if (scaleStr == "200%")
        scaleComboBox->setCurrentIndex(4);
    else if (scaleStr == "400%")
        scaleComboBox->setCurrentIndex(5);
    else if (scaleStr == "800%")
        scaleComboBox->setCurrentIndex(6);
    else {
        scaleComboBox->setCurrentIndex(-1);
        scaleComboBox->setCurrentText(scaleStr);
    }
}

void ImageView::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(static_cast<int>(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ScalableImage::paintEvent(QPaintEvent *event)
{
    double scaleFactor = static_cast<double>(_pixmap.size().width()) / static_cast<double>(size().width());

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF pixmapRect;
    pixmapRect.setTopLeft(scaleFactor * QPointF(event->rect().topLeft()));
    pixmapRect.setSize(scaleFactor * QSizeF(event->rect().size()));
    painter.drawPixmap(QRectF(event->rect()), _pixmap, pixmapRect);
}

ZoomToolButtons::ZoomToolButtons(ImageView *parent)
    : QWidget(parent)
    , parent(parent)
{
    zoomIn = {
        QIcon(":/Zoom_In.png"),
        QRect(50, 10, 40, 22), false, true};

    zoomOut = {
        QIcon(":/Zoom_Out.png"),
        QRect(10, 10, 40, 22), false, true};

    zoomToFit = {
        QIcon(":/Zoom_To_Fit.png"),
        QRect(95, 10, 40, 22), false, true};

    zoomActualSize = {
        QIcon(":/Zoom_Actual_Size.png"),
        QRect(135, 10, 40, 22), false, true};
}

bool ZoomToolButtons::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if (zoomIn.rect.contains(helpEvent->pos()))
            QToolTip::showText(helpEvent->globalPos(), "Zoom In");
        else if (zoomOut.rect.contains(helpEvent->pos()))
            QToolTip::showText(helpEvent->globalPos(), "Zoom Out");
        else if (zoomToFit.rect.contains(helpEvent->pos()))
            QToolTip::showText(helpEvent->globalPos(), "Fit to Window");
        else if (zoomActualSize.rect.contains(helpEvent->pos()))
            QToolTip::showText(helpEvent->globalPos(), "Actual Size");
        else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }

    return QWidget::event(event);
}

void ZoomToolButtons::drawButton(QPainter &painter, const ToolButton &toolButton) const
{
    QPainterPath path;
    path.addRoundedRect(QRectF(toolButton.rect), 4, 4);
    QPen pen(QColor(195, 194, 194), 1);
    painter.setPen(pen);
    painter.fillPath(path, toolButton.pressed ? QColor(227, 227, 227) : QColor(246, 246, 246));
    painter.drawPath(path);
    painter.drawPixmap(toolButton.rect.topLeft() + QPoint(12, 3), toolButton.icon.pixmap(QSize(16, 16)));
}

void ZoomToolButtons::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPen textPen(Qt::black, 1);
    painter.setPen(textPen);

    QFont font = painter.font() ;
    font.setPointSize(11);
    painter.setFont(font);
    painter.drawText(QRect(78, 36, 40, 22), Qt::AlignLeft, "Zoom");

    drawButton(painter, zoomIn);
    drawButton(painter, zoomOut);
    drawButton(painter, zoomToFit);
    drawButton(painter, zoomActualSize);
}

bool ZoomToolButtons::buttonPressed(const QPoint &pos, ToolButton &toolButton)
{
    if (toolButton.enabled && toolButton.rect.contains(pos)) {
        toolButton.pressed = true;
        update();
        return true;
    }

    return false;
}

void ZoomToolButtons::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint relPos = mapFromGlobal(event->globalPos());
        if (buttonPressed(relPos, zoomOut) ||
            buttonPressed(relPos, zoomIn)  ||
            buttonPressed(relPos, zoomToFit) ||
            buttonPressed(relPos, zoomActualSize))

            event->accept();
    }
}

void ZoomToolButtons::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint relPos = mapFromGlobal(event->globalPos());
    if (zoomOut.pressed && zoomOut.enabled && zoomOut.rect.contains(relPos))
        parent->zoomOut();
    else if (zoomIn.pressed && zoomIn.enabled && zoomIn.rect.contains(relPos))
        parent->zoomIn();
    else if (zoomToFit.pressed && zoomToFit.enabled && zoomToFit.rect.contains(relPos))
        parent->fitToWindow();
    else if (zoomActualSize.pressed && zoomActualSize.enabled && zoomActualSize.rect.contains(relPos))
        parent->actualSize();
    if (zoomOut.pressed || zoomIn.pressed || zoomToFit.pressed || zoomActualSize.pressed) {
        zoomOut.pressed = false;
        zoomIn.pressed = false;
        zoomToFit.pressed = false;
        zoomActualSize.pressed = false;
        update();
        event->accept();
    }
}

QSize ZoomToolButtons::sizeHint() const
{
    return QSize(195, 55);
}

bool LabelEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonRelease) {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (event->type() == QEvent::MouseButtonPress) {
            if (mouseEvent->button() == Qt::LeftButton) {
                dragPosition = mouseEvent->globalPos() - parent->frameGeometry().topLeft();
                QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
                event->accept();
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                QPoint movePt = mouseEvent->globalPos() - parent->frameGeometry().topLeft() - dragPosition;
                parent->scrollArea->horizontalScrollBar()->setValue(
                    parent->scrollArea->horizontalScrollBar()->value() - movePt.x());
                parent->scrollArea->verticalScrollBar()->setValue(
                    parent->scrollArea->verticalScrollBar()->value() - movePt.y());
                event->accept();
                dragPosition = mouseEvent->globalPos() - parent->frameGeometry().topLeft();
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QApplication::restoreOverrideCursor();
            event->accept();
        }
        return true;
    } else
        return QObject::eventFilter(watched, event);
}
