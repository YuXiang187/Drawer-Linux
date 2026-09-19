#pragma once

#include <QStringList>

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
    QString filePath() const;
    QString m_overridePath;
};
