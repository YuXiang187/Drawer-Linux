#include "tray_manager.h"
#include "application/application_controller.h"
#include "editor_dialog.h"
#include "password_dialog.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QStyle>
#include <QSystemTrayIcon>

TrayManager::TrayManager(ApplicationController *controller, QObject *parent)
    : QObject(parent)
    , m_controller(controller)
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_menu(std::make_unique<QMenu>())
    , m_actionAutoLaunch(nullptr)
    , m_actionFloatingWindow(nullptr)
{
    m_trayIcon->setToolTip("YuXiang Drawer");
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    buildMenu();
    m_trayIcon->setContextMenu(m_menu.get());

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            m_controller->triggerDraw();
        }
    });

    // The window may also be closed from outside the menu (e.g. Alt+F4).
    connect(m_controller, &ApplicationController::floatingWindowChanged,
            this, &TrayManager::syncFloatingWindowAction);
}

TrayManager::~TrayManager()
{
    // Detach the menu before it is destroyed, then drop the tray icon.
    m_trayIcon->setContextMenu(nullptr);
    m_trayIcon->hide();
}

void TrayManager::buildMenu()
{
    // Draw
    QAction *actionDraw = m_menu->addAction("抽取");
    connect(actionDraw, &QAction::triggered, m_controller, &ApplicationController::triggerDraw);

    m_menu->addSeparator();

    // Auto launch
    m_actionAutoLaunch = m_menu->addAction("自启");
    m_actionAutoLaunch->setCheckable(true);
    m_actionAutoLaunch->setChecked(m_controller->isAutoLaunch());
    connect(m_actionAutoLaunch, &QAction::toggled, m_controller, &ApplicationController::setAutoLaunch);

    // Floating window
    m_actionFloatingWindow = m_menu->addAction("浮窗");
    m_actionFloatingWindow->setCheckable(true);
    m_actionFloatingWindow->setChecked(m_controller->isFloatingWindowEnabled());
    connect(m_actionFloatingWindow, &QAction::toggled,
            m_controller, &ApplicationController::setFloatingWindow);

    // Edit
    QAction *actionEdit = m_menu->addAction("编辑");
    connect(actionEdit, &QAction::triggered, this, [this]() {
        PasswordDialog dlg("编辑", "密码：", nullptr);
        if (dlg.exec() == QDialog::Accepted) {
            if (m_controller->verifyPassword(dlg.password())) {
                if (dlg.password() == QStringLiteral("123456")) {
                    QMessageBox::information(nullptr, "编辑",
                        "检测到您正在使用初始密码。\n"
                        "为确保列表内容不被恶意篡改，请及时使用“密码”功能更换密码。");
                }
                EditorDialog editor(m_controller, nullptr);
                editor.exec();
            } else {
                QMessageBox::critical(nullptr, "编辑", "密码错误。");
            }
        }
    });

    // Statistics
    QAction *actionStats = m_menu->addAction("统计");
    connect(actionStats, &QAction::triggered, m_controller, &ApplicationController::showStatistics);

    // Hotkey
    QAction *actionHotkey = m_menu->addAction("帮助");
    connect(actionHotkey, &QAction::triggered, this, [this]() {
        QMessageBox::information(nullptr, "帮助",
            "如需绑定外部热键，请在 系统设置 → 快捷键 中，\n"
            "将命令绑定到：\n\n"
            "  drawer --trigger\n\n"
            "如需让浮窗始终置顶，请按照以下指引操作。\n\n"
            "KDE桌面置顶：\n"
            "按下Alt+F3，在弹出的菜单中点击“更多操作” - “置顶”\n\n"
            "GNOME桌面置顶：\n"
            "按下Alt+Space，在弹出的菜单中点击“置顶”\n\n"
            "如需设置浮窗固定位置，请在KWin中添加窗口规则。");
    });

    // About
    QAction *actionAbout = m_menu->addAction("关于");
    connect(actionAbout, &QAction::triggered, m_controller, &ApplicationController::about);

    m_menu->addSeparator();

    // Quit
    QAction *actionQuit = m_menu->addAction("退出");
    connect(actionQuit, &QAction::triggered, this, [this]() {
        // Quitting closes the floating window as well; mark the shutdown first
        // so that close is not mistaken for a user close (which writes Mode:0).
        m_controller->prepareForQuit();
        qApp->quit();
    });
}

void TrayManager::show()
{
    m_trayIcon->show();
}

void TrayManager::syncFloatingWindowAction(bool enabled)
{
    if (!m_actionFloatingWindow)
        return;

    // Block the signal: the controller already owns the state.
    const QSignalBlocker blocker(m_actionFloatingWindow);
    m_actionFloatingWindow->setChecked(enabled);
}
