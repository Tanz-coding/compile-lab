#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAbstractItemView>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTextStream>
#include <stdexcept>

namespace {
const QSet<QString> kKeywords = {"if",   "then",  "else",  "end",   "repeat", "until", "read",  "write",
                                  "while", "do",   "enddo", "for"};
const QString kFileFilter = "TINY Source (*.tiny);;All Files (*.*)";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupTable()
{
    ui->quadTable->setColumnCount(5);
    ui->quadTable->setHorizontalHeaderLabels({"#", "Op", "Arg1", "Arg2", "Result"});
    ui->quadTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->quadTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->quadTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::populateTable(const QVector<Quadruple> &quads)
{
    ui->quadTable->clearContents();
    ui->quadTable->setRowCount(quads.size());
    for (int i = 0; i < quads.size(); ++i) {
        const auto &q = quads[i];
        ui->quadTable->setItem(i, 0, new QTableWidgetItem(QString::number(q.index)));
        ui->quadTable->setItem(i, 1, new QTableWidgetItem(q.op));
        ui->quadTable->setItem(i, 2, new QTableWidgetItem(q.arg1));
        ui->quadTable->setItem(i, 3, new QTableWidgetItem(q.arg2));
        ui->quadTable->setItem(i, 4, new QTableWidgetItem(q.result));
    }
}

void MainWindow::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(this, title, message);
}

MainWindow::ParseResult MainWindow::generateIR(const QString &source)
{
    auto tokens = tokenize(source);
    ParseResult result;
    Parser parser(tokens, result.quads);
    parser.parseProgram();
    return result;
}

QVector<MainWindow::Token> MainWindow::tokenize(const QString &source)
{
    QVector<Token> tokens;
    int pos = 0;
    int line = 1;
    int col = 1;
    while (pos < source.size()) {
        skipWhitespace(source, pos, line, col);
        if (pos >= source.size()) {
            break;
        }
        const QChar ch = source.at(pos);
        if (ch.isLetter()) {
            tokens.append(readIdentifier(source, pos, line, col));
        } else if (ch.isDigit()) {
            tokens.append(readNumber(source, pos, line, col));
        } else {
            tokens.append(readOperator(source, pos, line, col));
        }
    }
    tokens.append({TokenKind::End, "", line, col});
    return tokens;
}

void MainWindow::skipWhitespace(const QString &src, int &pos, int &line, int &col)
{
    while (pos < src.size()) {
        const QChar ch = src.at(pos);
        if (ch == '\n') {
            ++pos;
            ++line;
            col = 1;
            continue;
        }
        if (ch.isSpace()) {
            ++pos;
            ++col;
            continue;
        }
        if (ch == '#') {
            // skip comment until end of line
            while (pos < src.size() && src.at(pos) != '\n') {
                ++pos;
                ++col;
            }
            continue;
        }
        break;
    }
}

MainWindow::Token MainWindow::readIdentifier(const QString &src, int &pos, int &line, int &col)
{
    const int startCol = col;
    QString lexeme;
    while (pos < src.size() && (src.at(pos).isLetterOrNumber() || src.at(pos) == '_')) {
        lexeme.append(src.at(pos));
        ++pos;
        ++col;
    }
    const TokenKind kind = kKeywords.contains(lexeme) ? TokenKind::Keyword : TokenKind::Identifier;
    return {kind, lexeme, line, startCol};
}

MainWindow::Token MainWindow::readNumber(const QString &src, int &pos, int &line, int &col)
{
    const int startCol = col;
    QString lexeme;
    while (pos < src.size() && src.at(pos).isDigit()) {
        lexeme.append(src.at(pos));
        ++pos;
        ++col;
    }
    return {TokenKind::Number, lexeme, line, startCol};
}

MainWindow::Token MainWindow::readOperator(const QString &src, int &pos, int &line, int &col)
{
    const int startCol = col;
    auto makeToken = [&](const QString &lex, int advance) {
        pos += advance;
        col += advance;
        return Token{TokenKind::Operator, lex, line, startCol};
    };

    const QStringView remaining(src.constData() + pos, src.size() - pos);
    const QString two = remaining.left(2).toString();
    if (two == "++" || two == "--" || two == "+=" || two == "-=" || two == "<=" || two == ">=" || two == "<>") {
        return makeToken(two, 2);
    }
    if (two == ":=") {
        return makeToken(":=", 2);
    }

    const QChar ch = src.at(pos);
    switch (ch.unicode()) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case '<':
    case '>':
    case '=':
    case '(':
    case ')':
    case ';':
        ++pos;
        ++col;
        return {TokenKind::Operator, QString(ch), line, startCol};
    default:
        throw std::runtime_error(QString("Unknown symbol '%1' at %2:%3")
                                     .arg(ch)
                                     .arg(line)
                                     .arg(col)
                                     .toStdString());
    }
}

