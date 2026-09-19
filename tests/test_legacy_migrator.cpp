#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "data/legacy_migrator.h"

class TestLegacyMigrator : public QObject
{
    Q_OBJECT

private slots:
    void testParseConfig();
    void testAesDecryption();
    void testFileNotExist();
    void testCorruptedData();
};

void TestLegacyMigrator::testParseConfig()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile file(tempDir.path() + "/Drawer.config");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << "Mode:0\n";
    out << "isAutoLaunch:true\n";
    out << "Hotkey:F8\n";
    out << "Key:Yw5eKi//NQgt69jux/1HfQ==\n";
    out << "initPool:OMpOcezBBlbG3U4oQTaooNuDCgXUQzz74B7FN6IAzE8=\n";
    out << "pool:OMpOcezBBlbG3U4oQTaooNuDCgXUQzz74B7FN6IAzE8=\n";
    file.close();

    LegacyMigrator migrator;
    LegacyData data = migrator.migrate(file.fileName());

    QVERIFY(data.valid);
    QCOMPARE(data.mode, QString("hotkey"));
    QCOMPARE(data.isAutoLaunch, true);
    QCOMPARE(data.hotkey, QString("F8"));
    QCOMPARE(data.password, QString("123456"));
    QCOMPARE(data.initPool, QStringList({"Item1", "Item2", "Item3", "Item4", "Item5"}));
    QCOMPARE(data.pool, QStringList({"Item1", "Item2", "Item3", "Item4", "Item5"}));
}

void TestLegacyMigrator::testAesDecryption()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile file(tempDir.path() + "/Drawer.config");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << "Mode:1\n";
    out << "isAutoLaunch:false\n";
    out << "Hotkey:Ctrl+Shift+H\n";
    out << "Key:Yw5eKi//NQgt69jux/1HfQ==\n";
    out << "initPool:OMpOcezBBlbG3U4oQTaooNuDCgXUQzz74B7FN6IAzE8=\n";
    out << "pool:OMpOcezBBlbG3U4oQTaooNuDCgXUQzz74B7FN6IAzE8=\n";
    file.close();

    LegacyMigrator migrator;
    LegacyData data = migrator.migrate(file.fileName());

    QVERIFY(data.valid);
    QCOMPARE(data.mode, QString("float"));
    QCOMPARE(data.isAutoLaunch, false);
    QCOMPARE(data.hotkey, QString("Ctrl+Shift+H"));
}

void TestLegacyMigrator::testFileNotExist()
{
    LegacyMigrator migrator;
    LegacyData data = migrator.migrate("/nonexistent/path/Drawer.config");
    QVERIFY(!data.valid);
}

void TestLegacyMigrator::testCorruptedData()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile file(tempDir.path() + "/Drawer.config");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << "Mode:0\n";
    out << "isAutoLaunch:false\n";
    out << "Hotkey:F9\n";
    out << "Key:invalid_base64!!!\n";
    out << "initPool:\n";
    out << "pool:\n";
    file.close();

    LegacyMigrator migrator;
    LegacyData data = migrator.migrate(file.fileName());

    QVERIFY(data.valid);  // file parsed, but decryption fields may be empty
    QCOMPARE(data.mode, QString("hotkey"));
    QCOMPARE(data.isAutoLaunch, false);
    QCOMPARE(data.hotkey, QString("F9"));
    QCOMPARE(data.password, QString());  // invalid base64 -> empty
    QVERIFY(data.initPool.isEmpty());
    QVERIFY(data.pool.isEmpty());
}

QTEST_MAIN(TestLegacyMigrator)
#include "test_legacy_migrator.moc"
