#include <stdio.h>
#include "OSTController.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    OSTController *MyOSTController = new OSTController();
    //QQuickItem* childItem = qobject_cast<QQuickItem*>(component.create());
    //OSTController* MyOSTController = qobject_cast<OSTController*>(&app);

    app.exec();
    
    delete MyOSTController;

    return 0;
}
