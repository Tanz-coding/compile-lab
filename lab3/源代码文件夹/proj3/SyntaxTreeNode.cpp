#include "SyntaxTreeNode.h"

#include <utility>

SyntaxTreeNode::SyntaxTreeNode(QString type, QString value)
    : m_type(std::move(type))
    , m_value(std::move(value))
{
}

const QString &SyntaxTreeNode::type() const
{
    return m_type;
}

const QString &SyntaxTreeNode::value() const
{
    return m_value;
}

const QVector<SyntaxTreeNode::Ptr> &SyntaxTreeNode::children() const
{
    return m_children;
}

void SyntaxTreeNode::addChild(const Ptr &child)
{
    if (!child) {
        return;
    }
    m_children.push_back(child);
}
