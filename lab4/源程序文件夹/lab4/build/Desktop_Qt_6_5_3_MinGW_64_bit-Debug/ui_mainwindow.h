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
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpenGrammar;
    QAction *actionSaveGrammar;
    QAction *actionComputeFirstFollow;
    QAction *actionBuildLR0SLR;
    QAction *actionBuildLR1Table;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tabGrammar;
    QVBoxLayout *verticalLayout_grammar;
    QPlainTextEdit *grammarEdit;
    QWidget *tabFirstFollow;
    QVBoxLayout *verticalLayout_firstfollow;
    QTableWidget *tableFirst;
    QTableWidget *tableFollow;
    QWidget *tabLR0;
    QVBoxLayout *verticalLayout_lr0;
    QTableWidget *tableLR0States;
    QTableWidget *tableLR0Trans;
    QWidget *tabSLR;
    QVBoxLayout *verticalLayout_slr;
    QLabel *labelSLRResult;
    QTableWidget *tableSLR;
    QPlainTextEdit *plainTextSLRConflicts;
    QWidget *tabLR1DFA;
    QVBoxLayout *verticalLayout_lr1dfa;
    QTableWidget *tableLR1States;
    QTableWidget *tableLR1Trans;
    QWidget *tabLR1Table;
    QVBoxLayout *verticalLayout_lr1table;
    QTableWidget *tableLR1Parse;
    QWidget *tabSentence;
    QVBoxLayout *verticalLayout_sentence;
    QHBoxLayout *horizontalLayout_sentence;
    QLabel *labelSentence;
    QLineEdit *lineSentence;
    QTableWidget *tableSteps;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuAnalyze;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1000, 700);
        actionOpenGrammar = new QAction(MainWindow);
        actionOpenGrammar->setObjectName("actionOpenGrammar");
        actionSaveGrammar = new QAction(MainWindow);
        actionSaveGrammar->setObjectName("actionSaveGrammar");
        actionComputeFirstFollow = new QAction(MainWindow);
        actionComputeFirstFollow->setObjectName("actionComputeFirstFollow");
        actionBuildLR0SLR = new QAction(MainWindow);
        actionBuildLR0SLR->setObjectName("actionBuildLR0SLR");
        actionBuildLR1Table = new QAction(MainWindow);
        actionBuildLR1Table->setObjectName("actionBuildLR1Table");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabGrammar = new QWidget();
        tabGrammar->setObjectName("tabGrammar");
        verticalLayout_grammar = new QVBoxLayout(tabGrammar);
        verticalLayout_grammar->setObjectName("verticalLayout_grammar");
        grammarEdit = new QPlainTextEdit(tabGrammar);
        grammarEdit->setObjectName("grammarEdit");

        verticalLayout_grammar->addWidget(grammarEdit);

        tabWidget->addTab(tabGrammar, QString());
        tabFirstFollow = new QWidget();
        tabFirstFollow->setObjectName("tabFirstFollow");
        verticalLayout_firstfollow = new QVBoxLayout(tabFirstFollow);
        verticalLayout_firstfollow->setObjectName("verticalLayout_firstfollow");
        tableFirst = new QTableWidget(tabFirstFollow);
        tableFirst->setObjectName("tableFirst");

        verticalLayout_firstfollow->addWidget(tableFirst);

        tableFollow = new QTableWidget(tabFirstFollow);
        tableFollow->setObjectName("tableFollow");

        verticalLayout_firstfollow->addWidget(tableFollow);

        tabWidget->addTab(tabFirstFollow, QString());
        tabLR0 = new QWidget();
        tabLR0->setObjectName("tabLR0");
        verticalLayout_lr0 = new QVBoxLayout(tabLR0);
        verticalLayout_lr0->setObjectName("verticalLayout_lr0");
        tableLR0States = new QTableWidget(tabLR0);
        tableLR0States->setObjectName("tableLR0States");

        verticalLayout_lr0->addWidget(tableLR0States);

        tableLR0Trans = new QTableWidget(tabLR0);
        tableLR0Trans->setObjectName("tableLR0Trans");

        verticalLayout_lr0->addWidget(tableLR0Trans);

        tabWidget->addTab(tabLR0, QString());
        tabSLR = new QWidget();
        tabSLR->setObjectName("tabSLR");
        verticalLayout_slr = new QVBoxLayout(tabSLR);
        verticalLayout_slr->setObjectName("verticalLayout_slr");
        labelSLRResult = new QLabel(tabSLR);
        labelSLRResult->setObjectName("labelSLRResult");

        verticalLayout_slr->addWidget(labelSLRResult);

        tableSLR = new QTableWidget(tabSLR);
        tableSLR->setObjectName("tableSLR");

        verticalLayout_slr->addWidget(tableSLR);

        plainTextSLRConflicts = new QPlainTextEdit(tabSLR);
        plainTextSLRConflicts->setObjectName("plainTextSLRConflicts");

        verticalLayout_slr->addWidget(plainTextSLRConflicts);

        tabWidget->addTab(tabSLR, QString());
        tabLR1DFA = new QWidget();
        tabLR1DFA->setObjectName("tabLR1DFA");
        verticalLayout_lr1dfa = new QVBoxLayout(tabLR1DFA);
        verticalLayout_lr1dfa->setObjectName("verticalLayout_lr1dfa");
        tableLR1States = new QTableWidget(tabLR1DFA);
        tableLR1States->setObjectName("tableLR1States");

        verticalLayout_lr1dfa->addWidget(tableLR1States);

        tableLR1Trans = new QTableWidget(tabLR1DFA);
        tableLR1Trans->setObjectName("tableLR1Trans");

        verticalLayout_lr1dfa->addWidget(tableLR1Trans);

        tabWidget->addTab(tabLR1DFA, QString());
        tabLR1Table = new QWidget();
        tabLR1Table->setObjectName("tabLR1Table");
        verticalLayout_lr1table = new QVBoxLayout(tabLR1Table);
        verticalLayout_lr1table->setObjectName("verticalLayout_lr1table");
        tableLR1Parse = new QTableWidget(tabLR1Table);
        tableLR1Parse->setObjectName("tableLR1Parse");

        verticalLayout_lr1table->addWidget(tableLR1Parse);

        tabWidget->addTab(tabLR1Table, QString());
        tabSentence = new QWidget();
        tabSentence->setObjectName("tabSentence");
        tabSentence->setEnabled(true);
        verticalLayout_sentence = new QVBoxLayout(tabSentence);
        verticalLayout_sentence->setObjectName("verticalLayout_sentence");
        horizontalLayout_sentence = new QHBoxLayout();
        horizontalLayout_sentence->setObjectName("horizontalLayout_sentence");
        labelSentence = new QLabel(tabSentence);
        labelSentence->setObjectName("labelSentence");

        horizontalLayout_sentence->addWidget(labelSentence);

        lineSentence = new QLineEdit(tabSentence);
        lineSentence->setObjectName("lineSentence");

        horizontalLayout_sentence->addWidget(lineSentence);


        verticalLayout_sentence->addLayout(horizontalLayout_sentence);

        tableSteps = new QTableWidget(tabSentence);
        tableSteps->setObjectName("tableSteps");

        verticalLayout_sentence->addWidget(tableSteps);

        tabWidget->addTab(tabSentence, QString());

        verticalLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1000, 21));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuAnalyze = new QMenu(menubar);
        menuAnalyze->setObjectName("menuAnalyze");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuAnalyze->menuAction());
        menuFile->addAction(actionOpenGrammar);
        menuFile->addAction(actionSaveGrammar);
        menuAnalyze->addAction(actionComputeFirstFollow);
        menuAnalyze->addAction(actionBuildLR0SLR);
        menuAnalyze->addAction(actionBuildLR1Table);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LR(1) \345\210\206\346\236\220\347\224\237\346\210\220\345\231\250", nullptr));
        actionOpenGrammar->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\346\226\207\346\263\225", nullptr));
        actionSaveGrammar->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230\346\226\207\346\263\225", nullptr));
        actionComputeFirstFollow->setText(QCoreApplication::translate("MainWindow", "\350\256\241\347\256\227 FIRST/FOLLOW", nullptr));
        actionBuildLR0SLR->setText(QCoreApplication::translate("MainWindow", "\346\236\204\351\200\240 LR(0)/SLR", nullptr));
        actionBuildLR1Table->setText(QCoreApplication::translate("MainWindow", "\346\236\204\351\200\240 LR(1) \350\241\250", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabGrammar), QCoreApplication::translate("MainWindow", "\346\226\207\346\263\225", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabFirstFollow), QCoreApplication::translate("MainWindow", "FIRST/FOLLOW", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabLR0), QCoreApplication::translate("MainWindow", "LR(0) DFA", nullptr));
        labelSLRResult->setText(QCoreApplication::translate("MainWindow", "\345\260\232\346\234\252\345\210\244\346\226\255", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabSLR), QCoreApplication::translate("MainWindow", "SLR(1) \345\210\244\346\226\255", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabLR1DFA), QCoreApplication::translate("MainWindow", "LR(1) DFA", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabLR1Table), QCoreApplication::translate("MainWindow", "LR(1) \345\210\206\346\236\220\350\241\250", nullptr));
        labelSentence->setText(QCoreApplication::translate("MainWindow", "\350\276\223\345\205\245\345\217\245\345\255\220\357\274\232", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabSentence), QCoreApplication::translate("MainWindow", "\345\217\245\345\255\220\345\210\206\346\236\220", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr));
        menuAnalyze->setTitle(QCoreApplication::translate("MainWindow", "\345\210\206\346\236\220", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
