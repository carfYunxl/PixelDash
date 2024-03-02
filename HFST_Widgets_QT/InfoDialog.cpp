#include "InfoDialog.h"

InfoDialog::InfoDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    setWindowTitle("IC Info");
    setWindowIcon(QIcon("icon/chip.png"));
}

InfoDialog::~InfoDialog()
{}

void InfoDialog::SetChipID(int chipid)
{
    ui.lineEdit_ChipID->setText(QString::number(chipid));
}
void InfoDialog::SetFirmwareVersion(const QString& strVersion)
{
    ui.lineEdit_Firmware_Version->setText(strVersion);
}
void InfoDialog::SetVersion(int version)
{
    ui.lineEdit_Version->setText(QString::number(version,16));
}
void InfoDialog::SetKeyCount(int nKey)
{
    ui.lineEdit_KeyCount->setText(QString::number(nKey));
}
void InfoDialog::SetProtocol(const QString& strVersion)
{
    ui.lineEdit_Protocol_Version->setText(strVersion);
}
void InfoDialog::SetRxTxCount(int rx, int tx)
{
    ui.lineEdit_NumX->setText(QString::number(rx));
    ui.lineEdit_NumY->setText(QString::number(tx));
}
void InfoDialog::SetResolution(int resX, int resY)
{
    ui.lineEdit_ResX->setText(QString::number(resX));
    ui.lineEdit_ResY->setText(QString::number(resY));
}

void InfoDialog::SetI2CAddr(const std::vector<HFST::I2CAddr>& addrs)
{
    if (addrs.empty())
        return;

    for ( size_t i = 0; i < addrs.size(); ++i )
    {
        auto add = addrs.at(i);
        ui.cbsAddr->addItem(QString::number(add.nAddr,16));

        if (!add.bICP)
            ui.cbsAddr->setCurrentIndex(i);
    }
}

void InfoDialog::SetResXY(int resX, int resY)
{
    ui.lineEdit_ResX->setText(QString::number(resX));
    ui.lineEdit_ResY->setText(QString::number(resY));
}

void InfoDialog::SetFirmwareRevision(const std::string& strRec)
{
    ui.lineEdit_Version->setText(strRec.c_str());
}
