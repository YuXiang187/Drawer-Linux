#include "legacy_migrator.h"
#include "encryptor.h"

#include <QFile>
#include <QTextStream>

LegacyMigrator::LegacyMigrator()
{
}

LegacyData LegacyMigrator::migrate(const QString &configFilePath) const
{
    LegacyData data;
    data.valid = false;

    QFile file(configFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return data;

    QTextStream stream(&file);
    Encryptor encryptor;

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty())
            continue;

        int sep = line.indexOf(':');
        if (sep < 0)
            continue;

        QString key = line.left(sep).trimmed();
        QString value = line.mid(sep + 1).trimmed();

        if (key == "Mode") {
            if (value == "0")
                data.mode = "hotkey";
            else if (value == "1")
                data.mode = "float";
            else
                data.mode = "pause";
        } else if (key == "isAutoLaunch") {
            data.isAutoLaunch = (value.toLower() == "true");
        } else if (key == "Hotkey") {
            data.hotkey = value;
        } else if (key == "Key") {
            data.password = encryptor.decrypt(value);
        } else if (key == "initPool") {
            QString decrypted = encryptor.decrypt(value);
            if (!decrypted.isEmpty()) {
                data.initPool = decrypted.split(',', Qt::SkipEmptyParts);
            }
        } else if (key == "pool") {
            QString decrypted = encryptor.decrypt(value);
            if (!decrypted.isEmpty()) {
                data.pool = decrypted.split(',', Qt::SkipEmptyParts);
            }
        }
    }

    data.valid = true;
    return data;
}
