#include <QApplication>
#include <QImage>

#include "ImageView.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    QImage pngImg("/Users/albrecht/Documents/Code/pptool/imageView/resources/Zoom_Actual_Size.png");
//    pngImg.save("/Users/albrecht/Documents/Code/pptool/imageView/resources/Zoom_Actual_Size.png");
    ImageView imageView;
    imageView.showImage(QImage("/Users/albrecht/Documents/Code/pptool/output/screen.png"));
    return 0;
}
