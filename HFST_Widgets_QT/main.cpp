#include "QCMNR.h"
#include <QtWidgets/QApplication>
#include "HFST_Exception.hpp"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    std::unique_ptr<QCMNR> pQCMNR;
    try {
        pQCMNR = std::make_unique<QCMNR>();
        pQCMNR->show();
    }
    catch (const HFST::HFST_Exception& exception)
    {
        qDebug() << "Á¬½ÓÊ§°Ü";
    }
    return a.exec();
}
