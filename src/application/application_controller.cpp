#include "application_controller.h"
#include "core/name_pool.h"
#include "data/config_manager.h"
#include "data/data_store.h"
#include "data/legacy_migrator.h"
#include "platform/single_instance.h"
#include "platform/auto_launch.h"
#include "ui/draw_window.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>

ApplicationController::ApplicationController(QObject *parent)
    : QObject(parent)
    , m_namePool(std::make_unique<NamePool>())
    , m_config(std::make_unique<ConfigManager>())
    , m_dataStore(std::make_unique<DataStore>())
    , m_singleInstance(std::make_unique<SingleInstance>())
    , m_autoLaunch(std::make_unique<AutoLaunch>())
    , m_drawWindow(std::make_unique<DrawWindow>())
{
    connect(m_singleInstance.get(), &SingleInstance::commandReceived, this, [this](const QString &cmd) {
        if (cmd == "trigger") {
            triggerDraw();
        }
    });
}

ApplicationController::~ApplicationController() = default;

bool ApplicationController::init()
{
    if (!m_singleInstance->tryLock()) {
        return false;
    }

    PoolData data = m_dataStore->load();
    if (data.initPool.isEmpty()) {
        const QString legacyPath = QDir(QCoreApplication::applicationDirPath()).filePath("Drawer.config");
        if (QFile::exists(legacyPath)) {
            LegacyMigrator migrator;
            const LegacyData legacy = migrator.migrate(legacyPath);
            if (legacy.valid && !legacy.initPool.isEmpty()) {
                data.initPool = legacy.initPool;
                data.pool = legacy.pool;
                m_dataStore->save(data);
            }
            if (legacy.valid && !legacy.password.isEmpty()) {
                m_config->setPassword(legacy.password);
                m_config->sync();
            }
        }
    }

    if (!data.initPool.isEmpty()) {
        m_namePool->setNames(data.initPool);
    }

    return true;
}

QStringList ApplicationController::currentNames() const
{
    return m_namePool->names();
}

bool ApplicationController::verifyPassword(const QString &password) const
{
    return m_config->password() == password;
}

bool ApplicationController::changePassword(const QString &oldPassword, const QString &newPassword)
{
    if (!verifyPassword(oldPassword))
        return false;

    if (newPassword.isEmpty())
        return false;

    m_config->setPassword(newPassword);
    m_config->sync();
    return true;
}

void ApplicationController::setNames(const QStringList &names)
{
    m_namePool->setNames(names);

    PoolData data;
    data.initPool = names;
    data.pool = names;
    m_dataStore->save(data);
}

void ApplicationController::triggerDraw()
{
    const QString result = m_namePool->draw();
    if (result.isEmpty()) {
        QMessageBox::information(nullptr, "Drawer", "列表为空，请先添加名称。");
        return;
    }

    m_drawWindow->showName(result);

    PoolData data;
    data.initPool = m_namePool->names();
    data.pool = m_namePool->remainingNames();
    m_dataStore->save(data);
}

void ApplicationController::setAutoLaunch(bool enabled)
{
    if (enabled)
        m_autoLaunch->enable();
    else
        m_autoLaunch->disable();
    m_config->setAutoLaunch(enabled);
    m_config->sync();
}

bool ApplicationController::isAutoLaunch() const
{
    return m_autoLaunch->isEnabled();
}

void ApplicationController::showStatistics()
{
    const QStringList names = m_namePool->names();
    const QString text = QString("抽取数量：%1\n\n抽取名单：%2")
                             .arg(names.size())
                             .arg(names.join(", "));
    QMessageBox::information(nullptr, "统计", text);
}

void ApplicationController::about()
{
    QMessageBox::about(nullptr, "关于 YuXiang Drawer",
        "YuXiang Drawer：名称随机抽取器\n\n"
        "版本 1.0\n"
        "作者 YuXiang187\n\n"
        "“编辑”功能的初始密码为 123456。");
}
