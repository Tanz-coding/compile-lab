#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <functional>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "SyntaxTreeNode.h"

class QTextEdit;
class QTextBrowser;
class QTableWidget;
class QTreeWidget;
class QPushButton;
class QAction;
class QProgressDialog;
class TinyHighlighter;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openFile();
    void saveFile();
    void performLexicalAnalysis();
    void performSyntaxAnalysis();
    void toggleTreeStyle();
    void showAboutDialog();

private:
    void setupUi();
    void setupMenus();
    void setupConnections();
    void configureEditors();
    void runWithProgress(const QString &label, const std::function<void()> &task);
    bool loadFromFile(const QString &filePath);
    bool writeToFile(const QString &filePath);
    void updateWindowTitle();
    void updateStatusBar(const QString &message, int timeoutMs = 5000);
    QVector<LexicalAnalyzer::Token> executeLexicalAnalysis(bool updateView);
    void populateLexicalResults(const QVector<LexicalAnalyzer::Token> &tokens,
                                const QVector<LexicalAnalyzer::AnalysisError> &errors);
    void populateSyntaxResults(const QVector<LexicalAnalyzer::AnalysisError> &errors);
    void populateSyntaxTree(const std::shared_ptr<SyntaxTreeNode> &root);
    void clearSyntaxTree();
    bool maybeWarnOnUnsavedChanges();

    QTextEdit *m_sourceEditor;
    QTextBrowser *m_lexicalResultBrowser;
    QTableWidget *m_syntaxResultTable;
    QTreeWidget *m_syntaxTreeWidget;
    QPushButton *m_treeStyleButton;

    QAction *m_actionOpen;
    QAction *m_actionSave;
    QAction *m_actionExit;
    QAction *m_actionLexical;
    QAction *m_actionSyntax;
    QAction *m_actionGenerateTree;
    QAction *m_actionAbout;

    QString m_currentFilePath;
    LexicalAnalyzer m_lexicalAnalyzer;
    QVector<LexicalAnalyzer::Token> m_lastTokens;
    QVector<LexicalAnalyzer::AnalysisError> m_lastLexicalErrors;

    bool m_useAlternateTreeStyle;
    TinyHighlighter *m_highlighter;
};

#endif // MAINWINDOW_H
