#include "lr.h"

#include <QQueue>
#include <QObject>

LRAnalyzer::LRAnalyzer(const Grammar &g)
    : grammar(g)
{
}

void LRAnalyzer::buildAugmentedGrammar()
{
    augmentedGrammar = grammar;
    augmentedStartProdId = -1;

    // 若文法已是形如 A' -> A 的增广形式，则直接使用
    if (!grammar.startSymbol.isEmpty()) {
        const QList<int> &list = grammar.prodsByLeft.value(grammar.startSymbol);
        if (list.size() == 1) {
            int pid = list[0];
            const Production &p0 = grammar.productions[pid];
            if (p0.right.size() == 1) {
                const QString &B = p0.right[0];
                if (grammar.nonTerminals.contains(B) && B != grammar.startSymbol && !grammar.prodsByLeft.value(B).isEmpty()) {
                    // 认为 grammar 已经是增广文法：startSymbol -> B
                    augmentedStartProdId = pid;
                    return;
                }
            }
        }
    }

    // 否则自动增广：S' -> S
    if (grammar.startSymbol.isEmpty()) return;
    QString newStart = grammar.startSymbol + "'";
    while (augmentedGrammar.nonTerminals.contains(newStart)) {
        newStart.append("'");
    }
    augmentedGrammar.nonTerminals.insert(newStart);
    Production p;
    p.id = augmentedGrammar.productions.size();
    p.left = newStart;
    p.right = QList<QString>({grammar.startSymbol});
    augmentedStartProdId = p.id;
    augmentedGrammar.productions.append(p);
    augmentedGrammar.prodsByLeft[newStart].append(p.id);
    augmentedGrammar.startSymbol = newStart;
}

