#include "config_manager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace {

QString configFilePath(const QString &overridePath)
{
    QString dirPath = overridePath;
    if (dirPath.isEmpty())
        dirPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    QDir().mkpath(dirPath);
    return QDir(dirPath).absoluteFilePath(QStringLiteral("Drawer.config"));
}

} // namespace

ConfigManager::ConfigManager(const QString &overridePath)
    : m_filePath(configFilePath(overridePath))
    , m_store(m_filePath)
    , m_groups(&m_store)
{
    m_store.load();

    // A config written before groups existed holds a single list in the
    // "initPool"/"pool" keys; turn it into the first group. A missing config
    // (or one without any group) gets a group with the sample names.
    m_groups.migrateLegacyList();

    loadSettings();

    if (m_password.isEmpty())
        m_password = QStringLiteral("123456");
}

ConfigManager::~ConfigManager() = default;

void ConfigManager::loadSettings()
{
    m_autoLaunch = (m_store.value(QStringLiteral("isAutoLaunch")).trimmed().compare(
                        QLatin1String("true"), Qt::CaseInsensitive)
                    == 0);
    m_floatingWindow = (m_store.value(QStringLiteral("Mode")).trimmed().toInt() == 1);
    m_password = m_encryptor.decrypt(m_store.value(QStringLiteral("Key")).trimmed());

    // "Hotkey" is fixed on write and ignored on read, like the Windows version.
}

int ConfigManager::version() const
{
    return 2;
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
    return m_groups.init(m_groups.activeId());
}

void ConfigManager::setInitPool(const QStringList &names)
{
    const QString id = m_groups.activeId();
    if (!id.isEmpty())
        m_groups.setInit(id, names);
}

QStringList ConfigManager::pool() const
{
    return m_groups.pool(m_groups.activeId());
}

void ConfigManager::setPool(const QStringList &names)
{
    const QString id = m_groups.activeId();
    if (!id.isEmpty())
        m_groups.setPool(id, names);
}

GroupStore *ConfigManager::groups()
{
    return &m_groups;
}

void ConfigManager::sync()
{
    // The group keys are written by GroupStore; only the settings owned by
    // this class are refreshed here. Everything else in the store (including
    // keys written by other versions) is preserved as it is.
    m_store.setValue(QStringLiteral("Mode"), m_floatingWindow ? QStringLiteral("1")
                                                             : QStringLiteral("0"));
    m_store.setValue(QStringLiteral("isAutoLaunch"), m_autoLaunch ? QStringLiteral("true")
                                                                 : QStringLiteral("false"));
    m_store.setValue(QStringLiteral("Hotkey"), QStringLiteral("F8"));

    if (m_password.isEmpty()) {
        m_store.setValue(QStringLiteral("Key"), QString());
    } else {
        m_store.setValue(QStringLiteral("Key"), m_encryptor.encrypt(m_password));
    }

    m_store.save();
}
