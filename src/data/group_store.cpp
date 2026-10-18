#include "group_store.h"
#include "key_value_store.h"

#include "core/name_pool.h"

namespace {
// Names a new group starts with, identical to a fresh installation.
const QStringList kSampleNames{"Item1", "Item2", "Item3", "Item4", "Item5"};
} // namespace

const QString GroupStore::DefaultGroupName = QStringLiteral("默认");

GroupStore::GroupStore(KeyValueStore *store)
    : m_store(store)
{
}

bool GroupStore::migrateLegacyList()
{
    if (!ids().isEmpty())
        return false;

    // Versions without groups stored a single list in "initPool" and "pool".
    // An empty value is a valid list, so presence of the key matters, not
    // whether it holds anything.
    const bool hasLegacyInit = m_store->contains(QStringLiteral("initPool"));
    const bool hasLegacyPool = m_store->contains(QStringLiteral("pool"));
    const QString legacyInit = m_store->value(QStringLiteral("initPool"));
    const QString legacyPool = m_store->value(QStringLiteral("pool"));
    const QString sample = m_encryptor.encrypt(kSampleNames.join(QLatin1Char(',')));

    m_store->setValue(QStringLiteral("Groups"), QStringLiteral("g0"));
    m_store->setValue(QStringLiteral("CurrentGroup"), QStringLiteral("g0"));
    m_store->setValue(QStringLiteral("NextId"), QStringLiteral("1"));
    m_store->setValue(QStringLiteral("name@g0"), DefaultGroupName);
    m_store->setValue(QStringLiteral("initPool@g0"), hasLegacyInit ? legacyInit : sample);
    m_store->setValue(QStringLiteral("pool@g0"),
                      hasLegacyPool ? legacyPool : (hasLegacyInit ? legacyInit : sample));

    m_store->remove(QStringLiteral("initPool"));
    m_store->remove(QStringLiteral("pool"));
    return true;
}

QStringList GroupStore::ids() const
{
    return split(m_store->value(QStringLiteral("Groups")));
}

QString GroupStore::name(const QString &id) const
{
    return m_store->value(QStringLiteral("name@") + id);
}

QString GroupStore::activeId() const
{
    const QStringList list = ids();
    if (list.isEmpty())
        return QString();

    const QString active = m_store->value(QStringLiteral("CurrentGroup"));
    return list.contains(active) ? active : list.first();
}

bool GroupStore::setActiveId(const QString &id)
{
    if (!ids().contains(id))
        return false;

    m_store->setValue(QStringLiteral("CurrentGroup"), id);
    return true;
}

QStringList GroupStore::init(const QString &id) const
{
    if (id.isEmpty())
        return QStringList();

    const auto cached = m_initCache.constFind(id);
    if (cached != m_initCache.constEnd())
        return cached.value();

    const QStringList names = decryptedList(QStringLiteral("initPool@") + id);
    m_initCache.insert(id, names);
    return names;
}

void GroupStore::setInit(const QString &id, const QStringList &names)
{
    if (id.isEmpty())
        return;

    // Keep the draw history of the names that remain listed, exactly like
    // changing the list of the single-group version did. The pool may hold
    // draws that are staged but not written yet, so read it from the cache.
    const QStringList rebuiltPool = buildSyncedPool(names, pool(id));

    m_initCache.insert(id, names);
    m_poolCache.insert(id, rebuiltPool);
    m_dirtyPools.remove(id);

    writeList(QStringLiteral("initPool@") + id, names);
    writeList(QStringLiteral("pool@") + id, rebuiltPool);
}

QStringList GroupStore::pool(const QString &id) const
{
    if (id.isEmpty())
        return QStringList();

    const auto cached = m_poolCache.constFind(id);
    if (cached != m_poolCache.constEnd())
        return cached.value();

    const QStringList names = decryptedList(QStringLiteral("pool@") + id);
    m_poolCache.insert(id, names);
    return names;
}

