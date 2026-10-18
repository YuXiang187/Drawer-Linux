#include "key_value_store.h"

#include <QDebug>
#include <QFile>
#include <QSaveFile>
#include <QTextStream>

KeyValueStore::KeyValueStore(const QString &filePath)
    : m_filePath(filePath)
{
}

bool KeyValueStore::load()
{
    m_entries.clear();

    QFile file(m_filePath);
    if (!file.exists())
        return false;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "KeyValueStore::load: cannot read" << m_filePath << file.errorString();
        return false;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.endsWith(QLatin1Char('\r')))
            line.chop(1);

        const int separator = line.indexOf(QLatin1Char(':'));
        if (separator <= 0)
            continue;

        const QString key = line.left(separator).trimmed();
        if (key.isEmpty())
            continue;

        // Like the Windows version, the first occurrence of a key wins.
        if (indexOf(key) < 0)
            m_entries.append(qMakePair(key, line.mid(separator + 1)));
    }

    return true;
}

bool KeyValueStore::save() const
{
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "KeyValueStore::save: cannot write" << m_filePath << file.errorString();
        return false;
    }

    QTextStream stream(&file);
    for (const auto &entry : m_entries)
        stream << entry.first << ':' << entry.second << '\n';
    stream.flush();

    if (!file.commit()) {
        qWarning() << "KeyValueStore::save: cannot commit" << m_filePath << file.errorString();
        return false;
    }

    return true;
}

bool KeyValueStore::contains(const QString &key) const
{
    return indexOf(key) >= 0;
}

QString KeyValueStore::value(const QString &key, const QString &defaultValue) const
{
    const int index = indexOf(key);
    return index >= 0 ? m_entries.at(index).second : defaultValue;
}

void KeyValueStore::setValue(const QString &key, const QString &value)
{
    Q_ASSERT(!key.contains(QLatin1Char(':')));
    Q_ASSERT(!key.contains(QLatin1Char('\n')));

    const int index = indexOf(key);
    if (index >= 0) {
        // Keep the position so the file keeps its layout across writes.
        m_entries[index].second = value;
        return;
    }

    m_entries.append(qMakePair(key, value));
}

void KeyValueStore::remove(const QString &key)
{
    for (int i = m_entries.size() - 1; i >= 0; --i) {
        if (m_entries.at(i).first == key)
            m_entries.removeAt(i);
    }
}

QString KeyValueStore::filePath() const
{
    return m_filePath;
}

int KeyValueStore::indexOf(const QString &key) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).first == key)
            return i;
    }
    return -1;
}
