#include <QApplication>
#include "dynamicwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DynamicWidget window;
    window.show();
    return app.exec();
}
