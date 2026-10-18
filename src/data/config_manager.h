#pragma once

#include "encryptor.h"
#include "group_store.h"
#include "key_value_store.h"

#include <QString>
#include <QStringList>

class ConfigManager
{
public:
    explicit ConfigManager(const QString &overridePath = QString());
    ~ConfigManager();

    // 2 = grouped name lists, 1 = the single list of earlier versions.
    int version() const;

    bool fileExists() const;

    bool autoLaunch() const;
    void setAutoLaunch(bool enabled);

    // Floating window (stored as "Mode" in the config file: 0 = disabled, 1 = enabled)
    bool floatingWindow() const;
    void setFloatingWindow(bool enabled);

    QString password() const;
    void setPassword(const QString &password);

    // Name lists of the active group.
    QStringList initPool() const;
    void setInitPool(const QStringList &names);

    QStringList pool() const;
    void setPool(const QStringList &names);

    GroupStore *groups();

    void sync();

private:
    void loadSettings();

    QString m_filePath;
    KeyValueStore m_store;
    Encryptor m_encryptor;
    GroupStore m_groups;
    bool m_autoLaunch = false;
    bool m_floatingWindow = false;
    QString m_password;
};
