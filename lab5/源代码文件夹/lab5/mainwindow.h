#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QString>
#include <QVector>

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
    enum class TokenKind {
        Identifier,
        Number,
        Keyword,
        Operator,
        Separator,
        End
    };

    struct Token {
        TokenKind kind;
        QString lexeme;
        int line;
        int column;
    };

    struct Quadruple {
        int index;
        QString op;
        QString arg1;
        QString arg2;
        QString result;
    };

    struct ParseResult {
        QVector<Quadruple> quads;
    };

    // UI helpers
    void setupTable();
    void populateTable(const QVector<Quadruple> &quads);
    void showError(const QString &title, const QString &message);

    // Front-end pipeline
    ParseResult generateIR(const QString &source);
    QVector<Token> tokenize(const QString &source);
    void skipWhitespace(const QString &src, int &pos, int &line, int &col);
    Token readIdentifier(const QString &src, int &pos, int &line, int &col);
    Token readNumber(const QString &src, int &pos, int &line, int &col);
    Token readOperator(const QString &src, int &pos, int &line, int &col);

    // Recursive descent parser
    class Parser {
    public:
        Parser(const QVector<Token> &tokens, QVector<Quadruple> &quads);
        void parseProgram();

    private:
        const QVector<Token> &tokens;
        QVector<Quadruple> &quads;
        int tempCounter = 0;
        int current = 0;

        // Entry points
        void stmtSequence(const QSet<QString> &terminators = {});
        void statement();
        void ifStmt();
        void repeatStmt();
        void whileStmt();
        void forStmt();
        void readStmt();
        void writeStmt();
        void assignStmt(QVector<Quadruple> *target = nullptr);
        void incrementStmt(QVector<Quadruple> *target = nullptr);

        // Expressions
        QString expression();
        QString simpleExp();
        QString term();
        QString power();
        QString prefix();
        QString factor();

        // Utilities
        Token peek() const;
        Token previous() const;
        bool isAtEnd() const;
        bool checkLexeme(const QString &lexeme) const;
        bool checkKind(TokenKind kind) const;
        Token advance();
        bool matchLexeme(const QString &lexeme);
        bool matchKind(TokenKind kind);
        void consumeLexeme(const QString &lexeme, const QString &errorMessage);
        void consumeKind(TokenKind kind, const QString &errorMessage);
        void expectSemicolonBoundary(const QSet<QString> &terminators);
        void error(const QString &msg, const Token &tok);
        QString newTemp();
        void appendQuad(const QString &op, const QString &arg1, const QString &arg2, const QString &result, QVector<Quadruple> *target = nullptr);
    };

private slots:
    void on_openButton_clicked();
    void on_saveButton_clicked();
    void on_clearButton_clicked();
    void on_generateButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
