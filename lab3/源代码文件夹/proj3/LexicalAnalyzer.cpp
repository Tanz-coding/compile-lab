#include "LexicalAnalyzer.h"

#include <QObject>

namespace
{
QString tokenTypeName(LexicalAnalyzer::TokenType type)
{
    using TokenType = LexicalAnalyzer::TokenType;
    switch (type) {
    case TokenType::Identifier:
        return QStringLiteral("标识符");
    case TokenType::Number:
        return QStringLiteral("整型常量");
    case TokenType::KeywordIf:
        return QStringLiteral("关键字(if)");
    case TokenType::KeywordElse:
        return QStringLiteral("关键字(else)");
    case TokenType::KeywordRepeat:
        return QStringLiteral("关键字(repeat)");
    case TokenType::KeywordUntil:
        return QStringLiteral("关键字(until)");
    case TokenType::KeywordRead:
        return QStringLiteral("关键字(read)");
    case TokenType::KeywordWrite:
        return QStringLiteral("关键字(write)");
    case TokenType::KeywordFor:
        return QStringLiteral("关键字(for)");
    case TokenType::KeywordEnd:
        return QStringLiteral("关键字(end)");
    case TokenType::Assign:
        return QStringLiteral("赋值运算符(:=)");
    case TokenType::RegexAssign:
        return QStringLiteral("正则赋值(::=)");
    case TokenType::Increment:
        return QStringLiteral("自增运算符(++)");
    case TokenType::Decrement:
        return QStringLiteral("自减运算符(--)");
    case TokenType::Plus:
        return QStringLiteral("加号(+)");
    case TokenType::Minus:
        return QStringLiteral("减号(-)");
    case TokenType::Multiply:
        return QStringLiteral("乘号(*)");
    case TokenType::Divide:
        return QStringLiteral("除号(/)");
    case TokenType::Modulo:
        return QStringLiteral("取余(%)");
    case TokenType::Power:
        return QStringLiteral("乘方(^)");
    case TokenType::Less:
        return QStringLiteral("比较运算符(<)");
    case TokenType::LessEqual:
        return QStringLiteral("比较运算符(<=)");
    case TokenType::Greater:
        return QStringLiteral("比较运算符(>)");
    case TokenType::GreaterEqual:
        return QStringLiteral("比较运算符(>=)");
    case TokenType::Equal:
        return QStringLiteral("比较运算符(=)");
    case TokenType::NotEqual:
        return QStringLiteral("比较运算符(<>)");
    case TokenType::Semicolon:
        return QStringLiteral("分号 ';'");
    case TokenType::LeftParen:
        return QStringLiteral("左括号 '('");
    case TokenType::RightParen:
        return QStringLiteral("右括号 ')'");
    case TokenType::Pipe:
        return QStringLiteral("正则或(|)");
    case TokenType::Ampersand:
        return QStringLiteral("正则连接(&)");
    case TokenType::Hash:
        return QStringLiteral("正则闭包(#)");
    case TokenType::Question:
        return QStringLiteral("正则可选(?)");
    case TokenType::Comment:
        return QStringLiteral("注释");
    case TokenType::EndOfFile:
        return QStringLiteral("文件结束");
    case TokenType::Unknown:
    default:
        return QStringLiteral("未知");
    }
}
}

LexicalAnalyzer::LexicalAnalyzer()
    : m_index(0)
    , m_line(1)
    , m_column(1)
{
    m_keywords.insert(QStringLiteral("if"), TokenType::KeywordIf);
    m_keywords.insert(QStringLiteral("else"), TokenType::KeywordElse);
    m_keywords.insert(QStringLiteral("repeat"), TokenType::KeywordRepeat);
    m_keywords.insert(QStringLiteral("until"), TokenType::KeywordUntil);
    m_keywords.insert(QStringLiteral("read"), TokenType::KeywordRead);
    m_keywords.insert(QStringLiteral("write"), TokenType::KeywordWrite);
    m_keywords.insert(QStringLiteral("for"), TokenType::KeywordFor);
    m_keywords.insert(QStringLiteral("end"), TokenType::KeywordEnd);
}

