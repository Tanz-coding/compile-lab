#ifndef LR_H
#define LR_H

#include "grammar.h"
#include <QMap>
#include <QSet>
#include <QVector>
#include <QHash>

struct LR0Item {
    int prodId;
    int dotPos;
    bool operator<(const LR0Item &other) const {
        if (prodId != other.prodId) return prodId < other.prodId;
        return dotPos < other.dotPos;
    }
    bool operator==(const LR0Item &other) const {
        return prodId == other.prodId && dotPos == other.dotPos;
    }
};

struct LR0State {
    int id;
    QSet<LR0Item> items;
    QMap<QString,int> transitions; // symbol -> state id
};

struct LR1Item {
    int prodId;
    int dotPos;
    QString lookahead;
    bool operator<(const LR1Item &other) const {
        if (prodId != other.prodId) return prodId < other.prodId;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookahead < other.lookahead;
    }
    bool operator==(const LR1Item &other) const {
        return prodId == other.prodId && dotPos == other.dotPos && lookahead == other.lookahead;
    }
};

// 为 QSet<Q> 提供哈希支持（Qt6 要求自定义类型有 qHash）
inline uint qHash(const LR0Item &key, uint seed = 0) noexcept
{
    return qHash(qMakePair(key.prodId, key.dotPos), seed);
}

inline uint qHash(const LR1Item &key, uint seed = 0) noexcept
{
    // 简单组合三个字段
    return qHash(qMakePair(qMakePair(key.prodId, key.dotPos), key.lookahead), seed);
}

struct LR1State {
    int id;
    QSet<LR1Item> items;
    QMap<QString,int> transitions;
};

struct ActionEntry {
    enum Type { None, Shift, Reduce, Accept } type = None;
    int target = -1; // 对于 Shift 是状态号；Reduce 是产生式 id
};

struct LRTable {
    QMap<int, QMap<QString, ActionEntry>> action; // state -> terminal -> action
    QMap<int, QMap<QString, int>> goTo;           // state -> nonterminal -> state
};

struct ConflictInfo {
    QString description;
};

class LRAnalyzer
{
public:
    LRAnalyzer(const Grammar &g);

    // LR(0) + SLR(1)
    void buildLR0();
    void buildSLRTable();
    bool isSLR1() const { return slrConflicts.isEmpty(); }
    const QVector<LR0State>& getLR0States() const { return lr0States; }
    const QList<ConflictInfo>& getSLRConflicts() const { return slrConflicts; }
    const LRTable& getSLRTable() const { return slrTable; }

    // LR(1)
    void buildLR1();
    void buildLR1Table();
    const QVector<LR1State>& getLR1States() const { return lr1States; }
    const QList<ConflictInfo>& getLR1Conflicts() const { return lr1Conflicts; }
    const LRTable& getLR1ParseTable() const { return lr1Table; }

    // 提供给界面，用于打印项目集（注意 LR(0)/LR(1) 的产生式编号基于增广文法）
    const Grammar& getAugmentedGrammar() const { return augmentedGrammar; }

private:
    const Grammar &grammar;

    // 增广文法信息
    Grammar augmentedGrammar;
    int augmentedStartProdId = -1;

    // LR(0)
    QVector<LR0State> lr0States;
    LRTable slrTable;
    QList<ConflictInfo> slrConflicts;

    // LR(1)
    QVector<LR1State> lr1States;
    LRTable lr1Table;
    QList<ConflictInfo> lr1Conflicts;

    // 工具函数
    QSet<LR0Item> closureLR0(const QSet<LR0Item> &I) const;
    QSet<LR0Item> gotoLR0(const QSet<LR0Item> &I, const QString &X) const;

    QSet<LR1Item> closureLR1(const QSet<LR1Item> &I) const;
    QSet<LR1Item> gotoLR1(const QSet<LR1Item> &I, const QString &X) const;

    void buildAugmentedGrammar();
};

#endif // LR_H