QSet<LR0Item> LRAnalyzer::closureLR0(const QSet<LR0Item> &I) const
{
    QSet<LR0Item> result = I;
    bool changed = true;
    while (changed) {
        changed = false;
        QList<LR0Item> items = result.values();
        for (const LR0Item &item : items) {
            const Production &p = augmentedGrammar.productions[item.prodId];
            if (item.dotPos < p.right.size()) {
                QString B = p.right[item.dotPos];
                if (augmentedGrammar.nonTerminals.contains(B)) {
                    const QList<int> &plist = augmentedGrammar.prodsByLeft[B];
                    for (int pid : plist) {
                        LR0Item newItem{pid, 0};
                        if (!result.contains(newItem)) {
                            result.insert(newItem);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    return result;
}

QSet<LR0Item> LRAnalyzer::gotoLR0(const QSet<LR0Item> &I, const QString &X) const
{
    QSet<LR0Item> J;
    for (const LR0Item &item : I) {
        const Production &p = augmentedGrammar.productions[item.prodId];
        if (item.dotPos < p.right.size() && p.right[item.dotPos] == X) {
            LR0Item moved{item.prodId, item.dotPos + 1};
            J.insert(moved);
        }
    }
    if (J.isEmpty()) return J;
    return closureLR0(J);
}

void LRAnalyzer::buildLR0()
{
    buildAugmentedGrammar();
    lr0States.clear();

    // 初始项目集 I0
    QSet<LR0Item> I0;
    I0.insert(LR0Item{augmentedStartProdId, 0});
    I0 = closureLR0(I0);

    QVector<QSet<LR0Item>> C; // states as item sets
    C.append(I0);
    LR0State s0;
    s0.id = 0;
    s0.items = I0;
    lr0States.append(s0);

    QQueue<int> q;
    q.enqueue(0);

    while (!q.isEmpty()) {
        int si = q.dequeue();
        QSet<LR0Item> I = lr0States[si].items;

        QSet<QString> symbols;
        for (const LR0Item &item : I) {
            const Production &p = augmentedGrammar.productions[item.prodId];
            if (item.dotPos < p.right.size()) {
                symbols.insert(p.right[item.dotPos]);
            }
        }

        for (const QString &X : symbols) {
            QSet<LR0Item> J = gotoLR0(I, X);
            if (J.isEmpty()) continue;

            int existing = -1;
            for (int i = 0; i < lr0States.size(); ++i) {
                if (lr0States[i].items == J) {
                    existing = i;
                    break;
                }
            }
            if (existing == -1) {
                LR0State s;
                s.id = lr0States.size();
                s.items = J;
                lr0States.append(s);
                existing = s.id;
                q.enqueue(existing);
            }
            lr0States[si].transitions[X] = existing;
        }
    }
}

void LRAnalyzer::buildSLRTable()
{
    slrTable.action.clear();
    slrTable.goTo.clear();
    slrConflicts.clear();

    // 确保 FOLLOW 已计算
    if (augmentedGrammar.follow.isEmpty()) {
        augmentedGrammar.computeFirst();
        augmentedGrammar.computeFollow();
    }

    // 终结符集合包含 endMarker
    QSet<QString> terminals = augmentedGrammar.terminals;
    terminals.insert(augmentedGrammar.endMarker);

    for (const LR0State &state : lr0States) {
        int i = state.id;
        // 移进
        for (auto it = state.transitions.begin(); it != state.transitions.end(); ++it) {
            QString X = it.key();
            int j = it.value();
            if (augmentedGrammar.terminals.contains(X)) {
                ActionEntry entry;
                entry.type = ActionEntry::Shift;
                entry.target = j;
                ActionEntry &cell = slrTable.action[i][X];
                if (cell.type != ActionEntry::None && !(cell.type == entry.type && cell.target == entry.target)) {
                    ConflictInfo c;
                    c.description = QObject::tr("SLR 冲突: 状态 %1, 符号 %2 发生移进冲突").arg(i).arg(X);
                    slrConflicts.append(c);
                } else {
                    cell = entry;
                }
            } else if (augmentedGrammar.nonTerminals.contains(X)) {
                slrTable.goTo[i][X] = j;
            }
        }

        // 归约和接收
        for (const LR0Item &item : state.items) {
            const Production &p = augmentedGrammar.productions[item.prodId];
            if (item.dotPos == p.right.size()) {
                if (item.prodId == augmentedStartProdId) {
                    // S' -> S.
                    ActionEntry entry;
                    entry.type = ActionEntry::Accept;
                    ActionEntry &cell = slrTable.action[i][augmentedGrammar.endMarker];
                    if (cell.type != ActionEntry::None && cell.type != ActionEntry::Accept) {
                        ConflictInfo c;
                        c.description = QObject::tr("SLR 冲突: 状态 %1 上存在接受/其它动作冲突").arg(i);
                        slrConflicts.append(c);
                    } else {
                        cell = entry;
                    }
                } else {
                    // 对 FOLLOW(A) 中的每个 a，设置 reduce
                    const QString &A = p.left;
                    const QSet<QString> &followA = augmentedGrammar.follow[A];
                    for (const QString &a : followA) {
                        ActionEntry entry;
                        entry.type = ActionEntry::Reduce;
                        entry.target = item.prodId;
                        ActionEntry &cell = slrTable.action[i][a];
                        if (cell.type != ActionEntry::None && !(cell.type == entry.type && cell.target == entry.target)) {
                            ConflictInfo c;
                            c.description = QObject::tr("SLR 冲突: 状态 %1, 符号 %2 上产生归约冲突").arg(i).arg(a);
                            slrConflicts.append(c);
                        } else {
                            cell = entry;
                        }
                    }
                }
            }
        }
    }
}

QSet<LR1Item> LRAnalyzer::closureLR1(const QSet<LR1Item> &I) const
{
    QSet<LR1Item> result = I;
    bool changed = true;
    while (changed) {
        changed = false;
        QList<LR1Item> items = result.values();
        for (const LR1Item &item : items) {
            const Production &p = augmentedGrammar.productions[item.prodId];
            if (item.dotPos < p.right.size()) {
                QString B = p.right[item.dotPos];
                if (augmentedGrammar.nonTerminals.contains(B)) {
                    // 计算 FIRST(beta a)
                    QList<QString> beta;
                    for (int k = item.dotPos + 1; k < p.right.size(); ++k) {
                        beta.append(p.right[k]);
                    }
                    beta.append(item.lookahead);

                    bool nullable = false;
                    QSet<QString> firstSet = augmentedGrammar.firstOfSequence(beta, nullable);

                    const QList<int> &plist = augmentedGrammar.prodsByLeft[B];
                    for (int pid : plist) {
                        for (const QString &b : firstSet) {
                            if (b == augmentedGrammar.epsilon) continue;
                            LR1Item newItem{pid, 0, b};
                            if (!result.contains(newItem)) {
                                result.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

QSet<LR1Item> LRAnalyzer::gotoLR1(const QSet<LR1Item> &I, const QString &X) const
{
    QSet<LR1Item> J;
    for (const LR1Item &item : I) {
        const Production &p = augmentedGrammar.productions[item.prodId];
        if (item.dotPos < p.right.size() && p.right[item.dotPos] == X) {
            LR1Item moved{item.prodId, item.dotPos + 1, item.lookahead};
            J.insert(moved);
        }
    }
    if (J.isEmpty()) return J;
    return closureLR1(J);
}

void LRAnalyzer::buildLR1()
{
    buildAugmentedGrammar();
    lr1States.clear();

    // 需要 FIRST 信息
    if (augmentedGrammar.first.isEmpty()) {
        augmentedGrammar.computeFirst();
        augmentedGrammar.computeFollow();
    }

    QSet<LR1Item> I0;
    I0.insert(LR1Item{augmentedStartProdId, 0, augmentedGrammar.endMarker});
    I0 = closureLR1(I0);

    lr1States.reserve(64);
    LR1State s0;
    s0.id = 0;
    s0.items = I0;
    lr1States.append(s0);

    QQueue<int> q;
    q.enqueue(0);

    while (!q.isEmpty()) {
        int si = q.dequeue();
        QSet<LR1Item> I = lr1States[si].items;

        QSet<QString> symbols;
        for (const LR1Item &item : I) {
            const Production &p = augmentedGrammar.productions[item.prodId];
            if (item.dotPos < p.right.size()) {
                symbols.insert(p.right[item.dotPos]);
            }
        }

        for (const QString &X : symbols) {
            QSet<LR1Item> J = gotoLR1(I, X);
            if (J.isEmpty()) continue;

            int existing = -1;
            for (int i = 0; i < lr1States.size(); ++i) {
                if (lr1States[i].items == J) {
                    existing = i;
                    break;
                }
            }
            if (existing == -1) {
                LR1State s;
                s.id = lr1States.size();
                s.items = J;
                lr1States.append(s);
                existing = s.id;
                q.enqueue(existing);
            }
            lr1States[si].transitions[X] = existing;
        }
    }
}

void LRAnalyzer::buildLR1Table()
{
    lr1Table.action.clear();
    lr1Table.goTo.clear();
    lr1Conflicts.clear();

    for (const LR1State &state : lr1States) {
        int i = state.id;
        // 移进和 goto
        for (auto it = state.transitions.begin(); it != state.transitions.end(); ++it) {
            QString X = it.key();
            int j = it.value();
            if (augmentedGrammar.terminals.contains(X)) {
                ActionEntry entry;
                entry.type = ActionEntry::Shift;
                entry.target = j;
                ActionEntry &cell = lr1Table.action[i][X];
                if (cell.type != ActionEntry::None && !(cell.type == entry.type && cell.target == entry.target)) {
                    ConflictInfo c;
                    c.description = QObject::tr("LR(1) 冲突: 状态 %1, 符号 %2 发生移进冲突").arg(i).arg(X);
                    lr1Conflicts.append(c);
                } else {
                    cell = entry;
                }
            } else if (augmentedGrammar.nonTerminals.contains(X)) {
                lr1Table.goTo[i][X] = j;
            }
        }

        // 归约/接收
        for (const LR1Item &item : state.items) {
            const Production &p = augmentedGrammar.productions[item.prodId];
            if (item.dotPos == p.right.size()) {
                if (item.prodId == augmentedStartProdId && item.lookahead == augmentedGrammar.endMarker) {
                    ActionEntry entry;
                    entry.type = ActionEntry::Accept;
                    ActionEntry &cell = lr1Table.action[i][augmentedGrammar.endMarker];
                    if (cell.type != ActionEntry::None && cell.type != ActionEntry::Accept) {
                        ConflictInfo c;
                        c.description = QObject::tr("LR(1) 冲突: 状态 %1 上存在接受/其它动作冲突").arg(i);
                        lr1Conflicts.append(c);
                    } else {
                        cell = entry;
                    }
                } else {
                    ActionEntry entry;
                    entry.type = ActionEntry::Reduce;
                    entry.target = item.prodId;
                    const QString &a = item.lookahead;
                    ActionEntry &cell = lr1Table.action[i][a];
                    if (cell.type != ActionEntry::None && !(cell.type == entry.type && cell.target == entry.target)) {
                        ConflictInfo c;
                        c.description = QObject::tr("LR(1) 冲突: 状态 %1, 符号 %2 上产生归约冲突").arg(i).arg(a);
                        lr1Conflicts.append(c);
                    } else {
                        cell = entry;
                    }
                }
            }
        }
    }
}
