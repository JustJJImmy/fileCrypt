#include "FileEncrypt.h"
#include <QtWidgets/QApplication>

#include <QtCore/QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileEncrypt w;
    w.show();
    return a.exec();
}
