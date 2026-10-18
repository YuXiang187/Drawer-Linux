#pragma once

#include <QList>
#include <QPair>
#include <QString>

// Line based "key:value" store, using the same file format as the Windows
// version. A key ends at the first colon, so values may contain colons.
// Unknown keys are kept untouched, which lets both versions share a file.
class KeyValueStore
{
public:
    explicit KeyValueStore(const QString &filePath);

    // Reads the file. A missing or unreadable file simply yields an empty
    // store, so callers can always apply their defaults afterwards.
    bool load();
    bool save() const;

    bool contains(const QString &key) const;
    QString value(const QString &key, const QString &defaultValue = QString()) const;
    void setValue(const QString &key, const QString &value);
    void remove(const QString &key);

    QString filePath() const;

private:
    int indexOf(const QString &key) const;

    QString m_filePath;
    QList<QPair<QString, QString>> m_entries;
};
