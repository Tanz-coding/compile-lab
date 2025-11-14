#include "mainwindow.h"

#include "TinyHighlighter.h"

#include <QTextEdit>
#include <QTextBrowser>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QFileInfo>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFont>
#include <QFontMetrics>
#include <QAbstractItemView>
#include <QBrush>
#include <QStringList>
#include <QDateTime>
#include <QTextDocument>
#include <QStringConverter>

namespace
{
constexpr int DEFAULT_WIDTH = 1000;
constexpr int DEFAULT_HEIGHT = 800;
QString tokenToDisplayText(const LexicalAnalyzer::Token &token)
{
    return QStringLiteral("%1-%2-%3-%4")
        .arg(token.line)
        .arg(token.column)
        .arg(LexicalAnalyzer::tokenTypeToString(token.type))
        .arg(token.lexeme);
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_sourceEditor(nullptr)
    , m_lexicalResultBrowser(nullptr)
    , m_syntaxResultTable(nullptr)
    , m_syntaxTreeWidget(nullptr)
    , m_treeStyleButton(nullptr)
    , m_actionOpen(nullptr)
    , m_actionSave(nullptr)
    , m_actionExit(nullptr)
    , m_actionLexical(nullptr)
    , m_actionSyntax(nullptr)
    , m_actionGenerateTree(nullptr)
    , m_actionAbout(nullptr)
    , m_useAlternateTreeStyle(false)
    , m_highlighter(nullptr)
{
    setupUi();
    setupMenus();
    setupConnections();
    configureEditors();
    updateWindowTitle();
    updateStatusBar(tr("就绪"));
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("Tiny语法树生成器 V1.0"));
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    setMinimumSize(800, 600);

    auto *central = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    auto *splitter = new QSplitter(Qt::Horizontal, central);
    mainLayout->addWidget(splitter);

    m_sourceEditor = new QTextEdit(splitter);
    m_sourceEditor->setObjectName(QStringLiteral("textEdit_Source"));

    auto *tabWidget = new QTabWidget(splitter);
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->setTabsClosable(false);
    tabWidget->setMovable(false);

    // 词法分析结果标签页
    m_lexicalResultBrowser = new QTextBrowser(tabWidget);
    m_lexicalResultBrowser->setObjectName(QStringLiteral("textBrowser_LexicalResult"));
    tabWidget->addTab(m_lexicalResultBrowser, tr("词法分析结果"));

    // 语法分析结果标签页
    auto *syntaxTab = new QWidget(tabWidget);
    auto *syntaxLayout = new QVBoxLayout(syntaxTab);
    syntaxLayout->setContentsMargins(0, 0, 0, 0);
    syntaxLayout->setSpacing(6);

    m_syntaxResultTable = new QTableWidget(syntaxTab);
    m_syntaxResultTable->setObjectName(QStringLiteral("tableWidget_SyntaxResult"));
    m_syntaxResultTable->setColumnCount(4);
    m_syntaxResultTable->setHorizontalHeaderLabels({tr("说明"), tr("文件"), tr("行"), tr("列")});
    m_syntaxResultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_syntaxResultTable->verticalHeader()->setVisible(false);
    m_syntaxResultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_syntaxResultTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_syntaxResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_syntaxResultTable->setAlternatingRowColors(true);
    syntaxLayout->addWidget(m_syntaxResultTable);
    tabWidget->addTab(syntaxTab, tr("语法分析结果"));

    // 语法树可视化标签页
    auto *treeTab = new QWidget(tabWidget);
    auto *treeLayout = new QVBoxLayout(treeTab);
    treeLayout->setContentsMargins(0, 0, 0, 0);
    treeLayout->setSpacing(6);

    m_treeStyleButton = new QPushButton(tr("改变样式"), treeTab);
    treeLayout->addWidget(m_treeStyleButton, 0, Qt::AlignLeft);

    m_syntaxTreeWidget = new QTreeWidget(treeTab);
    m_syntaxTreeWidget->setObjectName(QStringLiteral("treeWidget_SyntaxTree"));
    m_syntaxTreeWidget->setHeaderHidden(true);
    m_syntaxTreeWidget->setAlternatingRowColors(true);
    treeLayout->addWidget(m_syntaxTreeWidget, 1);
    tabWidget->addTab(treeTab, tr("语法树可视化"));

    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);

    setCentralWidget(central);

