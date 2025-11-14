#ifndef TINYHIGHLIGHTER_H
#define TINYHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QVector>
#include <QRegularExpression>

class TinyHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit TinyHighlighter(QTextDocument *document);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void setupHighlightRules();

    QVector<HighlightRule> m_highlightRules;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_operatorFormat;
    QRegularExpression m_commentStartExpression;
    QRegularExpression m_commentEndExpression;
};

#endif // TINYHIGHLIGHTER_H
