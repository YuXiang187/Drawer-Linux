#include "tray_manager.h"
#include "application/application_controller.h"
#include "editor_dialog.h"
#include "password_dialog.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QSystemTrayIcon>

TrayManager::TrayManager(ApplicationController *controller, QObject *parent)
    : QObject(parent)
    , m_controller(controller)
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_menu(new QMenu)
    , m_actionAutoLaunch(nullptr)
{
    m_trayIcon->setToolTip("YuXiang Drawer");
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    buildMenu();
    m_trayIcon->setContextMenu(m_menu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            m_controller->triggerDraw();
        }
    });
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

    // Hotkey
    QAction *actionHotkey = m_menu->addAction("热键");
    connect(actionHotkey, &QAction::triggered, this, [this]() {
        QMessageBox::information(nullptr, "热键",
            "如需绑定外部热键，请在 系统设置 → 快捷键 中，\n"
            "将命令绑定到：\n\n"
            "  drawer --trigger");
    });

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

    // About
    QAction *actionAbout = m_menu->addAction("关于");
    connect(actionAbout, &QAction::triggered, m_controller, &ApplicationController::about);

    m_menu->addSeparator();

    // Quit
    QAction *actionQuit = m_menu->addAction("退出");
    connect(actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void TrayManager::show()
{
    m_trayIcon->show();
}