QVector<LexicalAnalyzer::Token> LexicalAnalyzer::analyze(const QString &source)
{
    reset(source);

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) {
            break;
        }

        const QChar ch = currentChar();
        if (ch == QLatin1Char('{')) {
            skipComment();
            continue;
        }

        if (ch.isLetter() || ch == QLatin1Char('_')) {
            scanIdentifier();
        } else if (ch.isDigit()) {
            scanNumber();
        } else {
            scanOperator();
        }
    }

    addToken(TokenType::EndOfFile, QStringLiteral("EOF"), m_line, m_column);
    return m_tokens;
}

const QVector<LexicalAnalyzer::AnalysisError> &LexicalAnalyzer::errors() const
{
    return m_errors;
}

QString LexicalAnalyzer::tokenTypeToString(TokenType type)
{
    return tokenTypeName(type);
}

void LexicalAnalyzer::reset(const QString &source)
{
    m_source = source;
    m_index = 0;
    m_line = 1;
    m_column = 1;
    m_tokens.clear();
    m_errors.clear();
}

void LexicalAnalyzer::skipWhitespace()
{
    while (!isAtEnd()) {
        const QChar ch = currentChar();
        if (ch == QLatin1Char(' ') || ch == QLatin1Char('\t') || ch == QLatin1Char('\f')) {
            advance();
            continue;
        }
        if (ch == QLatin1Char('\n') || ch == QLatin1Char('\r')) {
            advance();
            continue;
        }
        break;
    }
}

void LexicalAnalyzer::skipComment()
{
    const int commentLine = m_line;
    const int commentColumn = m_column;
    advance(); // consume '{'
    while (!isAtEnd() && currentChar() != QLatin1Char('}')) {
        advance();
    }
    if (isAtEnd()) {
        addError(QStringLiteral("注释未闭合"), commentLine, commentColumn);
        return;
    }
    advance(); // consume '}'
}

void LexicalAnalyzer::scanIdentifier()
{
    const int startLine = m_line;
    const int startColumn = m_column;
    QString lexeme;
    while (!isAtEnd() && (currentChar().isLetterOrNumber() || currentChar() == QLatin1Char('_'))) {
        lexeme.append(advance());
    }

    const QString lowered = lexeme.toLower();
    const TokenType type = m_keywords.value(lowered, TokenType::Identifier);
    addToken(type, lexeme, startLine, startColumn);
}

void LexicalAnalyzer::scanNumber()
{
    const int startLine = m_line;
    const int startColumn = m_column;
    QString lexeme;
    while (!isAtEnd() && currentChar().isDigit()) {
        lexeme.append(advance());
    }
    addToken(TokenType::Number, lexeme, startLine, startColumn);
}

