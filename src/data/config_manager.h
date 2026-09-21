#pragma once

#include <QString>
#include <QStringList>

class ConfigManager
{
public:
    explicit ConfigManager(const QString &overridePath = QString());
    ~ConfigManager();

    int version() const;

    bool fileExists() const;

    bool autoLaunch() const;
    void setAutoLaunch(bool enabled);

    // Floating window (stored as "Mode" in the config file: 0 = disabled, 1 = enabled)
    bool floatingWindow() const;
    void setFloatingWindow(bool enabled);

    QString password() const;
    void setPassword(const QString &password);

    QStringList initPool() const;
    void setInitPool(const QStringList &names);

    QStringList pool() const;
    void setPool(const QStringList &names);

    void sync();

private:
    void loadFromFile();

    QString m_filePath;
    bool m_autoLaunch = false;
    bool m_floatingWindow = false;
    QString m_password;
    QStringList m_initPool;
    QStringList m_pool;
};