    statusBar()->showMessage(tr("欢迎使用 Tiny语法树生成器"));
}

void MainWindow::setupMenus()
{
    auto *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    m_actionOpen = fileMenu->addAction(tr("打开(&O)"));
    m_actionOpen->setShortcut(QKeySequence::Open);
    m_actionOpen->setStatusTip(tr("打开一个Tiny源程序"));

    m_actionSave = fileMenu->addAction(tr("保存(&S)"));
    m_actionSave->setShortcut(QKeySequence::Save);
    m_actionSave->setStatusTip(tr("保存当前Tiny源程序"));

    fileMenu->addSeparator();
    m_actionExit = fileMenu->addAction(tr("退出(&X)"));
    m_actionExit->setShortcut(QKeySequence::Quit);

    auto *analyzeMenu = menuBar()->addMenu(tr("分析(&A)"));
    m_actionLexical = analyzeMenu->addAction(tr("词法分析(&L)"));
    m_actionLexical->setShortcut(Qt::Key_F5);

    m_actionSyntax = analyzeMenu->addAction(tr("语法分析(&P)"));
    m_actionSyntax->setShortcut(Qt::Key_F6);

    analyzeMenu->addSeparator();
    m_actionGenerateTree = analyzeMenu->addAction(tr("生成语法树(&T)"));
    m_actionGenerateTree->setShortcut(Qt::Key_F7);
    m_actionGenerateTree->setCheckable(true);
    m_actionGenerateTree->setChecked(true);
    m_actionGenerateTree->setStatusTip(tr("是否在语法分析后生成语法树"));

    auto *helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    m_actionAbout = helpMenu->addAction(tr("关于(&A)"));
}

void MainWindow::setupConnections()
{
    connect(m_actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(m_actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(m_actionExit, &QAction::triggered, this, &QWidget::close);
    connect(m_actionLexical, &QAction::triggered, this, &MainWindow::performLexicalAnalysis);
    connect(m_actionSyntax, &QAction::triggered, this, &MainWindow::performSyntaxAnalysis);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(m_treeStyleButton, &QPushButton::clicked, this, &MainWindow::toggleTreeStyle);
    connect(m_sourceEditor->document(), &QTextDocument::modificationChanged, this, [this](bool modified) {
        updateWindowTitle();
        if (modified) {
            updateStatusBar(tr("文档已修改，记得保存"), 3000);
        }
    });
}

void MainWindow::configureEditors()
{
    QFont editorFont(QStringLiteral("Microsoft YaHei"), 12);
    m_sourceEditor->setFont(editorFont);
    const QFontMetrics editorMetrics(editorFont);
    m_sourceEditor->setTabStopDistance(editorMetrics.horizontalAdvance(QStringLiteral("    ")));
    m_sourceEditor->setAcceptRichText(false);

    m_highlighter = new TinyHighlighter(m_sourceEditor->document());

    QFont resultFont(QStringLiteral("Microsoft YaHei"), 11);
    m_lexicalResultBrowser->setFont(resultFont);

    m_syntaxResultTable->setFont(resultFont);
    m_syntaxResultTable->horizontalHeader()->setStretchLastSection(true);
    m_syntaxResultTable->setRowCount(0);

    m_syntaxTreeWidget->setFont(resultFont);
    m_syntaxTreeWidget->setAnimated(true);
}

void MainWindow::runWithProgress(const QString &label, const std::function<void()> &task)
{
    QProgressDialog progress(label, QString(), 0, 0, this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setCancelButton(nullptr);
    progress.setMinimumDuration(0);
    progress.show();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    task();
    progress.close();
    QCoreApplication::processEvents();
}

bool MainWindow::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("打开失败"), tr("无法打开文件：%1").arg(file.errorString()));
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    const QString content = stream.readAll();
    m_sourceEditor->setPlainText(content);
    m_sourceEditor->document()->setModified(false);
    m_currentFilePath = filePath;
    updateWindowTitle();
    updateStatusBar(tr("已打开文件：%1").arg(QFileInfo(filePath).fileName()));
    return true;
}

bool MainWindow::writeToFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("保存失败"), tr("无法保存文件：%1").arg(file.errorString()));
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << m_sourceEditor->toPlainText();
    file.flush();

    m_sourceEditor->document()->setModified(false);
    m_currentFilePath = filePath;
    updateWindowTitle();
    updateStatusBar(tr("已保存文件：%1").arg(QFileInfo(filePath).fileName()));
    return true;
}

