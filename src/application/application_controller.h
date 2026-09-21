#pragma once

#include <QObject>
#include <QStringList>
#include <memory>

class NamePool;
class ConfigManager;
class SingleInstance;
class AutoLaunch;
class DrawWindow;
class FloatingWindow;

class ApplicationController : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();

    bool init();

    QStringList currentNames() const;
    bool verifyPassword(const QString &password) const;
    bool changePassword(const QString &oldPassword, const QString &newPassword);
    void setNames(const QStringList &names);

public slots:
    void triggerDraw();
    void setAutoLaunch(bool enabled);
    void setFloatingWindow(bool enabled);
    void showStatistics();
    void about();

    bool isAutoLaunch() const;
    bool isFloatingWindowEnabled() const;

signals:
    void floatingWindowChanged(bool enabled);

private:
    // Writes the floating window state to Drawer.config as Mode:0 / Mode:1.
    void saveFloatingWindowState(bool enabled);
    void showFloatingWindowTip() const;

    std::unique_ptr<NamePool> m_namePool;
    std::unique_ptr<ConfigManager> m_config;
    std::unique_ptr<SingleInstance> m_singleInstance;
    std::unique_ptr<AutoLaunch> m_autoLaunch;
    std::unique_ptr<DrawWindow> m_drawWindow;
    std::unique_ptr<FloatingWindow> m_floatingWindow;
    bool m_floatingWindowEnabled = false;
    bool m_isQuitting = false;
};
