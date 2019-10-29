#include <QApplication>

#include "SelectFrameWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SelectFrameWidget select_frame;
    select_frame.selectRect();
    return 0; // app.exec();
}
