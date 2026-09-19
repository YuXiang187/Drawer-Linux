#pragma once

#include <QString>

class AutoLaunch
{
public:
    explicit AutoLaunch(const QString &overrideConfigDir = QString());

    bool isEnabled() const;
    bool enable();
    bool disable();

private:
    QString desktopFilePath() const;
    QString execPath() const;
    QString m_overrideConfigDir;
};
