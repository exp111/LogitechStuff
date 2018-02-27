/********************************************************************************
** Form generated from reading UI file 'QtConfig.ui'
**
** Created by: Qt User Interface Compiler version 5.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTCONFIG_H
#define UI_QTCONFIG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtConfig
{
public:
    QPushButton *closeButton;
    QPushButton *initButton;
    QPushButton *shutdownButton;
    QLabel *label;
    QPushButton *addButton;

    void setupUi(QWidget *QtConfig)
    {
        if (QtConfig->objectName().isEmpty())
            QtConfig->setObjectName(QStringLiteral("QtConfig"));
        QtConfig->resize(280, 114);
        closeButton = new QPushButton(QtConfig);
        closeButton->setObjectName(QStringLiteral("closeButton"));
        closeButton->setGeometry(QRect(80, 80, 100, 25));
        initButton = new QPushButton(QtConfig);
        initButton->setObjectName(QStringLiteral("initButton"));
        initButton->setGeometry(QRect(30, 40, 101, 23));
        shutdownButton = new QPushButton(QtConfig);
        shutdownButton->setObjectName(QStringLiteral("shutdownButton"));
        shutdownButton->setGeometry(QRect(144, 40, 111, 23));
        label = new QLabel(QtConfig);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(70, 10, 181, 21));
        addButton = new QPushButton(QtConfig);
        addButton->setObjectName(QStringLiteral("addButton"));
        addButton->setGeometry(QRect(245, 80, 25, 25));

        retranslateUi(QtConfig);
        QObject::connect(closeButton, SIGNAL(clicked()), QtConfig, SLOT(close()));
        QObject::connect(initButton, SIGNAL(clicked()), QtConfig, SLOT(Init()));
        QObject::connect(shutdownButton, SIGNAL(clicked()), QtConfig, SLOT(Shutdown()));
        QObject::connect(addButton, SIGNAL(clicked()), QtConfig, SLOT(addPreset()));
        QObject::connect(closeButton, SIGNAL(clicked()), QtConfig, SLOT(save()));

        QMetaObject::connectSlotsByName(QtConfig);
    } // setupUi

    void retranslateUi(QWidget *QtConfig)
    {
        QtConfig->setWindowTitle(QApplication::translate("QtConfig", "Logitech G19 Plugin", Q_NULLPTR));
        closeButton->setText(QApplication::translate("QtConfig", "Close", Q_NULLPTR));
        initButton->setText(QApplication::translate("QtConfig", "Init", Q_NULLPTR));
        shutdownButton->setText(QApplication::translate("QtConfig", "Shutdown", Q_NULLPTR));
        label->setText(QApplication::translate("QtConfig", "Manual Init/Shutdown", Q_NULLPTR));
        addButton->setText(QApplication::translate("QtConfig", "+", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class QtConfig: public Ui_QtConfig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTCONFIG_H
