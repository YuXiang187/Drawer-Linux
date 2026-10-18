#pragma once

#include "random_engine.h"

#include <QStringList>

// Rebuilds a draw pool so that it holds exactly the names of init: the draw
// history order of the names that are still listed is preserved, while names
// missing from the pool (newly added, or restored from an incomplete config)
// go to the front, where the Gaussian model gives them the highest chance.
QStringList buildSyncedPool(const QStringList &init, const QStringList &history);

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
    QString randomName();

    // Up to maxNames entries spread evenly over the draw pool (or over the
    // name list when the pool is empty), for size estimates. Measuring every
    // entry of a 45000 name list would be far too slow.
    QStringList sample(int maxNames) const;

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
    // Restores the invariant described by buildSyncedPool().
    void syncPool();

    QStringList m_initPool;
    QStringList m_pool;
    RandomEngine m_random;
};
