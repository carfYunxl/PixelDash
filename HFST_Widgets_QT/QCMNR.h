#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QCMNR.h"
#include <memory>

namespace HFST
{
    class Connector;
}

class InfoDialog;
class QAction;
class QDockWidget;
class QListWidget;
class QTextEdit;

class QCMNR : public QMainWindow
{
    Q_OBJECT

public:
    QCMNR(QWidget *parent = nullptr);
    ~QCMNR();

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;

private slots:
    void ShowICInfo();

private:
    void InitUI_ICInfomation();
    void InitUI_ToolBar();
    void InitUI_StatusBar();
    void InitUI_CenterWidget();
    void InitUI_DockWidget();
private:
    Ui::QCMNRClass ui;

    std::unique_ptr<HFST::Connector>    m_Connector;
    std::unique_ptr<InfoDialog>         m_ICInfoDlg;
    QAction*                            m_ActionConnect;
};
