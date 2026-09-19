#pragma once

#include <QString>
#include <QStringList>

struct LegacyData
{
    QString mode;           // "hotkey", "float", or "pause"
    bool isAutoLaunch = false;
    QString hotkey;
    QString password;       // decrypted
    QStringList initPool;   // decrypted, split by comma
    QStringList pool;       // decrypted, split by comma
    bool valid = false;     // true if config was successfully parsed
};

class LegacyMigrator
{
public:
    LegacyMigrator();

    LegacyData migrate(const QString &configFilePath) const;
};