// Parser implementation
MainWindow::Parser::Parser(const QVector<Token> &tokensRef, QVector<Quadruple> &quadsRef)
    : tokens(tokensRef)
    , quads(quadsRef)
{
}

void MainWindow::Parser::parseProgram()
{
    stmtSequence();
    if (!isAtEnd()) {
        error("Unexpected tokens after program end", peek());
    }
}

void MainWindow::Parser::stmtSequence(const QSet<QString> &terminators)
{
    statement();
    while (matchLexeme(";")) {
        if (terminators.contains(peek().lexeme) || isAtEnd()) {
            break;
        }
        statement();
    }
}

void MainWindow::Parser::statement()
{
    if (checkLexeme("if")) {
        ifStmt();
    } else if (checkLexeme("repeat")) {
        repeatStmt();
    } else if (checkLexeme("while")) {
        whileStmt();
    } else if (checkLexeme("for")) {
        forStmt();
    } else if (checkLexeme("read")) {
        readStmt();
    } else if (checkLexeme("write")) {
        writeStmt();
    } else if (checkLexeme("++") || checkLexeme("--")) {
        incrementStmt();
    } else if (checkKind(TokenKind::Identifier)) {
        assignStmt();
    } else {
        error("Expected a statement", peek());
    }
}

void MainWindow::Parser::ifStmt()
{
    consumeLexeme("if", "Missing 'if'");
    const QString condition = expression();
    consumeLexeme("then", "Missing 'then' in if statement");
    const int jzIndex = quads.size();
    appendQuad("JZ", condition, "", "?");
    stmtSequence({"else", "end"});

    if (matchLexeme("else")) {
        const int jumpIndex = quads.size();
        appendQuad("JMP", "", "", "?");
        quads[jzIndex].result = QString::number(quads.size() + 1);
        stmtSequence({"end"});
        consumeLexeme("end", "Missing 'end' to close if-else");
        quads[jumpIndex].result = QString::number(quads.size() + 1);
    } else {
        consumeLexeme("end", "Missing 'end' to close if");
        quads[jzIndex].result = QString::number(quads.size() + 1);
    }
}

void MainWindow::Parser::repeatStmt()
{
    consumeLexeme("repeat", "Missing 'repeat'");
    const int startIndex = quads.size() + 1;
    stmtSequence({"until"});
    consumeLexeme("until", "Missing 'until' after repeat body");
    const QString cond = expression();
    appendQuad("JZ", cond, "", QString::number(startIndex));
}

void MainWindow::Parser::whileStmt()
{
    consumeLexeme("while", "Missing 'while'");
    const int startIndex = quads.size() + 1;
    const QString cond = expression();
    consumeLexeme("do", "Missing 'do' in while statement");
    const int jzIndex = quads.size();
    appendQuad("JZ", cond, "", "?");
    stmtSequence({"enddo"});
    consumeLexeme("enddo", "Missing 'enddo' to close while");
    appendQuad("JMP", "", "", QString::number(startIndex));
    quads[jzIndex].result = QString::number(quads.size() + 1);
}

