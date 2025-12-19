/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *sourceGroup;
    QVBoxLayout *verticalLayout;
    QPlainTextEdit *sourceEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *openButton;
    QPushButton *saveButton;
    QPushButton *clearButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *generateButton;
    QGroupBox *quadGroup;
    QVBoxLayout *verticalLayout_2;
    QTableWidget *quadTable;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(960, 640);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_3 = new QVBoxLayout(centralwidget);
        verticalLayout_3->setObjectName("verticalLayout_3");
        sourceGroup = new QGroupBox(centralwidget);
        sourceGroup->setObjectName("sourceGroup");
        verticalLayout = new QVBoxLayout(sourceGroup);
        verticalLayout->setObjectName("verticalLayout");
        sourceEdit = new QPlainTextEdit(sourceGroup);
        sourceEdit->setObjectName("sourceEdit");
        sourceEdit->setTabStopDistance(32.000000000000000);

        verticalLayout->addWidget(sourceEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        openButton = new QPushButton(sourceGroup);
        openButton->setObjectName("openButton");

        horizontalLayout->addWidget(openButton);

        saveButton = new QPushButton(sourceGroup);
        saveButton->setObjectName("saveButton");

        horizontalLayout->addWidget(saveButton);

        clearButton = new QPushButton(sourceGroup);
        clearButton->setObjectName("clearButton");

        horizontalLayout->addWidget(clearButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        generateButton = new QPushButton(sourceGroup);
        generateButton->setObjectName("generateButton");

        horizontalLayout->addWidget(generateButton);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_3->addWidget(sourceGroup);

        quadGroup = new QGroupBox(centralwidget);
        quadGroup->setObjectName("quadGroup");
        verticalLayout_2 = new QVBoxLayout(quadGroup);
        verticalLayout_2->setObjectName("verticalLayout_2");
        quadTable = new QTableWidget(quadGroup);
        if (quadTable->columnCount() < 5)
            quadTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        quadTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        quadTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        quadTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        quadTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        quadTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        quadTable->setObjectName("quadTable");

        verticalLayout_2->addWidget(quadTable);


        verticalLayout_3->addWidget(quadGroup);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 960, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "TINY Intermediate Code Generator", nullptr));
        sourceGroup->setTitle(QCoreApplication::translate("MainWindow", "Source Program", nullptr));
        sourceEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\234\250\346\255\244\350\276\223\345\205\245tiny\346\272\220\347\250\213\345\272\217", nullptr));
        openButton->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200", nullptr));
        saveButton->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
        clearButton->setText(QCoreApplication::translate("MainWindow", "\346\270\205\351\231\244", nullptr));
        generateButton->setText(QCoreApplication::translate("MainWindow", "\347\224\237\346\210\220\345\233\233\345\205\203\347\273\204", nullptr));
        quadGroup->setTitle(QCoreApplication::translate("MainWindow", "\345\233\233\345\205\203\347\273\204", nullptr));
        QTableWidgetItem *___qtablewidgetitem = quadTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "#", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = quadTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Op", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = quadTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Arg1", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = quadTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Arg2", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = quadTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Result", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
