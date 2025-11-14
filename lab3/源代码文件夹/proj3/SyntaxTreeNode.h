#ifndef SYNTAXTREENODE_H
#define SYNTAXTREENODE_H

#include <memory>

#include <QVector>
#include <QString>

class SyntaxTreeNode
{
public:
    using Ptr = std::shared_ptr<SyntaxTreeNode>;

    SyntaxTreeNode(QString type, QString value = QString());

    const QString &type() const;
    const QString &value() const;
    const QVector<Ptr> &children() const;

    void addChild(const Ptr &child);

private:
    QString m_type;
    QString m_value;
    QVector<Ptr> m_children;
};

#endif // SYNTAXTREENODE_H