void MainWindow::Parser::forStmt()
{
    consumeLexeme("for", "Missing 'for'");
    consumeLexeme("(", "Missing '(' after for");
    assignStmt();
    consumeLexeme(";", "Missing ';' after for initialization");

    const int conditionIndex = quads.size() + 1;
    const QString cond = expression();
    const int jzIndex = quads.size();
    appendQuad("JZ", cond, "", "?");
    consumeLexeme(";", "Missing ';' after for condition");

    QVector<Quadruple> stepBuffer;
    if (checkLexeme("++") || checkLexeme("--")) {
        incrementStmt(&stepBuffer);
    } else {
        assignStmt(&stepBuffer);
    }
    consumeLexeme(")", "Missing ')' after for step");
    consumeLexeme("do", "Missing 'do' in for statement");

    stmtSequence({"enddo"});
    consumeLexeme("enddo", "Missing 'enddo' to close for");

    for (const auto &q : stepBuffer) {
        appendQuad(q.op, q.arg1, q.arg2, q.result);
    }
    appendQuad("JMP", "", "", QString::number(conditionIndex));
    quads[jzIndex].result = QString::number(quads.size() + 1);
}

void MainWindow::Parser::readStmt()
{
    consumeLexeme("read", "Missing 'read'");
    consumeKind(TokenKind::Identifier, "Expected identifier after 'read'");
    const QString id = previous().lexeme;
    appendQuad("READ", id, "", "");
}

void MainWindow::Parser::writeStmt()
{
    consumeLexeme("write", "Missing 'write'");
    const QString value = expression();
    appendQuad("WRITE", value, "", "");
}

void MainWindow::Parser::assignStmt(QVector<Quadruple> *target)
{
    consumeKind(TokenKind::Identifier, "Expected identifier");
    const QString id = previous().lexeme;
    QString op;
    if (matchLexeme(":=")) {
        op = ":=";
    } else if (matchLexeme("+=")) {
        op = "+=";
    } else if (matchLexeme("-=")) {
        op = "-=";
    } else {
        error("Expected assignment operator (:=, +=, -=)", peek());
    }
    const QString rhs = expression();
    if (op == ":=") {
        appendQuad(":=", rhs, "", id, target);
    } else if (op == "+=") {
        appendQuad("+", id, rhs, id, target);
    } else if (op == "-=") {
        appendQuad("-", id, rhs, id, target);
    }
}

void MainWindow::Parser::incrementStmt(QVector<Quadruple> *target)
{
    Token opTok = advance();
    consumeKind(TokenKind::Identifier, "Expected identifier after ++/--");
    const QString id = previous().lexeme;
    appendQuad(opTok.lexeme, id, "", id, target);
}

QString MainWindow::Parser::expression()
{
    QString left = simpleExp();
    if (matchLexeme("<") || matchLexeme(">") || matchLexeme("<=") || matchLexeme(">=") || matchLexeme("=") || matchLexeme("<>") ) {
        QString op = previous().lexeme;
        QString right = simpleExp();
        QString t = newTemp();
        appendQuad(op, left, right, t);
        left = t;
    }
    return left;
}

QString MainWindow::Parser::simpleExp()
{
    QString left = term();
    while (matchLexeme("+") || matchLexeme("-")) {
        QString op = previous().lexeme;
        QString right = term();
        QString t = newTemp();
        appendQuad(op, left, right, t);
        left = t;
    }
    return left;
}

QString MainWindow::Parser::term()
{
    QString left = power();
    while (matchLexeme("*") || matchLexeme("/") || matchLexeme("%")) {
        QString op = previous().lexeme;
        QString right = power();
        QString t = newTemp();
        appendQuad(op, left, right, t);
        left = t;
    }
    return left;
}

QString MainWindow::Parser::power()
{
    QString left = prefix();
    while (matchLexeme("^")) {
        QString op = previous().lexeme;
        QString right = prefix();
        QString t = newTemp();
        appendQuad(op, left, right, t);
        left = t;
    }
    return left;
}

