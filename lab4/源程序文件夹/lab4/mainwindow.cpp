#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include "lr.h"

static QString lr0ItemToString(const Grammar &g, const LR0Item &item)
{
    const Production &p = g.productions[item.prodId];
    QStringList rhs;
    for (int i = 0; i <= p.right.size(); ++i) {
        if (i == item.dotPos) rhs << "·";
        if (i < p.right.size()) rhs << p.right[i];
    }
    return QString("%1 -> %2").arg(p.left, rhs.join(" "));
}

static QString lr1ItemToString(const Grammar &g, const LR1Item &item)
{
    const Production &p = g.productions[item.prodId];
    QStringList rhs;
    for (int i = 0; i <= p.right.size(); ++i) {
        if (i == item.dotPos) rhs << "·";
        if (i < p.right.size()) rhs << p.right[i];
    }
    return QString("[%1 -> %2, %3]").arg(p.left, rhs.join(" "), item.lookahead);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , grammar(new Grammar)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete grammar;
    delete ui;
}

void MainWindow::on_actionOpenGrammar_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文法"), QString(), tr("Text Files (*.txt);;All Files (*.*)"));
    if (fileName.isEmpty()) return;

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件: %1").arg(fileName));
        return;
    }
    QTextStream in(&f);
    QString text = in.readAll();
    f.close();
    ui->grammarEdit->setPlainText(text);
}

void MainWindow::on_actionSaveGrammar_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存文法"), QString(), tr("Text Files (*.txt);;All Files (*.*)"));
    if (fileName.isEmpty()) return;

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法写入文件: %1").arg(fileName));
        return;
    }
    QTextStream out(&f);
    out << ui->grammarEdit->toPlainText();
    f.close();
}

void MainWindow::on_actionComputeFirstFollow_triggered()
{
    QString text = ui->grammarEdit->toPlainText();
    QString error;
    if (!grammar->parseFromText(text, error)) {
        QMessageBox::warning(this, tr("文法错误"), error);
        return;
    }
    grammar->computeFirst();
    grammar->computeFollow();

    // 填充 FIRST 表
    ui->tableFirst->clear();
    QStringList firstHeaders;
    firstHeaders << tr("非终结符") << tr("FIRST 集合");
    ui->tableFirst->setColumnCount(2);
    ui->tableFirst->setHorizontalHeaderLabels(firstHeaders);
    ui->tableFirst->setRowCount(grammar->nonTerminals.size());

    int row = 0;
    for (const QString &nt : grammar->nonTerminals) {
        ui->tableFirst->setItem(row, 0, new QTableWidgetItem(nt));
        QStringList elems = grammar->first[nt].values();
        ui->tableFirst->setItem(row, 1, new QTableWidgetItem(elems.join(", ")));
        ++row;
    }

    // 填充 FOLLOW 表
    ui->tableFollow->clear();
    QStringList followHeaders;
    followHeaders << tr("非终结符") << tr("FOLLOW 集合");
    ui->tableFollow->setColumnCount(2);
    ui->tableFollow->setHorizontalHeaderLabels(followHeaders);
    ui->tableFollow->setRowCount(grammar->nonTerminals.size());

    row = 0;
    for (const QString &nt : grammar->nonTerminals) {
        ui->tableFollow->setItem(row, 0, new QTableWidgetItem(nt));
        QStringList elems = grammar->follow[nt].values();
        ui->tableFollow->setItem(row, 1, new QTableWidgetItem(elems.join(", ")));
        ++row;
    }
}

