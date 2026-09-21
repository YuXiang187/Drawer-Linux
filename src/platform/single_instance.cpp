#include "single_instance.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>
#include <memory>
#include <unistd.h>

static QString ipcServerName()
{
    return QStringLiteral("yuxiang-drawer-instance-%1").arg(getuid());
}

SingleInstance::SingleInstance(const QString &overridePath, QObject *parent)
    : QObject(parent)
    , m_overridePath(overridePath)
{
}

SingleInstance::~SingleInstance()
{
    unlock();
}

QString SingleInstance::lockFilePath() const
{
    if (!m_overridePath.isEmpty()) {
        return QDir(m_overridePath).absoluteFilePath("yuxiang-drawer.lock");
    }

    QString dir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (dir.isEmpty()) {
        dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    }

    QDir().mkpath(dir);
    return QDir(dir).absoluteFilePath("yuxiang-drawer.lock");
}

bool SingleInstance::tryLock()
{
    if (m_locked)
        return true;

    auto lockFile = std::make_unique<QLockFile>(lockFilePath());
    lockFile->setStaleLockTime(0);

    if (!lockFile->tryLock())
        return false;

    const QString serverName = ipcServerName();
    QLocalServer::removeServer(serverName);

    auto server = std::make_unique<QLocalServer>();
    if (!server->listen(serverName)) {
        qWarning() << "SingleInstance::tryLock: QLocalServer::listen() failed:" << server->errorString();
        return false;
    }

    m_lockFile = std::move(lockFile);
    m_server = std::move(server);
    m_locked = true;

    connect(m_server.get(), &QLocalServer::newConnection, this, [this]() {
        QLocalSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
            QByteArray data = socket->readAll();
            QString cmd = QString::fromUtf8(data).trimmed();
            if (!cmd.isEmpty()) {
                emit commandReceived(cmd);
            }
            socket->write("ok\n");
            socket->flush();
        });
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
    });

    return true;
}

void SingleInstance::unlock()
{
    if (!m_locked)
        return;

    if (m_server) {
        m_server->close();
        m_server.reset();
    }

    m_lockFile.reset();
    m_locked = false;
}

bool SingleInstance::isLocked() const
{
    return m_locked;
}

bool SingleInstance::sendCommand(const QString &cmd)
{
    QLocalSocket socket;
    socket.connectToServer(ipcServerName());
    if (!socket.waitForConnected(500))
        return false;

    socket.write(cmd.toUtf8() + "\n");
    socket.flush();
    if (!socket.waitForReadyRead(500))
        return false;

    socket.readAll();
    socket.close();
    return true;
}
