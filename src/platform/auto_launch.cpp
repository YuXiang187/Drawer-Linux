#include "auto_launch.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTextStream>

AutoLaunch::AutoLaunch(const QString &overrideConfigDir)
    : m_overrideConfigDir(overrideConfigDir)
{
}

QString AutoLaunch::desktopFilePath() const
{
    QString configDir = m_overrideConfigDir;
    if (configDir.isEmpty()) {
        configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    }
    return QDir(configDir).absoluteFilePath("autostart/drawer.desktop");
}

QString AutoLaunch::execPath() const
{
    if (QCoreApplication::instance()) {
        return QCoreApplication::applicationFilePath();
    }
    return QString("/usr/local/bin/drawer");
}

bool AutoLaunch::isEnabled() const
{
    return QFile::exists(desktopFilePath());
}

bool AutoLaunch::enable()
{
    const QString path = desktopFilePath();
    const QString dir = QFileInfo(path).absolutePath();
    QDir().mkpath(dir);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=YuXiang Drawer\n";
    out << "Comment=Name Random Drawer\n";
    // Quoted: the executable path may contain spaces.
    out << "Exec=\"" << execPath() << "\"\n";
    out << "Type=Application\n";
    out << "Terminal=false\n";
    out << "Icon=media-playback-start\n";
    out << "Categories=Utility;\n";
    out.flush();

    if (out.status() != QTextStream::Ok) {
        file.close();
        QFile::remove(path); // do not leave a half written entry behind
        return false;
    }

    file.close();
    return true;
}

bool AutoLaunch::disable()
{
    const QString path = desktopFilePath();
    // Nothing to remove means autostart is already off.
    if (!QFile::exists(path))
        return true;

    return QFile::remove(path);
}