void GroupStore::setPool(const QString &id, const QStringList &names)
{
    if (id.isEmpty())
        return;

    m_poolCache.insert(id, names);
    m_dirtyPools.remove(id);
    writeList(QStringLiteral("pool@") + id, names);
}

void GroupStore::stagePool(const QString &id, const QStringList &names)
{
    if (id.isEmpty())
        return;

    m_poolCache.insert(id, names);
    m_dirtyPools.insert(id);
}

bool GroupStore::flushPools()
{
    if (m_dirtyPools.isEmpty())
        return false;

    const QSet<QString> dirty = m_dirtyPools;
    m_dirtyPools.clear();

    for (const QString &id : dirty) {
        const auto cached = m_poolCache.constFind(id);
        if (cached != m_poolCache.constEnd())
            writeList(QStringLiteral("pool@") + id, cached.value());
    }
    return true;
}

int GroupStore::count() const
{
    return ids().size();
}

QString GroupStore::add(const QString &name)
{
    bool ok = false;
    int next = m_store->value(QStringLiteral("NextId")).toInt(&ok);
    if (!ok)
        next = 1;

    // Skip ids that are already taken, e.g. when NextId was edited by hand;
    // two groups sharing an id would overwrite each other's list.
    const QStringList existing = ids();
    QString id;
    do {
        id = QStringLiteral("g") + QString::number(next);
        ++next;
    } while (existing.contains(id));

    QStringList list = existing;
    list.append(id);

    m_store->setValue(QStringLiteral("Groups"), list.join(QLatin1Char(',')));
    m_store->setValue(QStringLiteral("NextId"), QString::number(next));
    m_store->setValue(QStringLiteral("name@") + id, name);

    const QString sample = m_encryptor.encrypt(kSampleNames.join(QLatin1Char(',')));
    m_store->setValue(QStringLiteral("initPool@") + id, sample);
    m_store->setValue(QStringLiteral("pool@") + id, sample);

    m_initCache.insert(id, kSampleNames);
    m_poolCache.insert(id, kSampleNames);
    m_dirtyPools.remove(id);
    return id;
}

bool GroupStore::rename(const QString &id, const QString &name)
{
    if (!ids().contains(id))
        return false;

    m_store->setValue(QStringLiteral("name@") + id, name);
    return true;
}

bool GroupStore::remove(const QString &id)
{
    QStringList list = ids();
    if (list.size() <= 1 || !list.removeOne(id))
        return false;

    const QString active = activeId();

    m_store->setValue(QStringLiteral("Groups"), list.join(QLatin1Char(',')));
    m_store->remove(QStringLiteral("name@") + id);
    m_store->remove(QStringLiteral("initPool@") + id);
    m_store->remove(QStringLiteral("pool@") + id);

    // Drop the cached lists too, so a staged history is never written back.
    m_initCache.remove(id);
    m_poolCache.remove(id);
    m_dirtyPools.remove(id);

    if (id == active)
        m_store->setValue(QStringLiteral("CurrentGroup"), list.first());
    return true;
}

QString GroupStore::findByName(const QString &name) const
{
    const QString wanted = name.trimmed();
    const QStringList list = ids();
    for (const QString &id : list) {
        if (this->name(id).trimmed().compare(wanted, Qt::CaseInsensitive) == 0)
            return id;
    }
    return QString();
}

QStringList GroupStore::split(const QString &value)
{
    if (value.isEmpty())
        return QStringList();

    return value.split(QLatin1Char(','), Qt::SkipEmptyParts);
}

QStringList GroupStore::decryptedList(const QString &key) const
{
    return split(m_encryptor.decrypt(m_store->value(key)));
}

void GroupStore::writeList(const QString &key, const QStringList &names)
{
    m_store->setValue(key, m_encryptor.encrypt(names.join(QLatin1Char(','))));
}
