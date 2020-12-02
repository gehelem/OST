#include <stdio.h>
#include "OSTController.h"
#include <QApplication>
#include "echoserver.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    OSTController *MyOSTController = new OSTController(&a);
    //EchoServer *server = new EchoServer(1234, true);
    //QObject::connect(server, &EchoServer::closed, &a, &QCoreApplication::quit);
    return a.exec();

}



