#pragma once

#include <QObject>
#include <QStringList>
#include <memory>

class NamePool;
class ConfigManager;
class DataStore;
class SingleInstance;
class AutoLaunch;
class DrawWindow;

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
    void showStatistics();
    void about();

    bool isAutoLaunch() const;

private:
    std::unique_ptr<NamePool> m_namePool;
    std::unique_ptr<ConfigManager> m_config;
    std::unique_ptr<DataStore> m_dataStore;
    SingleInstance *m_singleInstance;
    std::unique_ptr<AutoLaunch> m_autoLaunch;
    DrawWindow *m_drawWindow;
};
