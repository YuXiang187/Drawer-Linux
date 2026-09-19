#include "data_store.h"

// Deprecated: DataStore JSON logic is replaced by ConfigManager writing Drawer.config.
// This file is kept only to avoid breaking existing includes.

DataStore::DataStore(const QString &overridePath)
    : m_overridePath(overridePath)
{
    Q_UNUSED(m_overridePath)
}

bool DataStore::save(const PoolData &data) const
{
    Q_UNUSED(data)
    return false;
}

PoolData DataStore::load() const
{
    return PoolData();
}
