#include "SyntaxAnalyzer.h"

#include <QObject>
#include <QtGlobal>

SyntaxAnalyzer::SyntaxAnalyzer(const QVector<Token> &tokens)
    : m_tokens(tokens)
    , m_index(0)
    , m_buildTree(true)
{
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::analyze(bool buildTree)
{
    m_buildTree = buildTree;
    m_index = 0;
    m_errors.clear();

    auto root = parseProgram();
    if (!isAtEnd()) {
        const auto &token = currentToken();
        reportError(QObject::tr("多余的符号: %1").arg(token.lexeme), token);
    }
    return root;
}

const QVector<SyntaxAnalyzer::AnalysisError> &SyntaxAnalyzer::errors() const
{
    return m_errors;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseProgram()
{
    auto programNode = makeNode(QStringLiteral("program"));
    auto sequence = parseStatementSequence({TokenType::EndOfFile});
    if (programNode && sequence) {
        programNode->addChild(sequence);
    }
    return programNode;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseStatementSequence(const QVector<TokenType> &terminators)
{
    auto sequenceNode = makeNode(QStringLiteral("stmt-sequence"));
    bool hasStatement = false;
    while (!isAtEnd() && !isTerminator(currentType(), terminators)) {
        if (!canStartStatement(currentType())) {
            reportError(QObject::tr("期望语句"), currentToken());
            synchronize(terminators);
            break;
        }
        auto statement = parseStatement();
        if (sequenceNode && statement) {
            sequenceNode->addChild(statement);
        }
        hasStatement = true;

        while (match(TokenType::Semicolon)) {
            // 允许连续分号，继续尝试解析下一条语句。
        }
    }

    if (!hasStatement) {
        reportError(QObject::tr("语句序列不能为空"), currentToken());
    }

    return sequenceNode;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseStatement()
{
    switch (currentType()) {
    case TokenType::KeywordIf:
        return parseIfStatement();
    case TokenType::KeywordRepeat:
        return parseRepeatStatement();
    case TokenType::KeywordRead:
        return parseReadStatement();
    case TokenType::KeywordWrite:
        return parseWriteStatement();
    case TokenType::KeywordFor:
        return parseForStatement();
    case TokenType::Increment:
        return parseIncrementStatement(true);
    case TokenType::Decrement:
        return parseIncrementStatement(false);
    case TokenType::Identifier:
        if (peekType(1) == TokenType::RegexAssign) {
            return parseRegexAssignStatement();
        }
        return parseAssignStatement();
    default:
        reportError(QObject::tr("无法识别的语句"), currentToken());
        advance();
        return makeNode(QStringLiteral("error"));
    }
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseIfStatement()
{
    auto node = makeNode(QStringLiteral("if_stmt"));
    advance(); // consume 'if'

    consume(TokenType::LeftParen, QObject::tr("if语句缺少'('"));
    auto condition = parseExpression();
    consume(TokenType::RightParen, QObject::tr("if语句缺少')'"));

    auto thenSequence = parseStatementSequence(QVector<TokenType>{TokenType::KeywordElse, TokenType::KeywordEnd, TokenType::EndOfFile});
    std::shared_ptr<SyntaxTreeNode> elseSequence;
    if (match(TokenType::KeywordElse)) {
    elseSequence = parseStatementSequence(QVector<TokenType>{TokenType::KeywordEnd, TokenType::EndOfFile});
    }
    match(TokenType::KeywordEnd); // 允许保留end作为可选结束符

    if (node && condition) {
        if (auto wrapper = makeNode(QStringLiteral("condition"))) {
            wrapper->addChild(condition);
            node->addChild(wrapper);
        }
    }
    if (node && thenSequence) {
        if (auto wrapper = makeNode(QStringLiteral("then"))) {
            wrapper->addChild(thenSequence);
            node->addChild(wrapper);
        }
    }
    if (node && elseSequence) {
        if (auto wrapper = makeNode(QStringLiteral("else"))) {
            wrapper->addChild(elseSequence);
            node->addChild(wrapper);
        }
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseRepeatStatement()
{
    auto node = makeNode(QStringLiteral("repeat_stmt"));
    advance(); // consume 'repeat'
    auto body = parseStatementSequence(QVector<TokenType>{TokenType::KeywordUntil});
    consume(TokenType::KeywordUntil, QObject::tr("repeat语句缺少until"));
    auto condition = parseExpression();

    if (node && body) {
        if (auto wrapper = makeNode(QStringLiteral("body"))) {
            wrapper->addChild(body);
            node->addChild(wrapper);
        }
    }
    if (node && condition) {
        if (auto wrapper = makeNode(QStringLiteral("condition"))) {
            wrapper->addChild(condition);
            node->addChild(wrapper);
        }
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseReadStatement()
{
    auto node = makeNode(QStringLiteral("read_stmt"));
    advance(); // consume 'read'
    const Token identifier = consume(TokenType::Identifier, QObject::tr("read语句缺少标识符"));
    if (node) {
        node->addChild(makeLeaf(QStringLiteral("identifier"), identifier));
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseWriteStatement()
{
    auto node = makeNode(QStringLiteral("write_stmt"));
    advance(); // consume 'write'
    auto expression = parseExpression();
    if (node && expression) {
        node->addChild(expression);
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseAssignStatement()
{
    const Token identifier = consume(TokenType::Identifier, QObject::tr("赋值语句缺少标识符"));
    consume(TokenType::Assign, QObject::tr("赋值语句缺少':='"));
    auto expression = parseExpression();
    auto node = makeNode(QStringLiteral("assign_stmt"));
    if (node) {
        node->addChild(makeLeaf(QStringLiteral("identifier"), identifier));
        if (expression) {
            node->addChild(expression);
        }
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseRegexAssignStatement()
{
    const Token identifier = consume(TokenType::Identifier, QObject::tr("正则表达式赋值缺少标识符"));
    consume(TokenType::RegexAssign, QObject::tr("正则表达式赋值缺少'::='"));
    auto regexExpr = parseRegexExpression();
    auto node = makeNode(QStringLiteral("regex_assign_stmt"));
    if (node) {
        node->addChild(makeLeaf(QStringLiteral("identifier"), identifier));
        if (regexExpr) {
            node->addChild(regexExpr);
        }
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseForStatement()
{
    auto node = makeNode(QStringLiteral("for_stmt"));
    advance(); // consume 'for'
    consume(TokenType::LeftParen, QObject::tr("for语句缺少'('"));

    auto initNode = parseForInitializer();
    consume(TokenType::Semicolon, QObject::tr("for语句缺少第一个分号"));

    auto conditionNode = parseForCondition();
    consume(TokenType::Semicolon, QObject::tr("for语句缺少第二个分号"));

    auto updateNode = parseForUpdate();
    if (currentType() == TokenType::Semicolon) {
        advance();
    }
    consume(TokenType::RightParen, QObject::tr("for语句缺少')'"));

    // 允许for主体在if/else或repeat等结构中直接结束，因此把else、until也作为终止符。
    auto body = parseStatementSequence(QVector<TokenType>{TokenType::KeywordEnd,
                                                         TokenType::KeywordElse,
                                                         TokenType::KeywordUntil,
                                                         TokenType::EndOfFile});
    match(TokenType::KeywordEnd);

    if (node && initNode) {
        if (auto wrapper = makeNode(QStringLiteral("init"))) {
            wrapper->addChild(initNode);
            node->addChild(wrapper);
        }
    }
    if (node && conditionNode) {
        if (auto wrapper = makeNode(QStringLiteral("condition"))) {
            wrapper->addChild(conditionNode);
            node->addChild(wrapper);
        }
    }
    if (node && updateNode) {
        if (auto wrapper = makeNode(QStringLiteral("update"))) {
            wrapper->addChild(updateNode);
            node->addChild(wrapper);
        }
    }
    if (node && body) {
        if (auto wrapper = makeNode(QStringLiteral("body"))) {
            wrapper->addChild(body);
            node->addChild(wrapper);
        }
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseIncrementStatement(bool isIncrement)
{
    const Token opToken = advance();
    const Token identifier = consume(TokenType::Identifier, QObject::tr("自增/自减语句缺少标识符"));

    auto node = makeNode(isIncrement ? QStringLiteral("inc_stmt") : QStringLiteral("dec_stmt"));
    if (node) {
        node->addChild(makeLeaf(QStringLiteral("operator"), opToken));
        node->addChild(makeLeaf(QStringLiteral("identifier"), identifier));
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseForInitializer()
{
    if (currentType() == TokenType::Semicolon) {
        reportError(QObject::tr("for循环缺少初始化语句"), currentToken());
        return makeNode(QStringLiteral("for_init"));
    }
    const Token identifier = consume(TokenType::Identifier, QObject::tr("for初始化缺少标识符"));
    consume(TokenType::Assign, QObject::tr("for初始化缺少':='"));
    auto expression = parseExpression();
    auto node = makeNode(QStringLiteral("for_init"));
    if (node) {
        node->addChild(makeLeaf(QStringLiteral("identifier"), identifier));
        if (expression) {
            node->addChild(expression);
        }
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseForCondition()
{
    auto condition = parseExpression();
    if (!condition) {
        reportError(QObject::tr("for循环条件解析失败"), currentToken());
    }
    return condition;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseForUpdate()
{
    if (currentType() == TokenType::Increment || currentType() == TokenType::Decrement) {
        return parseIncrementStatement(currentType() == TokenType::Increment);
    }
    reportError(QObject::tr("for循环增量部分缺少++或--"), currentToken());
    return makeNode(QStringLiteral("for_update"));
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseExpression()
{
    auto left = parseSimpleExpression();
    if (isComparisonOperator(currentType())) {
        const Token op = advance();
        auto right = parseSimpleExpression();
        auto node = makeNode(QStringLiteral("comparison"), op.lexeme);
        if (node) {
            if (left) {
                node->addChild(left);
            }
            if (right) {
                node->addChild(right);
            }
        }
        return node;
    }
    return left;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseSimpleExpression()
{
    auto node = parseTerm();
    while (currentType() == TokenType::Plus || currentType() == TokenType::Minus) {
        const Token op = advance();
        auto rhs = parseTerm();
        auto parent = makeNode(QStringLiteral("binary_op"), op.lexeme);
        if (parent) {
            parent->addChild(node);
            if (rhs) {
                parent->addChild(rhs);
            }
        }
        node = parent ? parent : node;
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseTerm()
{
    auto node = parsePower();
    while (currentType() == TokenType::Multiply || currentType() == TokenType::Divide || currentType() == TokenType::Modulo) {
        const Token op = advance();
        auto rhs = parsePower();
        auto parent = makeNode(QStringLiteral("binary_op"), op.lexeme);
        if (parent) {
            parent->addChild(node);
            if (rhs) {
                parent->addChild(rhs);
            }
        }
        node = parent ? parent : node;
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parsePower()
{
    auto base = parseUnary();
    if (currentType() == TokenType::Power) {
        const Token op = advance();
        auto exponent = parsePower();
        auto node = makeNode(QStringLiteral("binary_op"), op.lexeme);
        if (node) {
            if (base) {
                node->addChild(base);
            }
            if (exponent) {
                node->addChild(exponent);
            }
        }
        return node;
    }
    return base;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseUnary()
{
    if (currentType() == TokenType::Increment || currentType() == TokenType::Decrement ||
        currentType() == TokenType::Plus || currentType() == TokenType::Minus) {
        const Token op = advance();
        auto operand = parseUnary();
        auto node = makeNode(QStringLiteral("unary_op"), op.lexeme);
        if (node && operand) {
            node->addChild(operand);
        }
        return node;
    }
    return parsePrimary();
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parsePrimary()
{
    if (match(TokenType::LeftParen)) {
        auto expr = parseExpression();
        consume(TokenType::RightParen, QObject::tr("缺少右括号") );
        return expr;
    }
    if (currentType() == TokenType::Identifier || currentType() == TokenType::Number) {
        Token token = advance();
        return makeLeaf(token.type == TokenType::Identifier ? QStringLiteral("identifier") : QStringLiteral("number"), token);
    }
    reportError(QObject::tr("非法的表达式因子"), currentToken());
    advance();
    return makeNode(QStringLiteral("error"));
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseRegexExpression()
{
    auto node = parseRegexConcat();
    while (currentType() == TokenType::Pipe) {
        const Token op = advance();
        auto rhs = parseRegexConcat();
        auto parent = makeNode(QStringLiteral("regex_or"), op.lexeme);
        if (parent) {
            parent->addChild(node);
            if (rhs) {
                parent->addChild(rhs);
            }
        }
        node = parent ? parent : node;
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseRegexConcat()
{
    auto node = parseRegexPostfix();
    while (currentType() == TokenType::Ampersand) {
        const Token op = advance();
        auto rhs = parseRegexPostfix();
        auto parent = makeNode(QStringLiteral("regex_concat"), op.lexeme);
        if (parent) {
            parent->addChild(node);
            if (rhs) {
                parent->addChild(rhs);
            }
        }
        node = parent ? parent : node;
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseRegexPostfix()
{
    auto node = parseRegexPrimary();
    while (currentType() == TokenType::Hash || currentType() == TokenType::Question) {
        const Token op = advance();
        auto parent = makeNode(op.type == TokenType::Hash ? QStringLiteral("regex_closure")
                                                          : QStringLiteral("regex_optional"),
                               op.lexeme);
        if (parent && node) {
            parent->addChild(node);
        }
        node = parent ? parent : node;
    }
    return node;
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::parseRegexPrimary()
{
    if (match(TokenType::LeftParen)) {
        auto inner = parseRegexExpression();
        consume(TokenType::RightParen, QObject::tr("正则表达式缺少右括号"));
        auto groupNode = makeNode(QStringLiteral("regex_group"));
        if (groupNode && inner) {
            groupNode->addChild(inner);
        }
        return groupNode;
    }

    if (currentType() == TokenType::Identifier || currentType() == TokenType::Number) {
        Token token = advance();
        return makeLeaf(QStringLiteral("regex_literal"), token);
    }

    reportError(QObject::tr("非法的正则表达式基本单元"), currentToken());
    advance();
    return makeNode(QStringLiteral("regex_error"));
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::makeNode(const QString &type, const QString &value)
{
    if (!m_buildTree) {
        return nullptr;
    }
    return std::make_shared<SyntaxTreeNode>(type, value);
}

std::shared_ptr<SyntaxTreeNode> SyntaxAnalyzer::makeLeaf(const QString &type, const Token &token)
{
    if (!m_buildTree) {
        return nullptr;
    }
    return std::make_shared<SyntaxTreeNode>(type, token.lexeme);
}

SyntaxAnalyzer::Token SyntaxAnalyzer::advance()
{
    if (isAtEnd()) {
        return currentToken();
    }
    return m_tokens.at(m_index++);
}

const SyntaxAnalyzer::Token &SyntaxAnalyzer::currentToken() const
{
    if (m_tokens.isEmpty()) {
        static const Token dummy{TokenType::EndOfFile, QStringLiteral("EOF"), 0, 0};
        return dummy;
    }
    const int idx = qBound(0, m_index, m_tokens.size() - 1);
    return m_tokens.at(idx);
}

SyntaxAnalyzer::TokenType SyntaxAnalyzer::currentType() const
{
    return currentToken().type;
}

SyntaxAnalyzer::TokenType SyntaxAnalyzer::peekType(int offset) const
{
    const int idx = m_index + offset;
    if (idx < 0 || idx >= m_tokens.size()) {
        return TokenType::EndOfFile;
    }
    return m_tokens.at(idx).type;
}

bool SyntaxAnalyzer::match(TokenType type)
{
    if (currentType() == type) {
        advance();
        return true;
    }
    return false;
}

SyntaxAnalyzer::Token SyntaxAnalyzer::consume(TokenType type, const QString &message)
{
    if (currentType() == type) {
        return advance();
    }
    reportError(message, currentToken());
    Token fallback = currentToken();
    if (!isAtEnd()) {
        advance();
    }
    return fallback;
}

bool SyntaxAnalyzer::check(TokenType type) const
{
    return currentType() == type;
}

bool SyntaxAnalyzer::isAtEnd() const
{
    return currentType() == TokenType::EndOfFile;
}

bool SyntaxAnalyzer::isTerminator(TokenType type, const QVector<TokenType> &terminators) const
{
    return terminators.contains(type);
}

bool SyntaxAnalyzer::canStartStatement(TokenType type) const
{
    switch (type) {
    case TokenType::KeywordIf:
    case TokenType::KeywordRepeat:
    case TokenType::KeywordRead:
    case TokenType::KeywordWrite:
    case TokenType::KeywordFor:
    case TokenType::Increment:
    case TokenType::Decrement:
    case TokenType::Identifier:
        return true;
    default:
        return false;
    }
}

void SyntaxAnalyzer::synchronize(const QVector<TokenType> &terminators)
{
    while (!isAtEnd() && !isTerminator(currentType(), terminators) && currentType() != TokenType::Semicolon) {
        advance();
    }
    if (currentType() == TokenType::Semicolon) {
        advance();
    }
}

bool SyntaxAnalyzer::isComparisonOperator(TokenType type) const
{
    return type == TokenType::Less || type == TokenType::LessEqual || type == TokenType::Greater ||
           type == TokenType::GreaterEqual || type == TokenType::Equal || type == TokenType::NotEqual;
}

void SyntaxAnalyzer::reportError(const QString &message, int line, int column)
{
    m_errors.append(AnalysisError{message, line, column});
}

void SyntaxAnalyzer::reportError(const QString &message, const Token &token)
{
    reportError(message, token.line, token.column);
}
