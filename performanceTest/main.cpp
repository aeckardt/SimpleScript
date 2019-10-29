#include "../image/screenshot.h"
#include "../utils/measuretime.h"

#define PPTOOL_PATH "/Users/albrecht/Documents/Code/GameScript/"

int main(int, char **)
{
    QImage img;

    InitMeasurement()

    Iterate(captureDesktop(img), 150, "Take screenshot")
    Iterate(img.save(PPTOOL_PATH "output/screen.png", "PNG"), 1, "Save image")

    return 0;
}
