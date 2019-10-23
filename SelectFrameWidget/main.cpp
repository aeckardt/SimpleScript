#include <QApplication>

#include "selectframewidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SelectFrameWidget select_frame;
    select_frame.show();
    return app.exec();
}
