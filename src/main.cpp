#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QStyle>
#include <QSystemTrayIcon>
#include "cli/cli_parser.h"
#include "application/application_controller.h"
#include "platform/single_instance.h"
#include "ui/tray_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("Drawer");
    app.setOrganizationName("YuXiang");
    app.setApplicationVersion("1.0.0");
    app.setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));

    CliParser cli(argc, argv);

    if (cli.wantsHelp()) {
        qDebug() << "Usage: drawer [options]";
        qDebug() << "Options:";
        qDebug() << "  --trigger      Trigger a draw (for external hotkey bindings)";
        qDebug() << "  --version, -v  Show version";
        qDebug() << "  --help, -h     Show this help";
        return 0;
    }

    if (cli.wantsVersion()) {
        qDebug() << "Drawer 1.0.0";
        return 0;
    }

    if (cli.wantsTrigger()) {
        if (SingleInstance::sendCommand("trigger")) {
            qDebug() << "Triggered draw in running instance.";
            return 0;
        }
        qDebug() << "No running instance found.";
        return 1;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Drawer",
            "当前桌面环境不支持系统托盘。\n\n"
            "如果您使用 GNOME 桌面，请安装并启用 AppIndicator 扩展：\n\n"
            "Fedora:\n"
            "sudo dnf install gnome-shell-extension-appindicator\n\n"
            "Debian/Ubuntu:\n"
            "sudo apt install gnome-shell-extension-appindicator\n\n"
            "启用扩展后请重新登录桌面环境。");
        return 1;
    }

    ApplicationController controller;
    if (!controller.init()) {
        QMessageBox::warning(nullptr, "Drawer", "软件已经在运行！");
        return 1;
    }

    TrayManager tray(&controller);
    tray.show();

    return app.exec();
}
