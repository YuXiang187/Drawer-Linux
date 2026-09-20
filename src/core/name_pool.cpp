#include "name_pool.h"
#include "random_engine.h"

NamePool::NamePool()
    : m_random(new RandomEngine)
{}

NamePool::~NamePool()
{
    delete m_random;
}

void NamePool::setNames(const QStringList &names)
{
    m_initPool = names;
    m_pool = m_initPool;
}

void NamePool::setState(const QStringList &initPool, const QStringList &pool)
{
    m_initPool = initPool;
    m_pool = pool;
}

void NamePool::addName(const QString &name)
{
    if (name.isEmpty())
        return;

    m_initPool.append(name);
    m_pool.append(name);
}

void NamePool::removeName(const QString &name)
{
    m_initPool.removeAll(name);
    m_pool.removeAll(name);
}

QString NamePool::draw()
{
    if (m_pool.isEmpty()) {
        if (m_initPool.isEmpty())
            return QString();
        reset();
    }

    const int index = m_random->nextInt(0, m_pool.size() - 1);
    const QString result = m_pool.takeAt(index);
    return result;
}

void NamePool::reset()
{
    m_pool = m_initPool;
}

int NamePool::size() const
{
    return m_initPool.size();
}

int NamePool::remainingSize() const
{
    return m_pool.size();
}

bool NamePool::isEmpty() const
{
    return m_initPool.isEmpty();
}

QStringList NamePool::names() const
{
    return m_initPool;
}

QStringList NamePool::remainingNames() const
{
    return m_pool;
}

bool NamePool::hasName(const QString &name) const
{
    return m_initPool.contains(name);
}
