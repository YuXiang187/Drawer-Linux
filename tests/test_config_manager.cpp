#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "data/config_manager.h"
#include "data/encryptor.h"
#include "data/legacy_migrator.h"

class TestConfigManager : public QObject
{
    Q_OBJECT

private slots:
    void testDefaults();
    void testSetAndGet();
    void testPersistence();
    void testFileFormat();
    void testPasswordEncryption();
    void testAutoLaunchPersistence();
    void testConfigFileExistence();
    void testEmptyPoolIsValidState();
    void testMigratedLegacyDataWrittenToConfigFile();
};

void TestConfigManager::testDefaults()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    QCOMPARE(cm.version(), 1);
    QCOMPARE(cm.autoLaunch(), false);
    QCOMPARE(cm.password(), QString("123456"));
    QVERIFY(cm.initPool().isEmpty());
    QVERIFY(cm.pool().isEmpty());
}

void TestConfigManager::testSetAndGet()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    cm.setAutoLaunch(true);
    cm.setPassword("newpassword");
    cm.setInitPool(QStringList{"Alice", "Bob"});
    cm.setPool(QStringList{"Alice"});

    QCOMPARE(cm.autoLaunch(), true);
    QCOMPARE(cm.password(), QString("newpassword"));
    QCOMPARE(cm.initPool(), QStringList({"Alice", "Bob"}));
    QCOMPARE(cm.pool(), QStringList({"Alice"}));
}

void TestConfigManager::testPersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager cm(tempDir.path());
        cm.setAutoLaunch(true);
        cm.setPassword("secret");
        cm.setInitPool(QStringList{"A", "B", "C"});
        cm.setPool(QStringList{"A", "C"});
        cm.sync();
    }

    {
        ConfigManager cm(tempDir.path());
        QCOMPARE(cm.autoLaunch(), true);
        QCOMPARE(cm.password(), QString("secret"));
        QCOMPARE(cm.initPool(), QStringList({"A", "B", "C"}));
        QCOMPARE(cm.pool(), QStringList({"A", "C"}));
    }
}

void TestConfigManager::testFileFormat()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    cm.setAutoLaunch(false);
    cm.setPassword("123");
    cm.setInitPool(QStringList{"X", "Y"});
    cm.setPool(QStringList{"X"});
    cm.sync();

    QFile file(tempDir.path() + "/Drawer.config");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    QString content = in.readAll();

    QVERIFY(content.contains("Mode:0"));
    QVERIFY(content.contains("Hotkey:F8"));
    QVERIFY(content.contains("isAutoLaunch:false"));
}

void TestConfigManager::testPasswordEncryption()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    cm.setPassword("mypassword");
    cm.sync();

    QFile file(tempDir.path() + "/Drawer.config");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    QString content = in.readAll();

    // The raw password must NOT appear in the file
    QVERIFY(!content.contains("mypassword"));
    // The Key line must contain something (encrypted data)
    QVERIFY(content.contains("Key:"));
}

void TestConfigManager::testAutoLaunchPersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager cm(tempDir.path());
        cm.setAutoLaunch(true);
        cm.sync();
    }

    {
        ConfigManager cm(tempDir.path());
        QVERIFY(cm.autoLaunch());
    }

    {
        ConfigManager cm(tempDir.path());
        cm.setAutoLaunch(false);
        cm.sync();
    }

    {
        ConfigManager cm(tempDir.path());
        QVERIFY(!cm.autoLaunch());
    }
}

void TestConfigManager::testConfigFileExistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    QVERIFY(!cm.fileExists());

    cm.sync();
    QVERIFY(cm.fileExists());

    ConfigManager reloaded(tempDir.path());
    QVERIFY(reloaded.fileExists());
}

void TestConfigManager::testEmptyPoolIsValidState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager cm(tempDir.path());
        cm.setInitPool({});
        cm.setPool({});
        cm.sync();
    }

    ConfigManager cm(tempDir.path());
    QVERIFY(cm.fileExists());
    QVERIFY(cm.initPool().isEmpty());
    QVERIFY(cm.pool().isEmpty());
    QCOMPARE(cm.password(), QString("123456"));
}

void TestConfigManager::testMigratedLegacyDataWrittenToConfigFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Encryptor encryptor;
    QFile legacyFile(tempDir.path() + "/legacy.config");
    QVERIFY(legacyFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&legacyFile);
    out << "Mode:1\n";
    out << "isAutoLaunch:true\n";
    out << "Hotkey:Ctrl+Shift+H\n";
    out << "Key:" << encryptor.encrypt("secret") << "\n";
    out << "initPool:" << encryptor.encrypt("Alice,Bob,Carol") << "\n";
    out << "pool:" << encryptor.encrypt("Alice,Carol") << "\n";
    legacyFile.close();

    LegacyMigrator migrator;
    const LegacyData legacy = migrator.migrate(legacyFile.fileName());
    QVERIFY(legacy.valid);

    ConfigManager cm(tempDir.path());
    QVERIFY(!cm.fileExists());
    cm.setInitPool(legacy.initPool);
    cm.setPool(legacy.pool);
    cm.setAutoLaunch(legacy.isAutoLaunch);
    cm.setPassword(legacy.password);
    cm.sync();

    ConfigManager reloaded(tempDir.path());
    QCOMPARE(reloaded.initPool(), QStringList({"Alice", "Bob", "Carol"}));
    QCOMPARE(reloaded.pool(), QStringList({"Alice", "Carol"}));
    QCOMPARE(reloaded.autoLaunch(), true);
    QCOMPARE(reloaded.password(), QString("secret"));

    QFile configFile(tempDir.path() + "/Drawer.config");
    QVERIFY(configFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&configFile);
    const QString content = in.readAll();
    QVERIFY(content.contains("Mode:0"));
    QVERIFY(content.contains("Hotkey:F8"));
    QVERIFY(content.contains("isAutoLaunch:true"));
    QVERIFY(!content.contains("Ctrl+Shift+H"));
}

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"
