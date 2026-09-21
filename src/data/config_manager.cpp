#include "config_manager.h"
#include "encryptor.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

ConfigManager::ConfigManager(const QString &overridePath)
{
    QString dirPath = overridePath;
    if (dirPath.isEmpty()) {
        dirPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    }
    QDir().mkpath(dirPath);
    m_filePath = QDir(dirPath).absoluteFilePath("Drawer.config");

    loadFromFile();

    // Apply sensible defaults when file does not exist or fields are missing
    if (m_password.isEmpty()) {
        m_password = QStringLiteral("123456");
    }
}

ConfigManager::~ConfigManager() = default;

void ConfigManager::loadFromFile()
{
    QFile file(m_filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    Encryptor encryptor;

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty())
            continue;

        int sep = line.indexOf(':');
        if (sep < 0)
            continue;

        QString key = line.left(sep).trimmed();
        QString value = line.mid(sep + 1).trimmed();

        if (key == QLatin1String("isAutoLaunch")) {
            m_autoLaunch = (value.toLower() == QLatin1String("true"));
        } else if (key == QLatin1String("Mode")) {
            m_floatingWindow = (value.toInt() == 1);
        } else if (key == QLatin1String("Key")) {
            m_password = encryptor.decrypt(value);
        } else if (key == QLatin1String("initPool")) {
            QString decrypted = encryptor.decrypt(value);
            if (!decrypted.isEmpty()) {
                m_initPool = decrypted.split(',', Qt::SkipEmptyParts);
            }
        } else if (key == QLatin1String("pool")) {
            QString decrypted = encryptor.decrypt(value);
            if (!decrypted.isEmpty()) {
                m_pool = decrypted.split(',', Qt::SkipEmptyParts);
            }
        }
        // Hotkey is fixed on write and ignored on read
    }
}

int ConfigManager::version() const
{
    return 1;
}

bool ConfigManager::fileExists() const
{
    return QFile::exists(m_filePath);
}

bool ConfigManager::autoLaunch() const
{
    return m_autoLaunch;
}

void ConfigManager::setAutoLaunch(bool enabled)
{
    m_autoLaunch = enabled;
}

bool ConfigManager::floatingWindow() const
{
    return m_floatingWindow;
}

void ConfigManager::setFloatingWindow(bool enabled)
{
    m_floatingWindow = enabled;
}

QString ConfigManager::password() const
{
    return m_password;
}

void ConfigManager::setPassword(const QString &password)
{
    m_password = password;
}

QStringList ConfigManager::initPool() const
{
    return m_initPool;
}

void ConfigManager::setInitPool(const QStringList &names)
{
    m_initPool = names;
}

QStringList ConfigManager::pool() const
{
    return m_pool;
}

void ConfigManager::setPool(const QStringList &names)
{
    m_pool = names;
}

void ConfigManager::sync()
{
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "ConfigManager::sync: cannot write" << m_filePath << file.errorString();
        return;
    }

    QTextStream stream(&file);
    Encryptor encryptor;

    stream << "Mode:" << (m_floatingWindow ? 1 : 0) << "\n";
    stream << "isAutoLaunch:" << (m_autoLaunch ? "true" : "false") << "\n";
    stream << "Hotkey:F8\n";

    if (!m_password.isEmpty()) {
        stream << "Key:" << encryptor.encrypt(m_password) << "\n";
    } else {
        stream << "Key:\n";
    }

    if (!m_initPool.isEmpty()) {
        stream << "initPool:" << encryptor.encrypt(m_initPool.join(",")) << "\n";
    } else {
        stream << "initPool:\n";
    }

    if (!m_pool.isEmpty()) {
        stream << "pool:" << encryptor.encrypt(m_pool.join(",")) << "\n";
    } else {
        stream << "pool:\n";
    }

    stream.flush();
}

