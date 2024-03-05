#include "QCMNR.h"
#include <QtWidgets/QApplication>
#include "HFST_Exception.hpp"
#include <QDebug>
#include <thread>
#include "HFST_RawCollector.hpp"
#include "HFST_Connector.hpp"
#include "HFST_Exception.hpp"
#include "HFST_APICenter.hpp"

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

    std::thread thread([&]() {
        QString strText;
        HFST::IC_Info info = pQCMNR->Get()->IC_GetInfo();
        HFST::RawCollector collector(info);
        HFST::RAW::ChannelRaw<short> raw;
        while (1)
        {
            strText.clear();
            raw.vecRaw.clear();
            int ret = collector.ReadChannelRaw(raw);
            if (ret == 1)
            {
                for (size_t i = 0; i < raw.vecRaw.size(); ++i)
                {
                    if(raw.nDataType == 0x20)
                        strText.append(QString::number(raw.vecRaw.at(i), 16));
                    else
                        strText.append(QString::number(raw.vecRaw.at(i), 10));
                    strText.append(" ");
                }

                pQCMNR->SetUI(strText);

                Sleep(10);
            }
        }
        });

    thread.detach();

    return a.exec();
}
