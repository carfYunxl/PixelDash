#pragma once

#include <QDialog>
#include "ui_InfoDialog.h"
#include "HFST_CommonHeader.hpp"

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    InfoDialog(QWidget *parent = nullptr);
    ~InfoDialog();

public:
    void SetChipID(int chipid);
    void SetFirmwareVersion(const QString& strVersion);
    void SetVersion(int version);
    void SetKeyCount(int nKey);
    void SetProtocol(const QString& strVersion);
    void SetRxTxCount(int rx, int tx);
    void SetResolution(int resX, int resY);

    void SetI2CAddr(const std::vector<HFST::I2CAddr>& addrs);

    void SetResXY(int resX, int resY);
    void SetFirmwareRevision(const std::string& strRec);

private:
    Ui::InfoDialogClass ui;
};
