#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QGraphicsPixmapItem>

#define FILE_LOCATION "/Users/albrecht/Documents/Images/Buddhabude.jpg"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QImage image(FILE_LOCATION);

    QGraphicsScene scene(image.rect());
    QGraphicsView view(&scene);

    QGraphicsPixmapItem *pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene.addItem(pixmap);

    view.show();

    return app.exec();
}
