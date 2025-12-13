#include "grammar.h"

#include <QStringList>
#include <QObject>

bool Grammar::parseFromText(const QString &text, QString &errorMsg)
{
    productions.clear();
    nonTerminals.clear();
    terminals.clear();
    prodsByLeft.clear();
    first.clear();
    follow.clear();
    startSymbol.clear();

    QStringList lines = text.split('\n');
    int idCounter = 0;
    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split("->");
        if (parts.size() != 2) {
            errorMsg = QObject::tr("文法行格式错误: %1").arg(line);
            return false;
        }
        QString left = parts[0].trimmed();
        if (left.isEmpty()) {
            errorMsg = QObject::tr("左部为空: %1").arg(line);
            return false;
        }
        if (startSymbol.isEmpty()) startSymbol = left;
        nonTerminals.insert(left);

        QString rightPart = parts[1];
        QStringList alts = rightPart.split('|');
        for (QString alt : alts) {
            alt = alt.trimmed();
            QList<QString> symbols;
            if (alt == epsilon) {
                // epsilon 产生式，right 为空列表表示 @
            } else {
                // 自定义分词：字母/数字连续串为一个符号，其它单字符
                QString token;
                auto flushToken = [&]() {
                    if (!token.isEmpty()) {
                        symbols.append(token);
                        token.clear();
                    }
                };
                for (QChar ch : alt) {
                    if (ch.isSpace()) {
                        flushToken();
                    } else if (ch.isLetterOrNumber() || ch == '_') {
                        token.append(ch);
                    } else {
                        flushToken();
                        // 括号、运算符等单独成符号，例如 '(', ')', '+', '*', '/' 等
                        symbols.append(QString(ch));
                    }
                }
                flushToken();
            }
            Production p;
            p.id = idCounter++;
            p.left = left;
            p.right = symbols;
            productions.append(p);
            prodsByLeft[left].append(p.id);
        }
    }

    // 统计终结符：出现在右部但不在 nonTerminals 中且不是 epsilon
    for (const Production &p : productions) {
        for (const QString &sym : p.right) {
            if (sym == epsilon) continue;
            if (!nonTerminals.contains(sym)) {
                terminals.insert(sym);
            }
        }
    }

    return true;
}

void Grammar::computeFirst()
{
    first.clear();
    // 初始化非终结符 FIRST 集为空
    for (const QString &nt : nonTerminals) {
        first[nt] = QSet<QString>();
    }
    // 终结符的 FIRST 是其自身
    for (const QString &t : terminals) {
        first[t] = QSet<QString>({t});
    }
    first[epsilon] = QSet<QString>({epsilon});

    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production &p : productions) {
            QSet<QString> &firstA = first[p.left];
            if (p.right.isEmpty()) {
                if (!firstA.contains(epsilon)) {
                    firstA.insert(epsilon);
                    changed = true;
                }
                continue;
            }
            bool allNullable = true;
            for (const QString &X : p.right) {
                QSet<QString> &firstX = first[X];
                for (const QString &a : firstX) {
                    if (a == epsilon) continue;
                    if (!firstA.contains(a)) {
                        firstA.insert(a);
                        changed = true;
                    }
                }
                if (!firstX.contains(epsilon)) {
                    allNullable = false;
                    break;
                }
            }
            if (allNullable) {
                if (!firstA.contains(epsilon)) {
                    firstA.insert(epsilon);
                    changed = true;
                }
            }
        }
    }
}

void Grammar::computeFollow()
{
    follow.clear();
    for (const QString &nt : nonTerminals) {
        follow[nt] = QSet<QString>();
    }
    // 开始符号加入结束符
    if (!startSymbol.isEmpty()) {
        follow[startSymbol].insert(endMarker);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production &p : productions) {
            for (int i = 0; i < p.right.size(); ++i) {
                QString B = p.right[i];
                if (!nonTerminals.contains(B)) continue;

                // 计算 FIRST(beta)
                QList<QString> beta;
                for (int j = i + 1; j < p.right.size(); ++j) {
                    beta.append(p.right[j]);
                }

                bool betaNullable = false;
                QSet<QString> firstBeta = firstOfSequence(beta, betaNullable);

                // 将 FIRST(beta)\{@} 加入 FOLLOW(B)
                for (const QString &a : firstBeta) {
                    if (a == epsilon) continue;
                    if (!follow[B].contains(a)) {
                        follow[B].insert(a);
                        changed = true;
                    }
                }

                // 如果 beta 可推导空，将 FOLLOW(A) 加入 FOLLOW(B)
                if (betaNullable || beta.isEmpty()) {
                    for (const QString &b : follow[p.left]) {
                        if (!follow[B].contains(b)) {
                            follow[B].insert(b);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

QSet<QString> Grammar::firstOfSequence(const QList<QString> &seq, bool &canDeriveEpsilon) const
{
    QSet<QString> result;
    canDeriveEpsilon = true;
    if (seq.isEmpty()) {
        result.insert(epsilon);
        return result;
    }

    for (const QString &X : seq) {
        auto it = first.find(X);
        QSet<QString> setX;
        if (it != first.end()) {
            setX = it.value();
        } else {
            // 若是终结符但未记录，FIRST(X)=X
            setX.insert(X);
        }

        bool nullableX = false;
        for (const QString &a : setX) {
            if (a == epsilon) {
                nullableX = true;
            } else {
                result.insert(a);
            }
        }
        if (!nullableX) {
            canDeriveEpsilon = false;
            break;
        }
    }

    if (canDeriveEpsilon) result.insert(epsilon);
    return result;
}
