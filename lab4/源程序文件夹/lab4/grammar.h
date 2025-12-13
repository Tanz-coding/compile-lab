#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <QString>
#include <QVector>
#include <QSet>
#include <QMap>
#include <QList>

struct Production {
    int id;
    QString left;
    QList<QString> right;
};

struct Grammar {
    QString startSymbol;
    QSet<QString> nonTerminals;
    QSet<QString> terminals;
    QVector<Production> productions;
    QMap<QString, QList<int>> prodsByLeft; // left -> production indices

    QString epsilon = "@";
    QString endMarker = "#";

    bool parseFromText(const QString &text, QString &errorMsg);

    QMap<QString, QSet<QString>> first;
    QMap<QString, QSet<QString>> follow;

    void computeFirst();
    void computeFollow();

    // 提供给 LR(1) 构造使用
    QSet<QString> firstOfSequence(const QList<QString> &seq, bool &canDeriveEpsilon) const;
};

#endif // GRAMMAR_H
