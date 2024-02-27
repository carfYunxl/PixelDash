#include "QCMNR.h"
#include "HFST_Connector.hpp"
#include "HFST_Exception.hpp"
#include "InfoDialog.h"
#include <QToolButton>
#include <Dbt.h>
#include <windows.h>
#include <QMessageBox>
#include <QAction>
#include <string>
#include <QDebug>
#include <QDockWidget>
#include <QListWidget>
#include <QSplitter>

QCMNR::QCMNR(QWidget *parent)
    : QMainWindow(parent)
    , m_Connector(std::make_unique<HFST::Connector>())
    , m_ICInfoDlg(std::make_unique<InfoDialog>(this))
    , m_ActionConnect(nullptr)
{
    ui.setupUi(this);
    resize(1920, 1080);

    /*if ( !m_Connector->Connect() )
    {
        throw HFST::HFST_Exception("Connected Failed!");
    }
    ui.textEdit_Log->append("Connect Success!");

    if ( !RegisterUSBDevice() )
    {
        throw HFST::HFST_Exception("Register USB Device Failed!");
    }*/
    ui.textEdit_Log->append("Register USB Device Success!");

    InitUI_ICInfomation();

    InitUI_CenterWidget();
    InitUI_DockWidget();
    InitUI_StatusBar();
    InitUI_ToolBar();
}

QCMNR::~QCMNR()
{}

void QCMNR::ShowICInfo()
{
    m_ICInfoDlg->show();
}

bool QCMNR::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

    MSG* msg = reinterpret_cast<MSG*>(message);
    if ( msg->message == WM_DEVICECHANGE )
    {
        if (msg->wParam == DBT_DEVICEREMOVECOMPLETE)
        {
            m_ActionConnect->setIcon(QIcon("icon/disconnect.png"));
            m_ActionConnect->setText(QString::fromStdWString(L"Disconnect"));
        }
        else if ( msg->wParam == DBT_DEVICEARRIVAL )
        {
            m_ActionConnect->setIcon(QIcon("icon/connect.png"));
            m_ActionConnect->setText(QString::fromStdWString(L"Connect"));
        }
    }
    return false;
}

bool QCMNR::RegisterUSBDevice()
{
    HDEVNOTIFY hDeviceNotify;

    GUID tmpGuid{
        0x8D98FC49,
        0x7A37,
        0x4B2D,
        {0xA1, 0xA3, 0x55, 0xEA, 0x7B, 0xB2, 0xAE, 0x60}
    };

    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    HWND hWnd = (HWND)(this->winId()); //获取当前窗口句柄
    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = tmpGuid;
    hDeviceNotify = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

    return (hDeviceNotify != NULL);
}

void QCMNR::InitUI_ICInfomation()
{
    m_ICInfoDlg->SetChipID(m_Connector->GetChipID());

    auto [NumX, NumY] = m_Connector->GetRxTxCnt();
    m_ICInfoDlg->SetRxTxCount(NumX, NumY);
    m_ICInfoDlg->SetKeyCount(m_Connector->GetNumKey());

    auto addr = m_Connector->GetI2CAddr();
    m_ICInfoDlg->SetI2CAddr(addr);

    m_ICInfoDlg->SetProtocol(m_Connector->Protocol().c_str());

    auto [resX, resY] = m_Connector->GetResXY();
    m_ICInfoDlg->SetResXY(resX, resY);

    m_ICInfoDlg->SetFirmwareRevision(m_Connector->GetFwRevision());
    m_ICInfoDlg->SetFirmwareVersion(QString::number(m_Connector->GetFwVersion()));
}

void QCMNR::InitUI_ToolBar()
{
    ui.mainToolBar->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    ui.mainToolBar->setAllowedAreas(Qt::AllToolBarAreas);
    ui.mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui.mainToolBar->setWindowTitle("System Infomation");
    int status = m_Connector->GetStatus();
    if (status == 0)
    {
        m_ActionConnect = ui.mainToolBar->addAction(QIcon("icon/connect.png"), QString::fromStdWString(L"Connect"), std::bind(&QCMNR::ShowICInfo, this));
    }
    else
    {
        m_ActionConnect = ui.mainToolBar->addAction(QIcon("icon/disconnect.png"), QString::fromStdWString(L"Disconnect"), std::bind(&QCMNR::ShowICInfo, this));
    }
    ui.mainToolBar->addAction(QIcon("icon/chip.png"), QString::fromStdWString(L"Device Info"), std::bind(&QCMNR::ShowICInfo, this));
}

void QCMNR::InitUI_StatusBar()
{
    statusBar()->showMessage("  Initialized......");
}

void QCMNR::InitUI_CenterWidget()
{
    /*auto widget = takeCentralWidget();
    if (widget)
        delete widget;*/
}

void QCMNR::InitUI_DockWidget()
{
    
}