void MainWindow::on_actionBuildLR0SLR_triggered()
{
    QString text = ui->grammarEdit->toPlainText();
    QString error;
    if (!grammar->parseFromText(text, error)) {
        QMessageBox::warning(this, tr("文法错误"), error);
        return;
    }
    grammar->computeFirst();
    grammar->computeFollow();

    LRAnalyzer analyzer(*grammar);
    analyzer.buildLR0();
    analyzer.buildSLRTable();

    const QVector<LR0State> &states = analyzer.getLR0States();
    const Grammar &augG = analyzer.getAugmentedGrammar();

    // 状态项目集表
    ui->tableLR0States->clear();
    ui->tableLR0States->setColumnCount(2);
    ui->tableLR0States->setHorizontalHeaderLabels(QStringList() << tr("状态") << tr("项目集"));
    ui->tableLR0States->setRowCount(states.size());

    for (int i = 0; i < states.size(); ++i) {
        const LR0State &s = states[i];
        ui->tableLR0States->setItem(i, 0, new QTableWidgetItem(QString::number(s.id)));
        QStringList itemStrs;
        for (const LR0Item &it : s.items) {
            itemStrs << lr0ItemToString(augG, it);
        }
        ui->tableLR0States->setItem(i, 1, new QTableWidgetItem(itemStrs.join("\n")));
    }
    ui->tableLR0States->resizeRowsToContents();

    // LR(0) 转移表（点与边数据）
    ui->tableLR0Trans->clear();
    ui->tableLR0Trans->setColumnCount(3);
    ui->tableLR0Trans->setHorizontalHeaderLabels(QStringList() << tr("From") << tr("Symbol") << tr("To"));

    int edgeRowCount = 0;
    for (const LR0State &s : states) {
        edgeRowCount += s.transitions.size();
    }
    ui->tableLR0Trans->setRowCount(edgeRowCount);

    int r = 0;
    for (const LR0State &s : states) {
        for (auto it = s.transitions.begin(); it != s.transitions.end(); ++it) {
            ui->tableLR0Trans->setItem(r, 0, new QTableWidgetItem(QString::number(s.id)));
            ui->tableLR0Trans->setItem(r, 1, new QTableWidgetItem(it.key()));
            ui->tableLR0Trans->setItem(r, 2, new QTableWidgetItem(QString::number(it.value())));
            ++r;
        }
    }

    // SLR(1) 判断结果
    const QList<ConflictInfo> &conflicts = analyzer.getSLRConflicts();
    if (conflicts.isEmpty()) {
        ui->labelSLRResult->setText(tr("该文法是 SLR(1) 文法"));
        ui->plainTextSLRConflicts->clear();
    } else {
        ui->labelSLRResult->setText(tr("该文法不是 SLR(1) 文法"));
        QStringList lines;
        for (const ConflictInfo &c : conflicts) {
            lines << c.description;
        }
        ui->plainTextSLRConflicts->setPlainText(lines.join("\n"));
    }

    // 构造 SLR 分析表，格式参考示例：状态 / 动作 / 规则 / 输入 / Goto
    const LRTable &slrTable = analyzer.getSLRTable();
    const Grammar &slrG = analyzer.getAugmentedGrammar();

    // 终结符（输入列）
    QSet<QString> termSet = slrG.terminals;
    termSet.insert(slrG.endMarker);
    QStringList termList = termSet.values();
    termList.sort();

    // 非终结符（Goto 列）
    QStringList nonTermList = slrG.nonTerminals.values();
    nonTermList.removeAll(slrG.startSymbol); // 可以按需移除增广开始符
    nonTermList.sort();

    int colCount = 3 + termList.size() + nonTermList.size();
    ui->tableSLR->clear();
    ui->tableSLR->setColumnCount(colCount);

    QStringList headers;
    headers << tr("状态") << tr("动作") << tr("规则");
    for (const QString &t : termList) headers << t;
    for (const QString &nt : nonTermList) headers << nt;
    ui->tableSLR->setHorizontalHeaderLabels(headers);

    ui->tableSLR->setRowCount(states.size());
    for (int i = 0; i < states.size(); ++i) {
        int stateId = states[i].id;
        ui->tableSLR->setItem(i, 0, new QTableWidgetItem(QString::number(stateId)));

        // 统计该状态的动作类型和规约规则（如果有）
        QString actionType;
        QString ruleText;

        auto aRowIt = slrTable.action.find(stateId);
        if (aRowIt != slrTable.action.end()) {
            const auto &rowMap = aRowIt.value();
            for (auto it = rowMap.begin(); it != rowMap.end(); ++it) {
                const ActionEntry &ae = it.value();
                if (ae.type == ActionEntry::Shift) {
                    if (!actionType.contains(tr("移进"))) actionType += tr("移进 ");
                } else if (ae.type == ActionEntry::Reduce) {
                    if (!actionType.contains(tr("归约"))) actionType += tr("归约 ");
                    const Production &p = slrG.productions[ae.target];
                    QString rhs = p.right.isEmpty() ? slrG.epsilon : p.right.join(" ");
                    QString oneRule = QString("%1 → %2").arg(p.left, rhs);
                    if (!ruleText.contains(oneRule)) {
                        if (!ruleText.isEmpty()) ruleText += " ; ";
                        ruleText += oneRule;
                    }
                } else if (ae.type == ActionEntry::Accept) {
                    if (!actionType.contains(tr("接收"))) actionType += tr("接收 ");
                }
            }
        }

        ui->tableSLR->setItem(i, 1, new QTableWidgetItem(actionType.trimmed()));
        ui->tableSLR->setItem(i, 2, new QTableWidgetItem(ruleText));

        // 填输入列
        for (int ti = 0; ti < termList.size(); ++ti) {
            QString t = termList[ti];
            QString cellText;
            if (aRowIt != slrTable.action.end()) {
                auto it = aRowIt.value().find(t);
                if (it != aRowIt.value().end()) {
                    const ActionEntry &ae = it.value();
                    if (ae.type == ActionEntry::Shift) {
                        cellText = QString::number(ae.target);
                    } else if (ae.type == ActionEntry::Reduce) {
                        cellText = tr("r%1").arg(ae.target);
                    } else if (ae.type == ActionEntry::Accept) {
                        cellText = "acc";
                    }
                }
            }
            ui->tableSLR->setItem(i, 3 + ti, new QTableWidgetItem(cellText));
        }

        // 填 Goto 列
        auto gRowIt = slrTable.goTo.find(stateId);
        for (int ni = 0; ni < nonTermList.size(); ++ni) {
            QString nt = nonTermList[ni];
            QString cellText;
            if (gRowIt != slrTable.goTo.end()) {
                auto it = gRowIt.value().find(nt);
                if (it != gRowIt.value().end()) {
                    cellText = QString::number(it.value());
                }
            }
            ui->tableSLR->setItem(i, 3 + termList.size() + ni, new QTableWidgetItem(cellText));
        }
    }
}

