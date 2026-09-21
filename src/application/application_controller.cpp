#include "application_controller.h"
#include "core/name_pool.h"
#include "data/config_manager.h"
#include "platform/single_instance.h"
#include "platform/auto_launch.h"
#include "ui/draw_window.h"
#include "ui/floating_window.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMessageBox>

ApplicationController::ApplicationController(QObject *parent)
    : QObject(parent)
    , m_namePool(std::make_unique<NamePool>())
    , m_config(std::make_unique<ConfigManager>())
    , m_singleInstance(std::make_unique<SingleInstance>())
    , m_autoLaunch(std::make_unique<AutoLaunch>())
    , m_drawWindow(std::make_unique<DrawWindow>())
    , m_floatingWindow(std::make_unique<FloatingWindow>())
{
    connect(m_singleInstance.get(), &SingleInstance::commandReceived, this, [this](const QString &cmd) {
        if (cmd == "trigger") {
            triggerDraw();
        }
    });

    connect(m_floatingWindow.get(), &FloatingWindow::drawRequested,
            this, &ApplicationController::triggerDraw);

    // The window was closed from outside the menu (e.g. Alt+F4): keep the
    // menu item and the config file in sync with the real state.
    connect(m_floatingWindow.get(), &FloatingWindow::closedByUser, this, [this]() {
        if (m_isQuitting || !m_floatingWindowEnabled)
            return;

        m_floatingWindowEnabled = false;
        saveFloatingWindowState(false);
        emit floatingWindowChanged(false);
    });

    if (QCoreApplication *app = QCoreApplication::instance()) {
        // qApp->quit() makes QApplication close every top level window before
        // aboutToQuit is emitted, so watch QEvent::Quit to learn about the
        // shutdown early enough (see prepareForQuit()).
        app->installEventFilter(this);
        connect(app, &QCoreApplication::aboutToQuit, this, [this]() { m_isQuitting = true; });
    }
}

bool ApplicationController::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Quit)
        m_isQuitting = true;

    return QObject::eventFilter(watched, event);
}

void ApplicationController::prepareForQuit()
{
    m_isQuitting = true;
}

ApplicationController::~ApplicationController() = default;

bool ApplicationController::init()
{
    if (!m_singleInstance->tryLock()) {
        return false;
    }

    if (!m_config->fileExists()) {
        const QStringList defaultNames{"Item1", "Item2", "Item3", "Item4", "Item5"};
        m_config->setInitPool(defaultNames);
        m_config->setPool(defaultNames);

        m_config->sync();
    }

    m_namePool->setState(m_config->initPool(), m_config->pool());

    // Restore the floating window state persisted as "Mode" in the config file.
    m_floatingWindowEnabled = m_config->floatingWindow();
    if (m_floatingWindowEnabled)
        m_floatingWindow->showCentered();

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

    m_config->setInitPool(names);
    m_config->setPool(names);
    m_config->sync();
}

void ApplicationController::triggerDraw()
{
    const QString result = m_namePool->draw();
    if (result.isEmpty()) {
        QMessageBox::information(nullptr, "Drawer", "列表为空，请先添加名称。");
        return;
    }

    m_drawWindow->showName(result);

    m_config->setInitPool(m_namePool->names());
    m_config->setPool(m_namePool->remainingNames());
    m_config->sync();
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

void ApplicationController::setFloatingWindow(bool enabled)
{
    // clicked state to Drawer.config (Mode:1 / Mode:0).
    saveFloatingWindowState(enabled);

    if (m_floatingWindowEnabled == enabled)
        return;

    m_floatingWindowEnabled = enabled;

    if (enabled)
        m_floatingWindow->showCentered();
    else
        m_floatingWindow->hide();

    emit floatingWindowChanged(enabled);
}

bool ApplicationController::isFloatingWindowEnabled() const
{
    return m_floatingWindowEnabled;
}

void ApplicationController::saveFloatingWindowState(bool enabled)
{
    m_config->setFloatingWindow(enabled);
    m_config->sync();
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