void MainWindow::updateWindowTitle()
{
    const QString fileName = m_currentFilePath.isEmpty() ? tr("未命名.txt") : QFileInfo(m_currentFilePath).fileName();
    const bool modified = m_sourceEditor->document()->isModified();
    setWindowTitle(QStringLiteral("%1%2 - Tiny语法树生成器 V1.0")
                       .arg(fileName)
                       .arg(modified ? QLatin1String("*") : QLatin1String("")));
}

void MainWindow::updateStatusBar(const QString &message, int timeoutMs)
{
    statusBar()->showMessage(message, timeoutMs);
}

QVector<LexicalAnalyzer::Token> MainWindow::executeLexicalAnalysis(bool updateView)
{
    const QString source = m_sourceEditor->toPlainText();
    QVector<LexicalAnalyzer::Token> tokens;
    QVector<LexicalAnalyzer::AnalysisError> errors;

    runWithProgress(tr("正在进行词法分析..."), [&]() {
        tokens = m_lexicalAnalyzer.analyze(source);
        errors = m_lexicalAnalyzer.errors();
    });

    m_lastTokens = tokens;
    m_lastLexicalErrors = errors;

    if (updateView) {
        populateLexicalResults(tokens, errors);
    }

    return tokens;
}

void MainWindow::populateLexicalResults(const QVector<LexicalAnalyzer::Token> &tokens,
                                        const QVector<LexicalAnalyzer::AnalysisError> &errors)
{
    QStringList lines;
    lines.reserve(tokens.size() + errors.size());
    for (const auto &token : tokens) {
        if (token.type == LexicalAnalyzer::TokenType::EndOfFile) {
            continue;
        }
        lines << tokenToDisplayText(token);
    }
    if (!errors.isEmpty()) {
        lines << QString();
        for (const auto &error : errors) {
            lines << tr("[词法错误] 行%1 列%2: %3")
                         .arg(error.line)
                         .arg(error.column)
                         .arg(error.message);
        }
    }
    m_lexicalResultBrowser->setPlainText(lines.join(QLatin1Char('\n')));
}

void MainWindow::populateSyntaxResults(const QVector<LexicalAnalyzer::AnalysisError> &errors)
{
    m_syntaxResultTable->setRowCount(0);
    if (errors.isEmpty()) {
        m_syntaxResultTable->setRowCount(1);
        auto *infoItem = new QTableWidgetItem(tr("语法分析通过，无错误"));
        infoItem->setForeground(QBrush(Qt::darkGreen));
        m_syntaxResultTable->setItem(0, 0, infoItem);
        m_syntaxResultTable->setItem(0, 1, new QTableWidgetItem(m_currentFilePath.isEmpty() ? tr("未命名") : m_currentFilePath));
        m_syntaxResultTable->setItem(0, 2, new QTableWidgetItem(QStringLiteral("-")));
        m_syntaxResultTable->setItem(0, 3, new QTableWidgetItem(QStringLiteral("-")));
        return;
    }

    m_syntaxResultTable->setRowCount(errors.size());
    for (int row = 0; row < errors.size(); ++row) {
        const auto &error = errors.at(row);
        m_syntaxResultTable->setItem(row, 0, new QTableWidgetItem(error.message));
        m_syntaxResultTable->setItem(row, 1, new QTableWidgetItem(m_currentFilePath.isEmpty() ? tr("未命名") : m_currentFilePath));
        m_syntaxResultTable->setItem(row, 2, new QTableWidgetItem(QString::number(error.line)));
        m_syntaxResultTable->setItem(row, 3, new QTableWidgetItem(QString::number(error.column)));
    }
}

void MainWindow::clearSyntaxTree()
{
    m_syntaxTreeWidget->clear();
}

void MainWindow::populateSyntaxTree(const std::shared_ptr<SyntaxTreeNode> &root)
{
    clearSyntaxTree();
    if (!root) {
        return;
    }

    std::function<void(const std::shared_ptr<SyntaxTreeNode> &, QTreeWidgetItem *)> addNode;
    addNode = [this, &addNode](const std::shared_ptr<SyntaxTreeNode> &node, QTreeWidgetItem *parent) {
        if (!node) {
            return;
        }
        const QString label = node->value().isEmpty()
                                   ? node->type()
                                   : QStringLiteral("%1: %2").arg(node->type(), node->value());
        auto *item = new QTreeWidgetItem(QStringList{label});
        if (parent) {
            parent->addChild(item);
        } else {
            m_syntaxTreeWidget->addTopLevelItem(item);
        }
        for (const auto &child : node->children()) {
            addNode(child, item);
        }
    };

    addNode(root, nullptr);
    m_syntaxTreeWidget->expandAll();
}

