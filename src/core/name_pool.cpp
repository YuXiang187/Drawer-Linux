#include "name_pool.h"
#include "random_engine.h"

#include <QHash>

#include <algorithm>
#include <cmath>

namespace {
// deviation used for sampling is the length of the pool divided by this factor.
constexpr double kStddevScalingFactor = 3.0;
} // namespace

NamePool::NamePool() = default;

NamePool::~NamePool() = default;

void NamePool::setNames(const QStringList &names)
{
    m_initPool = names;
    syncPool();
}

void NamePool::setState(const QStringList &initPool, const QStringList &pool)
{
    m_initPool = initPool;
    m_pool = pool;
    // Repair configs written by older versions, whose pool only held the
    // names that had not been drawn yet.
    syncPool();
}

void NamePool::addName(const QString &name)
{
    if (name.isEmpty())
        return;

    m_initPool.append(name);
    syncPool();
}

void NamePool::removeName(const QString &name)
{
    m_initPool.removeAll(name);
    m_pool.removeAll(name);
}

QString NamePool::draw()
{
    if (m_initPool.isEmpty())
        return QString();

    // The pool invariant is kept by setNames()/setState()/addName()/
    // removeName(); a draw only moves one entry, so rebuilding it through
    // syncPool() on every draw would be wasted work on a large list.
    int index = 0;
    for (;;) {
        const double stddev = static_cast<double>(m_pool.size()) / kStddevScalingFactor;
        const double sample = std::fabs(m_random.nextGaussian(0.0, stddev));

        if (sample < static_cast<double>(m_pool.size())) {
            index = static_cast<int>(sample);
            break;
        }
    }

    const QString value = m_pool.at(index);

    // Move the drawn entry to the end by its index; searching for the value
    // would scan the whole list when it holds tens of thousands of names.
    m_pool.removeAt(index);
    m_pool.append(value);

    return value;
}

void NamePool::reset()
{
    m_pool = m_initPool;
}

QString NamePool::randomName()
{
    if (m_pool.isEmpty())
        return QString();

    return m_pool.at(m_random.nextInt(0, m_pool.size() - 1));
}

QStringList NamePool::sample(int maxNames) const
{
    const QStringList &source = m_pool.isEmpty() ? m_initPool : m_pool;
    const int count = static_cast<int>(std::min<qsizetype>(source.size(), maxNames));
    if (count <= 0)
        return QStringList();

    QStringList result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        const int index = static_cast<int>(static_cast<qint64>(i) * source.size() / count);
        result.append(source.at(index));
    }
    return result;
}

void NamePool::syncPool()
{
    m_pool = buildSyncedPool(m_initPool, m_pool);
}

QStringList buildSyncedPool(const QStringList &init, const QStringList &history)
{
    QHash<QString, int> required;
    for (const QString &name : init)
        required[name] = required.value(name, 0) + 1;

    // Keep the draw history order of the names that are still listed.
    QStringList keptHistory;
    QHash<QString, int> kept;
    for (const QString &name : history) {
        if (kept.value(name, 0) < required.value(name, 0)) {
            keptHistory.append(name);
            kept[name] = kept.value(name, 0) + 1;
        }
    }

    // Names that are missing from the pool have never been drawn, so they go
    // to the front (in init pool order), where the Gaussian distribution
    // gives them the highest chance of being picked.
    QStringList fresh;
    QHash<QString, int> added;
    for (const QString &name : init) {
        if (kept.value(name, 0) + added.value(name, 0) < required.value(name, 0)) {
            fresh.append(name);
            added[name] = added.value(name, 0) + 1;
        }
    }

    return fresh + keptHistory;
}

int NamePool::size() const
{
    return m_initPool.size();
}

bool NamePool::isEmpty() const
{
    return m_initPool.isEmpty();
}

QStringList NamePool::names() const
{
    return m_initPool;
}

QStringList NamePool::pool() const
{
    return m_pool;
}

bool NamePool::hasName(const QString &name) const
{
    return m_initPool.contains(name);
}