QString MainWindow::Parser::prefix()
{
    if (matchLexeme("++") || matchLexeme("--")) {
        QString op = previous().lexeme;
        consumeKind(TokenKind::Identifier, "Expected identifier after prefix ++/--");
        QString id = previous().lexeme;
        appendQuad(op, id, "", id);
        return id;
    }
    return factor();
}

QString MainWindow::Parser::factor()
{
    if (matchLexeme("(")) {
        QString value = expression();
        consumeLexeme(")", "Missing ')' after expression");
        return value;
    }
    if (matchKind(TokenKind::Number)) {
        return previous().lexeme;
    }
    if (matchKind(TokenKind::Identifier)) {
        return previous().lexeme;
    }
    error("Expected expression", peek());
    return "";
}

MainWindow::Token MainWindow::Parser::peek() const
{
    return tokens.at(current);
}

MainWindow::Token MainWindow::Parser::previous() const
{
    return tokens.at(current - 1);
}

bool MainWindow::Parser::isAtEnd() const
{
    return peek().kind == TokenKind::End;
}

bool MainWindow::Parser::checkLexeme(const QString &lexeme) const
{
    return !isAtEnd() && peek().lexeme == lexeme;
}

bool MainWindow::Parser::checkKind(TokenKind kind) const
{
    return !isAtEnd() && peek().kind == kind;
}

MainWindow::Token MainWindow::Parser::advance()
{
    if (!isAtEnd()) {
        ++current;
    }
    return previous();
}

bool MainWindow::Parser::matchLexeme(const QString &lexeme)
{
    if (checkLexeme(lexeme)) {
        advance();
        return true;
    }
    return false;
}

bool MainWindow::Parser::matchKind(TokenKind kind)
{
    if (checkKind(kind)) {
        advance();
        return true;
    }
    return false;
}

void MainWindow::Parser::consumeLexeme(const QString &lexeme, const QString &errorMessage)
{
    if (!matchLexeme(lexeme)) {
        error(errorMessage, peek());
    }
}

void MainWindow::Parser::consumeKind(TokenKind kind, const QString &errorMessage)
{
    if (!matchKind(kind)) {
        error(errorMessage, peek());
    }
}

void MainWindow::Parser::error(const QString &msg, const Token &tok)
{
    QString message = QString("%1 at line %2, column %3").arg(msg).arg(tok.line).arg(tok.column);
    throw std::runtime_error(message.toStdString());
}

QString MainWindow::Parser::newTemp()
{
    return QString("t%1").arg(++tempCounter);
}

void MainWindow::Parser::appendQuad(const QString &op, const QString &arg1, const QString &arg2, const QString &result, QVector<Quadruple> *target)
{
    QVector<Quadruple> &sink = target ? *target : quads;
    Quadruple quad{sink.size() + 1, op, arg1, arg2, result};
    sink.append(quad);
}

// UI slots
void MainWindow::on_openButton_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Open TINY Source", QString(), kFileFilter);
    if (fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        showError("Open Failed", QString("Cannot open file: %1").arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    ui->sourceEdit->setPlainText(in.readAll());
}

void MainWindow::on_saveButton_clicked()
{
    const QString fileName = QFileDialog::getSaveFileName(this, "Save TINY Source", QString(), kFileFilter);
    if (fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showError("Save Failed", QString("Cannot save file: %1").arg(file.errorString()));
        return;
    }
    QTextStream out(&file);
    out << ui->sourceEdit->toPlainText();
}

void MainWindow::on_clearButton_clicked()
{
    ui->sourceEdit->clear();
    ui->quadTable->clearContents();
    ui->quadTable->setRowCount(0);
}

void MainWindow::on_generateButton_clicked()
{
    const QString source = ui->sourceEdit->toPlainText();
    try {
        const ParseResult result = generateIR(source);
        populateTable(result.quads);
    } catch (const std::runtime_error &ex) {
        showError("Syntax Error", QString::fromStdString(ex.what()));
    }
}
