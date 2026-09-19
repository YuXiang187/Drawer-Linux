#include "encryptor.h"

#include <openssl/err.h>
#include <openssl/evp.h>

#include <QByteArray>

Encryptor::Encryptor()
    : m_key("3QjNx7RvAU58qkJF")
{
}

QString Encryptor::encrypt(const QString &plaintext) const
{
    if (plaintext.isEmpty())
        return QString();

    const QByteArray in = plaintext.toUtf8();
    const int blockSize = EVP_CIPHER_block_size(EVP_aes_128_ecb());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return QString();

    if (!EVP_CipherInit_ex(ctx, EVP_aes_128_ecb(), nullptr,
                           reinterpret_cast<const unsigned char *>(m_key.constData()),
                           nullptr, 1)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    if (!EVP_CIPHER_CTX_set_padding(ctx, 1)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    QByteArray out(in.size() + blockSize, 0);
    int outLen = 0;

    if (!EVP_CipherUpdate(ctx,
                          reinterpret_cast<unsigned char *>(out.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char *>(in.constData()),
                          static_cast<int>(in.size()))) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    int finalLen = 0;
    if (!EVP_CipherFinal_ex(ctx,
                            reinterpret_cast<unsigned char *>(out.data()) + outLen,
                            &finalLen)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outLen + finalLen);

    return QString::fromUtf8(out.toBase64());
}

QString Encryptor::decrypt(const QString &base64Cipher) const
{
    if (base64Cipher.isEmpty())
        return QString();

    const QByteArray in = QByteArray::fromBase64(base64Cipher.toUtf8());
    if (in.isEmpty())
        return QString();

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return QString();

    if (!EVP_CipherInit_ex(ctx, EVP_aes_128_ecb(), nullptr,
                           reinterpret_cast<const unsigned char *>(m_key.constData()),
                           nullptr, 0)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    if (!EVP_CIPHER_CTX_set_padding(ctx, 1)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    QByteArray out(in.size() + EVP_CIPHER_block_size(EVP_aes_128_ecb()), 0);
    int outLen = 0;

    if (!EVP_CipherUpdate(ctx,
                          reinterpret_cast<unsigned char *>(out.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char *>(in.constData()),
                          static_cast<int>(in.size()))) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    int finalLen = 0;
    if (!EVP_CipherFinal_ex(ctx,
                            reinterpret_cast<unsigned char *>(out.data()) + outLen,
                            &finalLen)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }

    EVP_CIPHER_CTX_free(ctx);
    out.resize(outLen + finalLen);

    return QString::fromUtf8(out);
}