void MainWindow::openFile()
{
    if (!maybeWarnOnUnsavedChanges()) {
        return;
    }

    const QString filePath = QFileDialog::getOpenFileName(this, tr("打开Tiny源程序"), QString(), tr("Tiny 源程序 (*.txt);;所有文件 (*.*)"));
    if (filePath.isEmpty()) {
        return;
    }
    loadFromFile(filePath);
}

void MainWindow::saveFile()
{
    QString targetPath = m_currentFilePath;
    if (targetPath.isEmpty()) {
        targetPath = QFileDialog::getSaveFileName(this, tr("保存Tiny源程序"), tr("未命名.txt"), tr("Tiny 源程序 (*.txt);;所有文件 (*.*)"));
        if (targetPath.isEmpty()) {
            return;
        }
    }
    writeToFile(targetPath);
}

void MainWindow::performLexicalAnalysis()
{
    executeLexicalAnalysis(true);
    updateStatusBar(tr("词法分析完成"));
}

void MainWindow::performSyntaxAnalysis()
{
    executeLexicalAnalysis(true);
    if (!m_lastLexicalErrors.isEmpty()) {
        populateSyntaxResults(m_lastLexicalErrors);
        clearSyntaxTree();
        updateStatusBar(tr("语法分析终止，存在词法错误"));
        return;
    }

    SyntaxAnalyzer syntaxAnalyzer(m_lastTokens);
    QVector<LexicalAnalyzer::AnalysisError> syntaxErrors;
    std::shared_ptr<SyntaxTreeNode> root;
    const bool generateTree = m_actionGenerateTree->isChecked();

    runWithProgress(tr("正在进行语法分析..."), [&]() {
        root = syntaxAnalyzer.analyze(generateTree);
        syntaxErrors = syntaxAnalyzer.errors();
    });

    populateSyntaxResults(syntaxErrors);
    if (generateTree && syntaxErrors.isEmpty()) {
        populateSyntaxTree(root);
    } else {
        clearSyntaxTree();
    }

    updateStatusBar(syntaxErrors.isEmpty() ? tr("语法分析完成，未发现错误")
                                           : tr("语法分析完成，发现%1个错误").arg(syntaxErrors.size()));
}

void MainWindow::toggleTreeStyle()
{
    m_useAlternateTreeStyle = !m_useAlternateTreeStyle;
    if (m_useAlternateTreeStyle) {
        m_syntaxTreeWidget->setStyleSheet(QStringLiteral(
            "QTreeWidget {"
            "    background: #f7f7ff;"
            "    border: 1px solid #8f8fbc;"
            "}"
            "QTreeWidget::item {"
            "    padding: 4px;"
            "}"
            "QTreeWidget::item:selected {"
            "    background: #4f6bed;"
            "    color: white;"
            "}"));
    } else {
        m_syntaxTreeWidget->setStyleSheet(QString());
    }
}

void MainWindow::showAboutDialog()
{
    const QString aboutText = tr("<h3>Tiny语法树生成器 v1.0</h3>"
                                 "<p>作者：tenz</p>"
                                 "<p>基于 Qt 6.5.3 + C++17 实现，支持扩展后的Tiny语言词法、语法分析以及语法树可视化。</p>");
    QMessageBox::about(this, tr("关于"), aboutText);
}

bool MainWindow::maybeWarnOnUnsavedChanges()
{
    if (!m_sourceEditor->document()->isModified()) {
        return true;
    }
    const auto ret = QMessageBox::question(this, tr("未保存的更改"),
                                           tr("当前文档已修改，是否保存？"),
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                           QMessageBox::Yes);
    if (ret == QMessageBox::Cancel) {
        return false;
    }
    if (ret == QMessageBox::Yes) {
        saveFile();
        return !m_sourceEditor->document()->isModified();
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeWarnOnUnsavedChanges()) {
        event->accept();
    } else {
        event->ignore();
    }
}
