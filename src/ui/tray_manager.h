#pragma once

#include <QObject>

class ApplicationController;
class QSystemTrayIcon;
class QMenu;
class QAction;

class TrayManager : public QObject
{
    Q_OBJECT
public:
    explicit TrayManager(ApplicationController *controller, QObject *parent = nullptr);
    void show();

private:
    void buildMenu();

    ApplicationController *m_controller;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_menu;
    QAction *m_actionAutoLaunch;
};
