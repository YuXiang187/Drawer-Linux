#pragma once

#include "encryptor.h"

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

class KeyValueStore;

// Several independent name lists ("groups") stored in the config file under
// the same keys as the Windows version, so both applications can read each
// other's configuration:
//
//   Groups:g0,g1           group ids in display order
//   CurrentGroup:g1        the group used for drawing
//   NextId:2               next numeric group id
//   name@g0:默认            group name
//   initPool@g0:<base64>   names in user-edited order
//   pool@g0:<base64>       Gaussian draw history
class GroupStore
{
public:
    static const QString DefaultGroupName;

    explicit GroupStore(KeyValueStore *store);

    // Creates the first group from a pre-group configuration (the "initPool"
    // and "pool" keys of versions without groups) or from the sample names,
    // then drops the legacy keys. Returns true when something changed.
    bool migrateLegacyList();

    QStringList ids() const;
    QString name(const QString &id) const;
    QString activeId() const;
    bool setActiveId(const QString &id);

    QStringList init(const QString &id) const;
    void setInit(const QString &id, const QStringList &names);

    QStringList pool(const QString &id) const;
    void setPool(const QString &id, const QStringList &names);

    // Stores a draw history in memory and marks it for the next flush. A
    // large list must not be encrypted and written on every draw; the caller
    // batches the writes and calls flushPools() (followed by
    // ConfigManager::sync()) from a timer, and once more when quitting.
    void stagePool(const QString &id, const QStringList &names);
    // Writes every staged history into the store, returns true when there was
    // anything to write.
    bool flushPools();

    int count() const;

    // Creates a group seeded with the sample names and returns its id.
    QString add(const QString &name);
    bool rename(const QString &id, const QString &name);
    // Removes a group, refusing to remove the last remaining one.
    bool remove(const QString &id);

    QString findByName(const QString &name) const;

private:
    static QStringList split(const QString &value);

    QStringList decryptedList(const QString &key) const;
    void writeList(const QString &key, const QStringList &names);

    KeyValueStore *m_store;
    Encryptor m_encryptor;

    // Decrypting a list of tens of thousands of names is expensive, so every
    // group is decrypted once and kept here. QStringList is implicitly
    // shared, so handing out a value does not copy the names.
    mutable QHash<QString, QStringList> m_initCache;
    mutable QHash<QString, QStringList> m_poolCache;
    QSet<QString> m_dirtyPools;
};
