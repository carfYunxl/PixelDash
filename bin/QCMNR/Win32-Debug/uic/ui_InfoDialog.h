/********************************************************************************
** Form generated from reading UI file 'InfoDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INFODIALOG_H
#define UI_INFODIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_InfoDialogClass
{
public:
    QGridLayout *gridLayout_3;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_11;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QLineEdit *lineEdit_ChipID;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_9;
    QLineEdit *lineEdit_Firmware_Version;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_5;
    QLineEdit *lineEdit_Protocol_Version;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_8;
    QLineEdit *lineEdit_KeyCount;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_4;
    QLineEdit *lineEdit_Version;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit_NumX;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *lineEdit_NumY;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QLineEdit *lineEdit_ResX;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_7;
    QLineEdit *lineEdit_ResY;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout_4;
    QComboBox *cbsCommunication;
    QComboBox *cbsProtocol;
    QPushButton *btnReconnect;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_10;
    QComboBox *cbsVDD;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_11;
    QComboBox *cbsIOVDD;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_12;
    QComboBox *cbsAddr;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_13;
    QComboBox *cbsI2CClock;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_10;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *okButton;
    QSpacerItem *horizontalSpacer_4;

    void setupUi(QDialog *InfoDialogClass)
    {
        if (InfoDialogClass->objectName().isEmpty())
            InfoDialogClass->setObjectName(QString::fromUtf8("InfoDialogClass"));
        InfoDialogClass->resize(430, 428);
        gridLayout_3 = new QGridLayout(InfoDialogClass);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        groupBox = new QGroupBox(InfoDialogClass);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font.setPointSize(12);
        groupBox->setFont(font);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(60, 0));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font1.setPointSize(10);
        label_3->setFont(font1);
        label_3->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(label_3);

        lineEdit_ChipID = new QLineEdit(groupBox);
        lineEdit_ChipID->setObjectName(QString::fromUtf8("lineEdit_ChipID"));
        lineEdit_ChipID->setFont(font1);
        lineEdit_ChipID->setAlignment(Qt::AlignCenter);
        lineEdit_ChipID->setReadOnly(true);

        horizontalLayout_3->addWidget(lineEdit_ChipID);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setMinimumSize(QSize(60, 0));
        label_9->setFont(font1);
        label_9->setAlignment(Qt::AlignCenter);

        horizontalLayout_9->addWidget(label_9);

        lineEdit_Firmware_Version = new QLineEdit(groupBox);
        lineEdit_Firmware_Version->setObjectName(QString::fromUtf8("lineEdit_Firmware_Version"));
        lineEdit_Firmware_Version->setFont(font1);
        lineEdit_Firmware_Version->setAlignment(Qt::AlignCenter);
        lineEdit_Firmware_Version->setReadOnly(true);

        horizontalLayout_9->addWidget(lineEdit_Firmware_Version);


        verticalLayout_2->addLayout(horizontalLayout_9);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(60, 0));
        label_5->setFont(font1);
        label_5->setAlignment(Qt::AlignCenter);

        horizontalLayout_5->addWidget(label_5);

        lineEdit_Protocol_Version = new QLineEdit(groupBox);
        lineEdit_Protocol_Version->setObjectName(QString::fromUtf8("lineEdit_Protocol_Version"));
        lineEdit_Protocol_Version->setFont(font1);
        lineEdit_Protocol_Version->setAlignment(Qt::AlignCenter);
        lineEdit_Protocol_Version->setReadOnly(true);

        horizontalLayout_5->addWidget(lineEdit_Protocol_Version);


        verticalLayout_2->addLayout(horizontalLayout_5);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy);
        label_8->setMinimumSize(QSize(60, 20));
        label_8->setMaximumSize(QSize(16777215, 20));
        label_8->setFont(font1);
        label_8->setAlignment(Qt::AlignCenter);

        horizontalLayout_8->addWidget(label_8);

        lineEdit_KeyCount = new QLineEdit(groupBox);
        lineEdit_KeyCount->setObjectName(QString::fromUtf8("lineEdit_KeyCount"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lineEdit_KeyCount->sizePolicy().hasHeightForWidth());
        lineEdit_KeyCount->setSizePolicy(sizePolicy1);
        lineEdit_KeyCount->setMinimumSize(QSize(0, 0));
        lineEdit_KeyCount->setMaximumSize(QSize(16777215, 16777215));
        lineEdit_KeyCount->setFont(font1);
        lineEdit_KeyCount->setAlignment(Qt::AlignCenter);
        lineEdit_KeyCount->setReadOnly(true);

        horizontalLayout_8->addWidget(lineEdit_KeyCount);


        verticalLayout_2->addLayout(horizontalLayout_8);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(60, 0));
        label_4->setFont(font1);
        label_4->setAlignment(Qt::AlignCenter);

        horizontalLayout_4->addWidget(label_4);

        lineEdit_Version = new QLineEdit(groupBox);
        lineEdit_Version->setObjectName(QString::fromUtf8("lineEdit_Version"));
        lineEdit_Version->setFont(font1);
        lineEdit_Version->setAlignment(Qt::AlignCenter);
        lineEdit_Version->setReadOnly(true);

        horizontalLayout_4->addWidget(lineEdit_Version);


        verticalLayout_2->addLayout(horizontalLayout_4);


        horizontalLayout_11->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(60, 0));
        label->setFont(font1);
        label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label);

        lineEdit_NumX = new QLineEdit(groupBox);
        lineEdit_NumX->setObjectName(QString::fromUtf8("lineEdit_NumX"));
        lineEdit_NumX->setFont(font1);
        lineEdit_NumX->setAlignment(Qt::AlignCenter);
        lineEdit_NumX->setReadOnly(true);

        horizontalLayout->addWidget(lineEdit_NumX);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(60, 0));
        label_2->setFont(font1);
        label_2->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_2);

        lineEdit_NumY = new QLineEdit(groupBox);
        lineEdit_NumY->setObjectName(QString::fromUtf8("lineEdit_NumY"));
        lineEdit_NumY->setFont(font1);
        lineEdit_NumY->setAlignment(Qt::AlignCenter);
        lineEdit_NumY->setReadOnly(true);

        horizontalLayout_2->addWidget(lineEdit_NumY);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setMinimumSize(QSize(60, 0));
        label_6->setFont(font1);
        label_6->setAlignment(Qt::AlignCenter);

        horizontalLayout_6->addWidget(label_6);

        lineEdit_ResX = new QLineEdit(groupBox);
        lineEdit_ResX->setObjectName(QString::fromUtf8("lineEdit_ResX"));
        lineEdit_ResX->setFont(font1);
        lineEdit_ResX->setAlignment(Qt::AlignCenter);
        lineEdit_ResX->setReadOnly(true);

        horizontalLayout_6->addWidget(lineEdit_ResX);


        verticalLayout->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setMinimumSize(QSize(60, 0));
        label_7->setFont(font1);
        label_7->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(label_7);

        lineEdit_ResY = new QLineEdit(groupBox);
        lineEdit_ResY->setObjectName(QString::fromUtf8("lineEdit_ResY"));
        lineEdit_ResY->setFont(font1);
        lineEdit_ResY->setAlignment(Qt::AlignCenter);
        lineEdit_ResY->setReadOnly(true);

        horizontalLayout_7->addWidget(lineEdit_ResY);


        verticalLayout->addLayout(horizontalLayout_7);

        verticalSpacer = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Ignored);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout_11->addLayout(verticalLayout);


        gridLayout->addLayout(horizontalLayout_11, 0, 0, 1, 1);


        gridLayout_3->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(InfoDialogClass);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setFont(font);
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        cbsCommunication = new QComboBox(groupBox_2);
        cbsCommunication->addItem(QString());
        cbsCommunication->addItem(QString());
        cbsCommunication->addItem(QString());
        cbsCommunication->addItem(QString());
        cbsCommunication->setObjectName(QString::fromUtf8("cbsCommunication"));
        cbsCommunication->setFont(font1);

        verticalLayout_4->addWidget(cbsCommunication);

        cbsProtocol = new QComboBox(groupBox_2);
        cbsProtocol->addItem(QString());
        cbsProtocol->addItem(QString());
        cbsProtocol->setObjectName(QString::fromUtf8("cbsProtocol"));
        cbsProtocol->setFont(font1);

        verticalLayout_4->addWidget(cbsProtocol);

        btnReconnect = new QPushButton(groupBox_2);
        btnReconnect->setObjectName(QString::fromUtf8("btnReconnect"));
        btnReconnect->setMinimumSize(QSize(0, 40));
        QFont font2;
        font2.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font2.setPointSize(11);
        btnReconnect->setFont(font2);

        verticalLayout_4->addWidget(btnReconnect);


        gridLayout_2->addLayout(verticalLayout_4, 0, 0, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setSpacing(6);
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label_10 = new QLabel(groupBox_2);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setMinimumSize(QSize(60, 0));
        label_10->setFont(font1);
        label_10->setAlignment(Qt::AlignCenter);

        horizontalLayout_12->addWidget(label_10);

        cbsVDD = new QComboBox(groupBox_2);
        cbsVDD->addItem(QString());
        cbsVDD->addItem(QString());
        cbsVDD->addItem(QString());
        cbsVDD->addItem(QString());
        cbsVDD->addItem(QString());
        cbsVDD->setObjectName(QString::fromUtf8("cbsVDD"));
        cbsVDD->setFont(font1);
        cbsVDD->setEditable(true);

        horizontalLayout_12->addWidget(cbsVDD);


        verticalLayout_3->addLayout(horizontalLayout_12);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setSpacing(6);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        label_11 = new QLabel(groupBox_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setMinimumSize(QSize(60, 0));
        label_11->setFont(font1);
        label_11->setAlignment(Qt::AlignCenter);

        horizontalLayout_13->addWidget(label_11);

        cbsIOVDD = new QComboBox(groupBox_2);
        cbsIOVDD->addItem(QString());
        cbsIOVDD->addItem(QString());
        cbsIOVDD->addItem(QString());
        cbsIOVDD->addItem(QString());
        cbsIOVDD->addItem(QString());
        cbsIOVDD->setObjectName(QString::fromUtf8("cbsIOVDD"));
        cbsIOVDD->setFont(font1);
        cbsIOVDD->setEditable(true);

        horizontalLayout_13->addWidget(cbsIOVDD);


        verticalLayout_3->addLayout(horizontalLayout_13);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setSpacing(6);
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        label_12 = new QLabel(groupBox_2);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setMinimumSize(QSize(60, 0));
        label_12->setFont(font1);
        label_12->setAlignment(Qt::AlignCenter);

        horizontalLayout_14->addWidget(label_12);

        cbsAddr = new QComboBox(groupBox_2);
        cbsAddr->setObjectName(QString::fromUtf8("cbsAddr"));
        cbsAddr->setFont(font1);

        horizontalLayout_14->addWidget(cbsAddr);


        verticalLayout_3->addLayout(horizontalLayout_14);

        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setSpacing(6);
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label_13 = new QLabel(groupBox_2);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setMinimumSize(QSize(60, 0));
        label_13->setFont(font1);
        label_13->setAlignment(Qt::AlignCenter);

        horizontalLayout_15->addWidget(label_13);

        cbsI2CClock = new QComboBox(groupBox_2);
        cbsI2CClock->addItem(QString());
        cbsI2CClock->addItem(QString());
        cbsI2CClock->addItem(QString());
        cbsI2CClock->addItem(QString());
        cbsI2CClock->addItem(QString());
        cbsI2CClock->addItem(QString());
        cbsI2CClock->setObjectName(QString::fromUtf8("cbsI2CClock"));
        cbsI2CClock->setFont(font1);
        cbsI2CClock->setEditable(true);

        horizontalLayout_15->addWidget(cbsI2CClock);


        verticalLayout_3->addLayout(horizontalLayout_15);


        gridLayout_2->addLayout(verticalLayout_3, 0, 1, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_2, 1, 0, 1, 1);


        gridLayout_3->addWidget(groupBox_2, 1, 0, 1, 1);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setSpacing(6);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        horizontalLayout_10->setSizeConstraint(QLayout::SetMinimumSize);
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_10->addItem(horizontalSpacer_3);

        okButton = new QPushButton(InfoDialogClass);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        sizePolicy1.setHeightForWidth(okButton->sizePolicy().hasHeightForWidth());
        okButton->setSizePolicy(sizePolicy1);
        okButton->setMinimumSize(QSize(0, 40));
        QFont font3;
        font3.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font3.setPointSize(14);
        okButton->setFont(font3);

        horizontalLayout_10->addWidget(okButton);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_10->addItem(horizontalSpacer_4);


        gridLayout_3->addLayout(horizontalLayout_10, 2, 0, 1, 1);


        retranslateUi(InfoDialogClass);
        QObject::connect(okButton, SIGNAL(clicked()), InfoDialogClass, SLOT(close()));

        QMetaObject::connectSlotsByName(InfoDialogClass);
    } // setupUi

    void retranslateUi(QDialog *InfoDialogClass)
    {
        InfoDialogClass->setWindowTitle(QCoreApplication::translate("InfoDialogClass", "InfoDialog", nullptr));
        groupBox->setTitle(QCoreApplication::translate("InfoDialogClass", "Info", nullptr));
        label_3->setText(QCoreApplication::translate("InfoDialogClass", "IC\345\236\213\345\217\267", nullptr));
        label_9->setText(QCoreApplication::translate("InfoDialogClass", "\345\233\272\344\273\266\347\211\210\345\217\267", nullptr));
        label_5->setText(QCoreApplication::translate("InfoDialogClass", "\345\215\217\350\256\256\347\211\210\346\234\254", nullptr));
        label_8->setText(QCoreApplication::translate("InfoDialogClass", "Key \344\270\252\346\225\260", nullptr));
        label_4->setText(QCoreApplication::translate("InfoDialogClass", "\345\233\272\344\273\266\347\211\210\346\234\254", nullptr));
        label->setText(QCoreApplication::translate("InfoDialogClass", "X \346\225\260\351\207\217", nullptr));
        label_2->setText(QCoreApplication::translate("InfoDialogClass", "Y \346\225\260\351\207\217", nullptr));
        label_6->setText(QCoreApplication::translate("InfoDialogClass", "X \345\210\206\350\276\250\347\216\207", nullptr));
        label_7->setText(QCoreApplication::translate("InfoDialogClass", "Y \345\210\206\350\276\250\347\216\207", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("InfoDialogClass", "Settings", nullptr));
        cbsCommunication->setItemText(0, QCoreApplication::translate("InfoDialogClass", "TouchLink", nullptr));
        cbsCommunication->setItemText(1, QCoreApplication::translate("InfoDialogClass", "TouchPad", nullptr));
        cbsCommunication->setItemText(2, QCoreApplication::translate("InfoDialogClass", "ADB", nullptr));
        cbsCommunication->setItemText(3, QCoreApplication::translate("InfoDialogClass", "WIFI", nullptr));

        cbsProtocol->setItemText(0, QCoreApplication::translate("InfoDialogClass", "I2C", nullptr));
        cbsProtocol->setItemText(1, QCoreApplication::translate("InfoDialogClass", "SPI", nullptr));

        btnReconnect->setText(QCoreApplication::translate("InfoDialogClass", "\351\207\215\350\277\236", nullptr));
        label_10->setText(QCoreApplication::translate("InfoDialogClass", "VDD(v)", nullptr));
        cbsVDD->setItemText(0, QCoreApplication::translate("InfoDialogClass", "3.6", nullptr));
        cbsVDD->setItemText(1, QCoreApplication::translate("InfoDialogClass", "3.3", nullptr));
        cbsVDD->setItemText(2, QCoreApplication::translate("InfoDialogClass", "2.8", nullptr));
        cbsVDD->setItemText(3, QCoreApplication::translate("InfoDialogClass", "1.8", nullptr));
        cbsVDD->setItemText(4, QCoreApplication::translate("InfoDialogClass", "1.2", nullptr));

        label_11->setText(QCoreApplication::translate("InfoDialogClass", "IOVDD(v)", nullptr));
        cbsIOVDD->setItemText(0, QCoreApplication::translate("InfoDialogClass", "3.6", nullptr));
        cbsIOVDD->setItemText(1, QCoreApplication::translate("InfoDialogClass", "3.3", nullptr));
        cbsIOVDD->setItemText(2, QCoreApplication::translate("InfoDialogClass", "2.8", nullptr));
        cbsIOVDD->setItemText(3, QCoreApplication::translate("InfoDialogClass", "1.8", nullptr));
        cbsIOVDD->setItemText(4, QCoreApplication::translate("InfoDialogClass", "1.2", nullptr));

        label_12->setText(QCoreApplication::translate("InfoDialogClass", "I2C \345\234\260\345\235\200(Hex)", nullptr));
        label_13->setText(QCoreApplication::translate("InfoDialogClass", "I2C \351\200\237\347\216\207(Hz)", nullptr));
        cbsI2CClock->setItemText(0, QCoreApplication::translate("InfoDialogClass", "2000000", nullptr));
        cbsI2CClock->setItemText(1, QCoreApplication::translate("InfoDialogClass", "1500000", nullptr));
        cbsI2CClock->setItemText(2, QCoreApplication::translate("InfoDialogClass", "1000000", nullptr));
        cbsI2CClock->setItemText(3, QCoreApplication::translate("InfoDialogClass", "700000", nullptr));
        cbsI2CClock->setItemText(4, QCoreApplication::translate("InfoDialogClass", "400000", nullptr));
        cbsI2CClock->setItemText(5, QCoreApplication::translate("InfoDialogClass", "100000", nullptr));

        okButton->setText(QCoreApplication::translate("InfoDialogClass", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InfoDialogClass: public Ui_InfoDialogClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INFODIALOG_H
