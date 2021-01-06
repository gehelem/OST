#include <stdio.h>
#include "client.h"
#include "job.h"
#include <QApplication>
#include <QMetaProperty>

#include <vector>
#include <utility>
#include <algorithm>

static void dump_props(QObject *o)
{
  auto mo = o->metaObject();
  qDebug() << "## Properties of" << o << "##";
  do {
    qDebug() << "### Class" << mo->className() << "###";
    std::vector<std::pair<QString, QVariant> > v;
    v.reserve(mo->propertyCount() - mo->propertyOffset());
    for (int i = mo->propertyOffset(); i < mo->propertyCount();
          ++i)
      v.emplace_back(mo->property(i).name(),
                     mo->property(i).read(o));
    std::sort(v.begin(), v.end());
    for (auto &i : v)
      qDebug() << i.first << "=>" << i.second;
  } while ((mo = mo->superClass()));
}

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MyClient *indiclient = new MyClient(&a);

    //watchDevice(MYCCD);
    Job *MyJob1= new Job(indiclient);
    //Job *MyJob2= new Job(indiclient);
    MyJob1->addnewtask(TT_SEND_NUMBER,"test1","test1","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    MyJob1->addnewtask(TT_WAIT_BLOB  ,"test2","test1","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    //MyJob1->addnewtask(TT_ANALYSE_SOLVE,"test3","test1","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    //MyJob1->addnewtask(TT_WAIT_SOLVE   ,"test4","test1","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    MyJob1->addnewtask(TT_ANALYSE_SEP,"test5","test1","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    MyJob1->addnewtask(TT_WAIT_SEP   ,"test6","test1","CCD Simulator","CCD_EXPOSURE","CCD_EXPOSURE_VALUE",10,"",ISS_OFF);
    indiclient->setServer("localhost", 7624);
    indiclient->connectServer();
    indiclient->setBLOBMode(B_ALSO, "CCD Simulator", nullptr);
    dump_props(MyJob1);

    return a.exec();

}



