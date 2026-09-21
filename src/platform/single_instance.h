#pragma once

#include <QObject>
#include <QString>
#include <memory>

class QLockFile;
class QLocalServer;

class SingleInstance : public QObject
{
    Q_OBJECT
public:
    explicit SingleInstance(const QString &overridePath = QString(), QObject *parent = nullptr);
    ~SingleInstance();

    bool tryLock();
    void unlock();
    bool isLocked() const;

    static bool sendCommand(const QString &cmd);

signals:
    void commandReceived(const QString &cmd);

private:
    QString lockFilePath() const;

    QString m_overridePath;
    std::unique_ptr<QLockFile> m_lockFile;
    std::unique_ptr<QLocalServer> m_server;
    bool m_locked = false;
};
