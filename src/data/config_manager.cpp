#include "config_manager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

ConfigManager::ConfigManager(const QString &overridePath)
{
    QString dirPath = overridePath;
    if (dirPath.isEmpty()) {
        dirPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir().mkpath(dirPath);

        // Migrate legacy config from the old hardcoded path if the new one does not exist
        const QString iniFile = QDir(dirPath).absoluteFilePath("drawer.ini");
        if (!QFile::exists(iniFile)) {
            const QString legacyDir = QDir::home().filePath(".config/Drawer");
            const QString legacyIni = QDir(legacyDir).absoluteFilePath("drawer.ini");
            if (QFile::exists(legacyIni)) {
                QFile::copy(legacyIni, iniFile);
            }
        }
    }

    const QString iniFile = QDir(dirPath).absoluteFilePath("drawer.ini");
    m_settings = std::make_unique<QSettings>(iniFile, QSettings::IniFormat);
}

ConfigManager::~ConfigManager() = default;

int ConfigManager::version() const
{
    return m_settings->value("version", 1).toInt();
}

bool ConfigManager::autoLaunch() const
{
    return m_settings->value("autoLaunch", false).toBool();
}

void ConfigManager::setAutoLaunch(bool enabled)
{
    m_settings->setValue("autoLaunch", enabled);
}

QString ConfigManager::password() const
{
    return m_settings->value("password", "123456").toString();
}

void ConfigManager::setPassword(const QString &password)
{
    m_settings->setValue("password", password);
}

void ConfigManager::sync()
{
    m_settings->sync();
}
