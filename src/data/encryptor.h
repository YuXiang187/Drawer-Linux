#pragma once

#include <QString>

class Encryptor
{
public:
    Encryptor();

    QString encrypt(const QString &plaintext) const;
    QString decrypt(const QString &base64Cipher) const;

private:
    QByteArray m_key;
};
