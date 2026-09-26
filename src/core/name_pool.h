#pragma once

#include "random_engine.h"

#include <QStringList>

class NamePool
{
public:
    NamePool();
    ~NamePool();

    void setNames(const QStringList &names);
    void setState(const QStringList &initPool, const QStringList &pool);
    void addName(const QString &name);
    void removeName(const QString &name);

    QString draw();
    void reset();

    int size() const;
    bool isEmpty() const;

    // The original names in user-edited order. This list never takes part in
    // the Gaussian ordering and is not modified by drawing.
    QStringList names() const;

    // The Gaussian draw pool: always holds every name of the init pool,
    // ordered by draw history (least recently drawn first, most recently
    // drawn last). This is the list persisted between runs.
    QStringList pool() const;

    bool hasName(const QString &name) const;

private:
    // Restores the invariant that m_pool contains exactly the names of
    // m_initPool, preserving the draw history order of the names that are
    // already present. Names missing from the pool (newly added, or restored
    // from an incomplete config) go to the front, where the Gaussian model
    // gives them the highest chance.
    void syncPool();

    QStringList m_initPool;
    QStringList m_pool;
    RandomEngine m_random;
};