void MainWindow::on_actionBuildLR1Table_triggered()
{
    QString text = ui->grammarEdit->toPlainText();
    QString error;
    if (!grammar->parseFromText(text, error)) {
        QMessageBox::warning(this, tr("文法错误"), error);
        return;
    }
    grammar->computeFirst();
    grammar->computeFollow();

    LRAnalyzer analyzer(*grammar);
    analyzer.buildLR1();
    analyzer.buildLR1Table();

    const QVector<LR1State> &states = analyzer.getLR1States();
    const Grammar &augG = analyzer.getAugmentedGrammar();

    // LR(1) 状态项目集
    ui->tableLR1States->clear();
    ui->tableLR1States->setColumnCount(2);
    ui->tableLR1States->setHorizontalHeaderLabels(QStringList() << tr("状态") << tr("项目集"));
    ui->tableLR1States->setRowCount(states.size());

    for (int i = 0; i < states.size(); ++i) {
        const LR1State &s = states[i];
        ui->tableLR1States->setItem(i, 0, new QTableWidgetItem(QString::number(s.id)));
        QStringList itemStrs;
        for (const LR1Item &it : s.items) {
            itemStrs << lr1ItemToString(augG, it);
        }
        ui->tableLR1States->setItem(i, 1, new QTableWidgetItem(itemStrs.join("\n")));
    }
    ui->tableLR1States->resizeRowsToContents();

    // LR(1) 转移表（点与边数据）
    ui->tableLR1Trans->clear();
    ui->tableLR1Trans->setColumnCount(3);
    ui->tableLR1Trans->setHorizontalHeaderLabels(QStringList() << tr("From") << tr("Symbol") << tr("To"));

    int edgeRowCount = 0;
    for (const LR1State &s : states) {
        edgeRowCount += s.transitions.size();
    }
    ui->tableLR1Trans->setRowCount(edgeRowCount);

    int r = 0;
    for (const LR1State &s : states) {
        for (auto it = s.transitions.begin(); it != s.transitions.end(); ++it) {
            ui->tableLR1Trans->setItem(r, 0, new QTableWidgetItem(QString::number(s.id)));
            ui->tableLR1Trans->setItem(r, 1, new QTableWidgetItem(it.key()));
            ui->tableLR1Trans->setItem(r, 2, new QTableWidgetItem(QString::number(it.value())));
            ++r;
        }
    }

    // LR(1) 分析表
    const LRTable &table = analyzer.getLR1ParseTable();

    // 终结符列（含 #）
    QSet<QString> terminals = grammar->terminals;
    terminals.insert(grammar->endMarker);
    QStringList termList = terminals.values();
    termList.sort();

    // 非终结符列
    QStringList nonTermList = grammar->nonTerminals.values();
    nonTermList.sort();

    int colCount = 1 + termList.size() + nonTermList.size();
    ui->tableLR1Parse->clear();
    ui->tableLR1Parse->setColumnCount(colCount);

    QStringList headers;
    headers << tr("状态");
    for (const QString &t : termList) headers << t;
    for (const QString &nt : nonTermList) headers << nt;
    ui->tableLR1Parse->setHorizontalHeaderLabels(headers);

    ui->tableLR1Parse->setRowCount(states.size());
    for (int i = 0; i < states.size(); ++i) {
        int stateId = states[i].id;
        ui->tableLR1Parse->setItem(i, 0, new QTableWidgetItem(QString::number(stateId)));

        // ACTION
        for (int ti = 0; ti < termList.size(); ++ti) {
            QString t = termList[ti];
            QString textCell;
            auto sit = table.action.find(stateId);
            if (sit != table.action.end()) {
                auto ait = sit.value().find(t);
                if (ait != sit.value().end()) {
                    const ActionEntry &ae = ait.value();
                    if (ae.type == ActionEntry::Shift) textCell = QString("s%1").arg(ae.target);
                    else if (ae.type == ActionEntry::Reduce) textCell = QString("r%1").arg(ae.target);
                    else if (ae.type == ActionEntry::Accept) textCell = "acc";
                }
            }
            ui->tableLR1Parse->setItem(i, 1 + ti, new QTableWidgetItem(textCell));
        }

        // GOTO
        for (int ni = 0; ni < nonTermList.size(); ++ni) {
            QString nt = nonTermList[ni];
            QString textCell;
            auto sit = table.goTo.find(stateId);
            if (sit != table.goTo.end()) {
                auto git = sit.value().find(nt);
                if (git != sit.value().end()) {
                    textCell = QString::number(git.value());
                }
            }
            ui->tableLR1Parse->setItem(i, 1 + termList.size() + ni, new QTableWidgetItem(textCell));
        }
    }
}

void MainWindow::on_actionAnalyzeSentence_triggered()
{
    // 留空：可选做，基于 LR(1) 表模拟分析过程并填充 tableSteps
    QMessageBox::information(this, tr("提示"), tr("句子分析功能为选做，可按需要自行补充。"));
}
