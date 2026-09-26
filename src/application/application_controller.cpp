#include "application_controller.h"
#include "core/name_pool.h"
#include "data/config_manager.h"
#include "platform/single_instance.h"
#include "platform/auto_launch.h"
#include "ui/draw_window.h"
#include "ui/floating_window.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QEvent>
#include <QGuiApplication>
#include <QMessageBox>

namespace {
// The session bus answers isShuttingDown() in well under a millisecond. This
// upper bound only matters when ksmserver is unresponsive.
constexpr int kSessionQueryTimeoutMs = 500;
} // namespace

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
    // menu item and the config file in sync with the real state. A close
    // issued by the session manager when logging out or shutting down must
    // not be persisted, otherwise the floating window would stay disabled
    // after the next login.
    connect(m_floatingWindow.get(), &FloatingWindow::closedByUser, this, [this]() {
        if (m_isQuitting || !m_floatingWindowEnabled)
            return;

        const bool sessionShuttingDown = isSessionShuttingDown();

        m_floatingWindowEnabled = false;
        if (!sessionShuttingDown)
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

bool ApplicationController::isSessionShuttingDown() const
{
    if ((qGuiApp && qGuiApp->isSavingSession()) || QCoreApplication::closingDown())
        return true;

    // KDE Plasma (at least up to 6.3) has no session management for native
    // Wayland clients: on logout or shutdown KWin simply closes every
    // toplevel, which reaches the application as an ordinary close event,
    // indistinguishable from Alt+F4. During that phase ksmserver is in its
    // shutdown state, so ask it directly to tell the two cases apart.
    if (!QDBusConnection::sessionBus().isConnected())
        return false;

    const QDBusMessage reply = QDBusConnection::sessionBus().call(
        QDBusMessage::createMethodCall(QStringLiteral("org.kde.ksmserver"),
                                       QStringLiteral("/KSMServer"),
                                       QStringLiteral("org.kde.KSMServerInterface"),
                                       QStringLiteral("isShuttingDown")),
        QDBus::Block, kSessionQueryTimeoutMs);

    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty())
        return reply.arguments().constFirst().toBool();

    if (reply.type() == QDBusMessage::ErrorMessage
        && reply.errorName() == QLatin1String("org.freedesktop.DBus.Error.ServiceUnknown")) {
        // Not a KDE session: no ksmserver, so this is a regular user close.
        return false;
    }

    // Unexpected reply (timeout, ...): never risk disabling the window.
    return true;
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
    // Persist the Gaussian pool (draw history order), which merges the new
    // init pool with the history kept so far.
    m_config->setPool(m_namePool->pool());
    m_config->sync();
}

void ApplicationController::triggerDraw()
{
    const QString result = m_namePool->draw();
    if (result.isEmpty()) {
        QMessageBox::information(nullptr, "错误", "列表为空，请先添加名称。");
        return;
    }

    m_drawWindow->showName(result);

    // The Gaussian draw moved the picked name to the end of the pool: persist
    // the updated history so it survives a restart.
    m_config->setInitPool(m_namePool->names());
    m_config->setPool(m_namePool->pool());
    m_config->sync();
}

void ApplicationController::setAutoLaunch(bool enabled)
{
    const bool applied = enabled ? m_autoLaunch->enable() : m_autoLaunch->disable();

    if (!applied) {
        // Do not let the menu item and Drawer.config claim a state that was
        // not actually applied to ~/.config/autostart.
        m_config->setAutoLaunch(!enabled);
        m_config->sync();
        emit autoLaunchChanged(!enabled);

        QMessageBox::warning(nullptr, "自启",
            enabled ? "无法写入开机自启配置。\n\n请检查 ~/.config/autostart 目录的写入权限。"
                    : "无法移除开机自启配置。\n\n请检查 ~/.config/autostart 目录的写入权限。");
        return;
    }

    m_config->setAutoLaunch(enabled);
    m_config->sync();
    emit autoLaunchChanged(enabled);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QString text =
        "YuXiang Drawer：名称随机抽取器<br><br>"
        "版本 1.1.0<br>"
        "作者 YuXiang187<br><br>"
        "“编辑”功能的初始密码为 123456。<br><br>"
        "抽取名称功能的高斯分布模型参考了 "
        "<a href=\"https://github.com/bowlofeggs/rpick\">rpick</a> 的实现。";
#else
    // Hyperlinks are not supported, plain text is displayed
    const QString text =
        "YuXiang Drawer：名称随机抽取器\n\n"
        "版本 1.1.0\n"
        "作者 YuXiang187\n\n"
        "“编辑”功能的初始密码为 123456。\n\n"
        "抽取名称功能的高斯分布模型参考了 rpick 的实现。";
#endif

    QMessageBox::about(nullptr, "关于 YuXiang Drawer", text);
}
