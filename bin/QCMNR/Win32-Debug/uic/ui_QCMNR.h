/********************************************************************************
** Form generated from reading UI file 'QCMNR.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QCMNR_H
#define UI_QCMNR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QCMNRClass
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QWidget *widget_Raw;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuIC;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockLog;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout;
    QTextEdit *textEdit_Log;
    QDockWidget *dockWidget_Raw;
    QWidget *dockWidgetContents_2;
    QDockWidget *dockWidget_Canvas;
    QWidget *dockWidgetContents_3;
    QDockWidget *dockWidget_CFG;
    QWidget *dockWidgetContents_4;
    QDockWidget *dockWidget_Flash;
    QWidget *dockWidgetContents_5;

    void setupUi(QMainWindow *QCMNRClass)
    {
        if (QCMNRClass->objectName().isEmpty())
            QCMNRClass->setObjectName(QString::fromUtf8("QCMNRClass"));
        QCMNRClass->resize(1385, 1125);
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font.setPointSize(10);
        QCMNRClass->setFont(font);
        QCMNRClass->setStyleSheet(QString::fromUtf8(""));
        centralWidget = new QWidget(QCMNRClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setStyleSheet(QString::fromUtf8(""));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        widget_Raw = new QWidget(centralWidget);
        widget_Raw->setObjectName(QString::fromUtf8("widget_Raw"));
        widget_Raw->setStyleSheet(QString::fromUtf8("background-color: rgb(142, 187, 255);"));

        gridLayout_2->addWidget(widget_Raw, 0, 0, 1, 1);

        QCMNRClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QCMNRClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1385, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuIC = new QMenu(menuBar);
        menuIC->setObjectName(QString::fromUtf8("menuIC"));
        QCMNRClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QCMNRClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        QCMNRClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QCMNRClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        QCMNRClass->setStatusBar(statusBar);
        dockLog = new QDockWidget(QCMNRClass);
        dockLog->setObjectName(QString::fromUtf8("dockLog"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font1.setPointSize(10);
        font1.setStyleStrategy(QFont::PreferAntialias);
        dockLog->setFont(font1);
        dockLog->setStyleSheet(QString::fromUtf8(""));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        gridLayout = new QGridLayout(dockWidgetContents);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textEdit_Log = new QTextEdit(dockWidgetContents);
        textEdit_Log->setObjectName(QString::fromUtf8("textEdit_Log"));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Consolas"));
        font2.setPointSize(10);
        font2.setStyleStrategy(QFont::PreferAntialias);
        textEdit_Log->setFont(font2);
        textEdit_Log->setReadOnly(true);

        gridLayout->addWidget(textEdit_Log, 0, 0, 1, 1);

        dockLog->setWidget(dockWidgetContents);
        QCMNRClass->addDockWidget(Qt::BottomDockWidgetArea, dockLog);
        dockWidget_Raw = new QDockWidget(QCMNRClass);
        dockWidget_Raw->setObjectName(QString::fromUtf8("dockWidget_Raw"));
        dockWidget_Raw->setFont(font);
        dockWidget_Raw->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QString::fromUtf8("dockWidgetContents_2"));
        dockWidget_Raw->setWidget(dockWidgetContents_2);
        QCMNRClass->addDockWidget(Qt::LeftDockWidgetArea, dockWidget_Raw);
        dockWidget_Canvas = new QDockWidget(QCMNRClass);
        dockWidget_Canvas->setObjectName(QString::fromUtf8("dockWidget_Canvas"));
        dockWidget_Canvas->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QString::fromUtf8("dockWidgetContents_3"));
        dockWidget_Canvas->setWidget(dockWidgetContents_3);
        QCMNRClass->addDockWidget(Qt::LeftDockWidgetArea, dockWidget_Canvas);
        dockWidget_CFG = new QDockWidget(QCMNRClass);
        dockWidget_CFG->setObjectName(QString::fromUtf8("dockWidget_CFG"));
        dockWidget_CFG->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        dockWidgetContents_4 = new QWidget();
        dockWidgetContents_4->setObjectName(QString::fromUtf8("dockWidgetContents_4"));
        dockWidget_CFG->setWidget(dockWidgetContents_4);
        QCMNRClass->addDockWidget(Qt::RightDockWidgetArea, dockWidget_CFG);
        dockWidget_Flash = new QDockWidget(QCMNRClass);
        dockWidget_Flash->setObjectName(QString::fromUtf8("dockWidget_Flash"));
        dockWidget_Flash->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        dockWidgetContents_5 = new QWidget();
        dockWidgetContents_5->setObjectName(QString::fromUtf8("dockWidgetContents_5"));
        dockWidget_Flash->setWidget(dockWidgetContents_5);
        QCMNRClass->addDockWidget(Qt::RightDockWidgetArea, dockWidget_Flash);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuIC->menuAction());

        retranslateUi(QCMNRClass);

        QMetaObject::connectSlotsByName(QCMNRClass);
    } // setupUi

    void retranslateUi(QMainWindow *QCMNRClass)
    {
        QCMNRClass->setWindowTitle(QCoreApplication::translate("QCMNRClass", "HFST", nullptr));
        menuFile->setTitle(QCoreApplication::translate("QCMNRClass", "File", nullptr));
        menuIC->setTitle(QCoreApplication::translate("QCMNRClass", "IC", nullptr));
        dockLog->setWindowTitle(QCoreApplication::translate("QCMNRClass", "\347\263\273\347\273\237\346\227\245\345\277\227", nullptr));
        dockWidget_Raw->setWindowTitle(QCoreApplication::translate("QCMNRClass", "Raw", nullptr));
        dockWidget_Canvas->setWindowTitle(QCoreApplication::translate("QCMNRClass", "Canvas", nullptr));
        dockWidget_CFG->setWindowTitle(QCoreApplication::translate("QCMNRClass", "CFG", nullptr));
        dockWidget_Flash->setWindowTitle(QCoreApplication::translate("QCMNRClass", "LCM", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QCMNRClass: public Ui_QCMNRClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QCMNR_H
