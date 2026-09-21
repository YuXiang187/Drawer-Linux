#include "encryptor.h"

#include <openssl/err.h>
#include <openssl/evp.h>

#include <QByteArray>

#include <memory>

namespace {

// Owns the OpenSSL cipher context, so every early return below releases it.
using CipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

CipherCtxPtr makeCipherCtx()
{
    return CipherCtxPtr(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
}

} // namespace

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

    CipherCtxPtr ctx = makeCipherCtx();
    if (!ctx)
        return QString();

    if (!EVP_CipherInit_ex(ctx.get(), EVP_aes_128_ecb(), nullptr,
                           reinterpret_cast<const unsigned char *>(m_key.constData()),
                           nullptr, 1)) {
        return QString();
    }

    if (!EVP_CIPHER_CTX_set_padding(ctx.get(), 1))
        return QString();

    QByteArray out(in.size() + blockSize, 0);
    int outLen = 0;

    if (!EVP_CipherUpdate(ctx.get(),
                          reinterpret_cast<unsigned char *>(out.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char *>(in.constData()),
                          static_cast<int>(in.size()))) {
        return QString();
    }

    int finalLen = 0;
    if (!EVP_CipherFinal_ex(ctx.get(),
                            reinterpret_cast<unsigned char *>(out.data()) + outLen,
                            &finalLen)) {
        return QString();
    }

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

    CipherCtxPtr ctx = makeCipherCtx();
    if (!ctx)
        return QString();

    if (!EVP_CipherInit_ex(ctx.get(), EVP_aes_128_ecb(), nullptr,
                           reinterpret_cast<const unsigned char *>(m_key.constData()),
                           nullptr, 0)) {
        return QString();
    }

    if (!EVP_CIPHER_CTX_set_padding(ctx.get(), 1))
        return QString();

    QByteArray out(in.size() + EVP_CIPHER_block_size(EVP_aes_128_ecb()), 0);
    int outLen = 0;

    if (!EVP_CipherUpdate(ctx.get(),
                          reinterpret_cast<unsigned char *>(out.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char *>(in.constData()),
                          static_cast<int>(in.size()))) {
        return QString();
    }

    int finalLen = 0;
    if (!EVP_CipherFinal_ex(ctx.get(),
                            reinterpret_cast<unsigned char *>(out.data()) + outLen,
                            &finalLen)) {
        return QString();
    }

    out.resize(outLen + finalLen);

    return QString::fromUtf8(out);
}
