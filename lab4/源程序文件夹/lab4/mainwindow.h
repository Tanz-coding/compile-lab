#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "grammar.h"

class QPlainTextEdit;
class QTableWidget;
class QLineEdit;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // 文法
    Grammar *grammar;

private slots:
    void on_actionOpenGrammar_triggered();
    void on_actionSaveGrammar_triggered();
    void on_actionComputeFirstFollow_triggered();
    void on_actionBuildLR0SLR_triggered();
    void on_actionBuildLR1Table_triggered();
    void on_actionAnalyzeSentence_triggered();
};
#endif // MAINWINDOW_H
