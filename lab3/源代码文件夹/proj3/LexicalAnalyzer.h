#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

#include <QHash>
#include <QVector>
#include <QString>

class LexicalAnalyzer
{
public:
    enum class TokenType {
        Identifier,
        Number,
        KeywordIf,
        KeywordElse,
        KeywordRepeat,
        KeywordUntil,
        KeywordRead,
        KeywordWrite,
        KeywordFor,
    KeywordEnd,
        Assign,
        RegexAssign,
        Increment,
        Decrement,
        Plus,
        Minus,
        Multiply,
        Divide,
        Modulo,
        Power,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
        Equal,
        NotEqual,
        Semicolon,
        LeftParen,
        RightParen,
        Pipe,
        Ampersand,
        Hash,
        Question,
        Comment,
        EndOfFile,
        Unknown
    };

    struct Token {
        TokenType type;
        QString lexeme;
        int line = 0;
        int column = 0;
    };

    struct AnalysisError {
        QString message;
        int line = 0;
        int column = 0;
    };

    LexicalAnalyzer();

    QVector<Token> analyze(const QString &source);
    const QVector<AnalysisError> &errors() const;

    static QString tokenTypeToString(TokenType type);

private:
    void reset(const QString &source);
    void skipWhitespace();
    void skipComment();
    void scanIdentifier();
    void scanNumber();
    void scanOperator();
    void addToken(TokenType type, const QString &lexeme, int line, int column);
    void addError(const QString &message, int line, int column);
    QChar currentChar(int lookahead = 0) const;
    bool isAtEnd(int lookahead = 0) const;
    QChar advance();
    void nextLine();

    QString m_source;
    int m_index;
    int m_line;
    int m_column;
    QVector<Token> m_tokens;
    QVector<AnalysisError> m_errors;
    QHash<QString, TokenType> m_keywords;
};

#endif // LEXICALANALYZER_H
