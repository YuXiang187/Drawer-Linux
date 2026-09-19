#pragma once

#include <QStringList>

class RandomEngine;

class NamePool
{
public:
    NamePool();
    ~NamePool();

    void setNames(const QStringList &names);
    void addName(const QString &name);
    void removeName(const QString &name);

    QString draw();
    void reset();

    int size() const;
    int remainingSize() const;
    bool isEmpty() const;

    QStringList names() const;
    QStringList remainingNames() const;
    bool hasName(const QString &name) const;

private:
    QStringList m_initPool;
    QStringList m_pool;
    RandomEngine *m_random;
};
