#pragma once

#include <QStringList>

// Deprecated: JSON storage replaced by ConfigManager writing Drawer.config.
struct PoolData
{
    int version = 1;
    QStringList initPool;
    QStringList pool;
};

class DataStore
{
public:
    explicit DataStore(const QString &overridePath = QString());

    bool save(const PoolData &data) const;
    PoolData load() const;

private:
    QString m_overridePath;
};
