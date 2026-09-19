#include "single_instance.h"
#include <QDebug>
#include <QLockFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <unistd.h>

static QString ipcServerName()
{
    return QStringLiteral("yuxiang-drawer-instance-%1").arg(getuid());
}

SingleInstance::SingleInstance(const QString &overridePath, QObject *parent)
    : QObject(parent)
    , m_overridePath(overridePath)
    , m_lockFile(nullptr)
    , m_locked(false)
    , m_server(nullptr)
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

    m_lockFile = new QLockFile(lockFilePath());
    m_lockFile->setStaleLockTime(0);

    if (m_lockFile->tryLock()) {
        m_locked = true;

        const QString serverName = ipcServerName();
        QLocalServer::removeServer(serverName);
        m_server = new QLocalServer(this);
        if (!m_server->listen(serverName)) {
            qWarning() << "SingleInstance::tryLock: QLocalServer::listen() failed:" << m_server->errorString();
            delete m_server;
            m_server = nullptr;
            delete m_lockFile;
            m_lockFile = nullptr;
            m_locked = false;
            return false;
        }

        connect(m_server, &QLocalServer::newConnection, this, [this]() {
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

    delete m_lockFile;
    m_lockFile = nullptr;
    return false;
}

void SingleInstance::unlock()
{
    if (!m_locked)
        return;

    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }

    delete m_lockFile;
    m_lockFile = nullptr;
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
