#pragma once

#include <QString>
#include <memory>

class QSettings;

class ConfigManager
{
public:
    explicit ConfigManager(const QString &overridePath = QString());
    ~ConfigManager();

    int version() const;

    bool autoLaunch() const;
    void setAutoLaunch(bool enabled);

    QString password() const;
    void setPassword(const QString &password);

    void sync();

private:
    std::unique_ptr<QSettings> m_settings;
};
