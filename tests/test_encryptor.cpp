#include <QTest>
#include <QString>
#include "data/encryptor.h"

class TestEncryptor : public QObject
{
    Q_OBJECT

private slots:
    void testRoundTrip();
    void testChinese();
    void testSpecialCharacters();
    void testEmptyString();
    void testCompatibilityWithLegacy();
};

void TestEncryptor::testRoundTrip()
{
    Encryptor enc;
    const QString original = "Hello, World! This is a test.";
    const QString cipher = enc.encrypt(original);
    QVERIFY(!cipher.isEmpty());
    const QString decrypted = enc.decrypt(cipher);
    QCOMPARE(decrypted, original);
}

void TestEncryptor::testChinese()
{
    Encryptor enc;
    const QString original = QString::fromUtf8("张三,李四,王五,这是中文测试");
    const QString cipher = enc.encrypt(original);
    QVERIFY(!cipher.isEmpty());
    const QString decrypted = enc.decrypt(cipher);
    QCOMPARE(decrypted, original);
}

void TestEncryptor::testSpecialCharacters()
{
    Encryptor enc;
    const QString original = "!@#$%^&*()_+-=[]{}|;':\",./<>?";
    const QString cipher = enc.encrypt(original);
    QVERIFY(!cipher.isEmpty());
    const QString decrypted = enc.decrypt(cipher);
    QCOMPARE(decrypted, original);
}

void TestEncryptor::testEmptyString()
{
    Encryptor enc;
    const QString cipher = enc.encrypt("");
    QCOMPARE(cipher, QString());

    const QString decrypted = enc.decrypt("");
    QCOMPARE(decrypted, QString());
}

void TestEncryptor::testCompatibilityWithLegacy()
{
    Encryptor enc;

    // Legacy Drawer.config default values from C# project
    QCOMPARE(enc.decrypt("Yw5eKi//NQgt69jux/1HfQ=="), QString("123456"));
    QCOMPARE(enc.decrypt("OMpOcezBBlbG3U4oQTaooNuDCgXUQzz74B7FN6IAzE8="),
             QString("Item1,Item2,Item3,Item4,Item5"));
}

QTEST_MAIN(TestEncryptor)
#include "test_encryptor.moc"
