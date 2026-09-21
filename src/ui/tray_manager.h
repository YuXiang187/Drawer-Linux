#pragma once

#include <QObject>
#include <memory>

class ApplicationController;
class QSystemTrayIcon;
class QMenu;
class QAction;

class TrayManager : public QObject
{
    Q_OBJECT
public:
    explicit TrayManager(ApplicationController *controller, QObject *parent = nullptr);
    ~TrayManager() override;

    void show();

private:
    void buildMenu();
    void syncFloatingWindowAction(bool enabled);
    void syncAutoLaunchAction(bool enabled);

    ApplicationController *m_controller;
    QSystemTrayIcon *m_trayIcon;
    std::unique_ptr<QMenu> m_menu;
    QAction *m_actionAutoLaunch;
    QAction *m_actionFloatingWindow;
};
