#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include <QVector>
#include <QString>

#include "LexicalAnalyzer.h"
#include "SyntaxTreeNode.h"

class SyntaxAnalyzer
{
public:
    using Token = LexicalAnalyzer::Token;
    using TokenType = LexicalAnalyzer::TokenType;
    using AnalysisError = LexicalAnalyzer::AnalysisError;

    explicit SyntaxAnalyzer(const QVector<Token> &tokens);

    std::shared_ptr<SyntaxTreeNode> analyze(bool buildTree);
    const QVector<AnalysisError> &errors() const;

private:
    std::shared_ptr<SyntaxTreeNode> parseProgram();
    std::shared_ptr<SyntaxTreeNode> parseStatementSequence(const QVector<TokenType> &terminators);
    std::shared_ptr<SyntaxTreeNode> parseStatement();
    std::shared_ptr<SyntaxTreeNode> parseIfStatement();
    std::shared_ptr<SyntaxTreeNode> parseRepeatStatement();
    std::shared_ptr<SyntaxTreeNode> parseReadStatement();
    std::shared_ptr<SyntaxTreeNode> parseWriteStatement();
    std::shared_ptr<SyntaxTreeNode> parseAssignStatement();
    std::shared_ptr<SyntaxTreeNode> parseRegexAssignStatement();
    std::shared_ptr<SyntaxTreeNode> parseForStatement();
    std::shared_ptr<SyntaxTreeNode> parseIncrementStatement(bool isIncrement);

    std::shared_ptr<SyntaxTreeNode> parseForInitializer();
    std::shared_ptr<SyntaxTreeNode> parseForCondition();
    std::shared_ptr<SyntaxTreeNode> parseForUpdate();

    std::shared_ptr<SyntaxTreeNode> parseExpression();
    std::shared_ptr<SyntaxTreeNode> parseSimpleExpression();
    std::shared_ptr<SyntaxTreeNode> parseTerm();
    std::shared_ptr<SyntaxTreeNode> parsePower();
    std::shared_ptr<SyntaxTreeNode> parseUnary();
    std::shared_ptr<SyntaxTreeNode> parsePrimary();

    std::shared_ptr<SyntaxTreeNode> parseRegexExpression();
    std::shared_ptr<SyntaxTreeNode> parseRegexConcat();
    std::shared_ptr<SyntaxTreeNode> parseRegexPostfix();
    std::shared_ptr<SyntaxTreeNode> parseRegexPrimary();

    std::shared_ptr<SyntaxTreeNode> makeNode(const QString &type, const QString &value = QString());
    std::shared_ptr<SyntaxTreeNode> makeLeaf(const QString &type, const Token &token);

    Token advance();
    const Token &currentToken() const;
    TokenType currentType() const;
    TokenType peekType(int offset) const;
    bool match(TokenType type);
    Token consume(TokenType type, const QString &message);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    bool isTerminator(TokenType type, const QVector<TokenType> &terminators) const;
    bool canStartStatement(TokenType type) const;
    void synchronize(const QVector<TokenType> &terminators);
    bool isComparisonOperator(TokenType type) const;

    void reportError(const QString &message, int line, int column);
    void reportError(const QString &message, const Token &token);

    QVector<Token> m_tokens;
    QVector<AnalysisError> m_errors;
    int m_index;
    bool m_buildTree;
};

#endif // SYNTAXANALYZER_H
