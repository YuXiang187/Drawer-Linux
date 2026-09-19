#include "data_store.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

DataStore::DataStore(const QString &overridePath)
    : m_overridePath(overridePath)
{}

bool DataStore::save(const PoolData &data) const
{
    QJsonObject obj;
    obj["version"] = data.version;

    QJsonArray initArr;
    for (const QString &name : data.initPool) {
        initArr.append(name);
    }
    obj["init_pool"] = initArr;

    QJsonArray poolArr;
    for (const QString &name : data.pool) {
        poolArr.append(name);
    }
    obj["pool"] = poolArr;

    QJsonDocument doc(obj);

    const QString path = filePath();
    const QString dir = QFileInfo(path).absolutePath();
    QDir().mkpath(dir);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "DataStore::save: failed to open" << path << "for writing:" << file.errorString();
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

PoolData DataStore::load() const
{
    PoolData data;

    QString path = filePath();

    // Fallback to the legacy hardcoded path only when using the default standard path
    if (!QFile::exists(path) && m_overridePath.isEmpty()) {
        const QString legacyDir = QDir::home().filePath(".config/Drawer");
        const QString legacyPath = QDir(legacyDir).absoluteFilePath("drawer_data.json");
        if (QFile::exists(legacyPath)) {
            path = legacyPath;
        } else {
            return data;
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "DataStore::load: failed to open" << path << "for reading:" << file.errorString();
        return data;
    }

    const QByteArray bytes = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "DataStore::load: corrupted JSON in" << filePath();
        return data;
    }

    const QJsonObject obj = doc.object();
    data.version = obj.value("version").toInt(1);

    const QJsonArray initArr = obj.value("init_pool").toArray();
    for (const QJsonValue &v : initArr) {
        data.initPool.append(v.toString());
    }

    const QJsonArray poolArr = obj.value("pool").toArray();
    for (const QJsonValue &v : poolArr) {
        data.pool.append(v.toString());
    }

    return data;
}

QString DataStore::filePath() const
{
    if (!m_overridePath.isEmpty()) {
        return QDir(m_overridePath).absoluteFilePath("drawer_data.json");
    }

    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    return QDir(path).absoluteFilePath("drawer_data.json");
}