void LexicalAnalyzer::scanOperator()
{
    const int startLine = m_line;
    const int startColumn = m_column;
    const QChar ch = currentChar();

    auto consumeSingle = [this](TokenType type, const QString &lexeme, int line, int column) {
        advance();
        addToken(type, lexeme, line, column);
    };

    switch (ch.unicode()) {
    case '+':
        if (!isAtEnd(1) && currentChar(1) == QLatin1Char('+')) {
            advance();
            advance();
            addToken(TokenType::Increment, QStringLiteral("++"), startLine, startColumn);
        } else {
            consumeSingle(TokenType::Plus, QStringLiteral("+"), startLine, startColumn);
        }
        break;
    case '-':
        if (!isAtEnd(1) && currentChar(1) == QLatin1Char('-')) {
            advance();
            advance();
            addToken(TokenType::Decrement, QStringLiteral("--"), startLine, startColumn);
        } else {
            consumeSingle(TokenType::Minus, QStringLiteral("-"), startLine, startColumn);
        }
        break;
    case '*':
        consumeSingle(TokenType::Multiply, QStringLiteral("*"), startLine, startColumn);
        break;
    case '/':
        consumeSingle(TokenType::Divide, QStringLiteral("/"), startLine, startColumn);
        break;
    case '%':
        consumeSingle(TokenType::Modulo, QStringLiteral("%"), startLine, startColumn);
        break;
    case '^':
        consumeSingle(TokenType::Power, QStringLiteral("^"), startLine, startColumn);
        break;
    case ';':
        consumeSingle(TokenType::Semicolon, QStringLiteral(";"), startLine, startColumn);
        break;
    case '(':
        consumeSingle(TokenType::LeftParen, QStringLiteral("("), startLine, startColumn);
        break;
    case ')':
        consumeSingle(TokenType::RightParen, QStringLiteral(")"), startLine, startColumn);
        break;
    case '|':
        consumeSingle(TokenType::Pipe, QStringLiteral("|"), startLine, startColumn);
        break;
    case '&':
        consumeSingle(TokenType::Ampersand, QStringLiteral("&"), startLine, startColumn);
        break;
    case '#':
        consumeSingle(TokenType::Hash, QStringLiteral("#"), startLine, startColumn);
        break;
    case '?':
        consumeSingle(TokenType::Question, QStringLiteral("?"), startLine, startColumn);
        break;
    case ':':
        if (!isAtEnd(1) && currentChar(1) == QLatin1Char(':') && !isAtEnd(2) && currentChar(2) == QLatin1Char('=')) {
            advance();
            advance();
            advance();
            addToken(TokenType::RegexAssign, QStringLiteral("::="), startLine, startColumn);
        } else if (!isAtEnd(1) && currentChar(1) == QLatin1Char('=')) {
            advance();
            advance();
            addToken(TokenType::Assign, QStringLiteral(":="), startLine, startColumn);
        } else {
            advance();
            addError(QStringLiteral("无法识别的符号 ':'"), startLine, startColumn);
        }
        break;
    case '<':
        if (!isAtEnd(1) && currentChar(1) == QLatin1Char('=')) {
            advance();
            advance();
            addToken(TokenType::LessEqual, QStringLiteral("<="), startLine, startColumn);
        } else if (!isAtEnd(1) && currentChar(1) == QLatin1Char('>')) {
            advance();
            advance();
            addToken(TokenType::NotEqual, QStringLiteral("<>"), startLine, startColumn);
        } else {
            consumeSingle(TokenType::Less, QStringLiteral("<"), startLine, startColumn);
        }
        break;
    case '>':
        if (!isAtEnd(1) && currentChar(1) == QLatin1Char('=')) {
            advance();
            advance();
            addToken(TokenType::GreaterEqual, QStringLiteral(">="), startLine, startColumn);
        } else {
            consumeSingle(TokenType::Greater, QStringLiteral(">"), startLine, startColumn);
        }
        break;
    case '=':
        consumeSingle(TokenType::Equal, QStringLiteral("="), startLine, startColumn);
        break;
    default:
        advance();
        addError(QObject::tr("无法识别的符号 '%1'").arg(ch), startLine, startColumn);
        break;
    }
}

void LexicalAnalyzer::addToken(TokenType type, const QString &lexeme, int line, int column)
{
    m_tokens.push_back(Token{type, lexeme, line, column});
}

void LexicalAnalyzer::addError(const QString &message, int line, int column)
{
    m_errors.push_back(AnalysisError{message, line, column});
}

QChar LexicalAnalyzer::currentChar(int lookahead) const
{
    if (isAtEnd(lookahead)) {
        return QChar();
    }
    return m_source.at(m_index + lookahead);
}

bool LexicalAnalyzer::isAtEnd(int lookahead) const
{
    return (m_index + lookahead) >= m_source.size();
}

QChar LexicalAnalyzer::advance()
{
    if (isAtEnd()) {
        return QChar();
    }
    const QChar ch = m_source.at(m_index++);
    if (ch == QLatin1Char('\r')) {
        if (!isAtEnd() && currentChar() == QLatin1Char('\n')) {
            ++m_index;
        }
        nextLine();
    } else if (ch == QLatin1Char('\n')) {
        nextLine();
    } else {
        ++m_column;
    }
    return ch;
}

void LexicalAnalyzer::nextLine()
{
    ++m_line;
    m_column = 1;
}
