#pragma once

#include <QObject>
#include <QStringList>
#include <QTimer>
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

    // Marks the application as shutting down, so that windows closed by the
    // shutdown itself are not mistaken for user closes.
    void prepareForQuit();

    QStringList currentNames() const;
    bool verifyPassword(const QString &password) const;
    bool changePassword(const QString &oldPassword, const QString &newPassword);

    // Groups: every group owns an independent name list, only the active one
    // takes part in drawing.
    QStringList groupIds() const;
    QString groupName(const QString &id) const;
    QString activeGroupId() const;
    QStringList groupInitNames(const QString &id) const;
    int groupCount() const;
    QString findGroupIdByName(const QString &name) const;
    QString addGroup(const QString &name);
    bool renameGroup(const QString &id, const QString &name);
    bool deleteGroup(const QString &id);
    // Makes the group active and reloads the draw pool from it.
    bool activateGroup(const QString &id);
    // Persists the edited name list of a group without activating it.
    void saveGroupNames(const QString &id, const QStringList &names);

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
    void autoLaunchChanged(bool enabled);
    void drawActiveChanged(bool active);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // Writes the floating window state to Drawer.config as Mode:0 / Mode:1.
    void saveFloatingWindowState(bool enabled);

    // Encrypts and stores the draw history staged since the last flush. The
    // history of a large list is written in batches instead of after every
    // single draw.
    void flushPendingPools();

    // True while the desktop session (logout/shutdown) is tearing the
    // application down. Window closes during that phase are not user closes.
    bool isSessionShuttingDown() const;

    std::unique_ptr<NamePool> m_namePool;
    std::unique_ptr<ConfigManager> m_config;
    std::unique_ptr<SingleInstance> m_singleInstance;
    std::unique_ptr<AutoLaunch> m_autoLaunch;
    std::unique_ptr<DrawWindow> m_drawWindow;
    std::unique_ptr<FloatingWindow> m_floatingWindow;
    QTimer m_poolFlushTimer;
    bool m_floatingWindowEnabled = false;
    bool m_isQuitting = false;
};
