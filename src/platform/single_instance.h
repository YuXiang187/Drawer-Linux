#pragma once

#include <QObject>
#include <QString>

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
    QLockFile *m_lockFile;
    bool m_locked;
    QLocalServer *m_server;
};
