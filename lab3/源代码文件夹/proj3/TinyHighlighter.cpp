#include "TinyHighlighter.h"

#include <QColor>
#include <QFont>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QStringList>

TinyHighlighter::TinyHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
    , m_commentStartExpression(QStringLiteral("\\{"))
    , m_commentEndExpression(QStringLiteral("\\}"))
{
    setupHighlightRules();
}

void TinyHighlighter::setupHighlightRules()
{
    m_highlightRules.clear();

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(QColor(41, 128, 185));
    keywordFormat.setFontWeight(QFont::Bold);

    const QStringList keywords = {
        QStringLiteral("\\bif\\b"), QStringLiteral("\\belse\\b"), QStringLiteral("\\brepeat\\b"),
        QStringLiteral("\\buntil\\b"), QStringLiteral("\\bread\\b"), QStringLiteral("\\bwrite\\b"),
        QStringLiteral("\\bfor\\b")
    };

    for (const QString &pattern : keywords) {
        HighlightRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        m_highlightRules.push_back(rule);
    }

    m_numberFormat.setForeground(QColor(118, 68, 138));
    HighlightRule numberRule;
    numberRule.pattern = QRegularExpression(QStringLiteral("\\b\\d+\\b"));
    numberRule.format = m_numberFormat;
    m_highlightRules.push_back(numberRule);

    m_operatorFormat.setForeground(QColor(192, 57, 43));
    HighlightRule operatorRule;
    operatorRule.pattern = QRegularExpression(QStringLiteral("(::=|:=|\\+\\+|--|<=|>=|<>|[%^&|#\\?])"));
    operatorRule.format = m_operatorFormat;
    m_highlightRules.push_back(operatorRule);

    m_commentFormat.setForeground(QColor(120, 120, 120));
    m_commentFormat.setFontItalic(true);
}

void TinyHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightRule &rule : m_highlightRules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const auto match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(m_commentStartExpression);
    }

    while (startIndex >= 0) {
        auto match = m_commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength;
        if (!match.hasMatch()) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, m_commentFormat);
        if (!match.hasMatch()) {
            break;
        }
        startIndex = text.indexOf(m_commentStartExpression, endIndex + match.capturedLength());
    }
}
