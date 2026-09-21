#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "data/config_manager.h"

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
    void testFloatingWindowDefaults();
    void testFloatingWindowPersistence();
    void testFloatingWindowModeParsing();
    void testConfigFileExistence();
    void testEmptyPoolIsValidState();
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

    // Enabling the floating window writes Mode:1
    cm.setFloatingWindow(true);
    cm.sync();

    QFile enabled(tempDir.path() + "/Drawer.config");
    QVERIFY(enabled.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream enabledIn(&enabled);
    QVERIFY(enabledIn.readAll().contains("Mode:1"));
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

void TestConfigManager::testFloatingWindowDefaults()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    QCOMPARE(cm.floatingWindow(), false);
    QCOMPARE(cm.version(), 1);
}

void TestConfigManager::testFloatingWindowPersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager cm(tempDir.path());
        cm.setFloatingWindow(true);
        cm.sync();
    }

    {
        ConfigManager cm(tempDir.path());
        QVERIFY(cm.floatingWindow());
        cm.setFloatingWindow(false);
        cm.sync();
    }

    {
        ConfigManager cm(tempDir.path());
        QVERIFY(!cm.floatingWindow());
    }
}

void TestConfigManager::testFloatingWindowModeParsing()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString path = tempDir.path() + "/Drawer.config";
    const auto writeMode = [&path](const QString &mode) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
        QTextStream out(&file);
        out << "Mode:" << mode << "\n";
        out << "isAutoLaunch:false\n";
        out.flush();
    };

    writeMode("1");
    QVERIFY(ConfigManager(tempDir.path()).floatingWindow());

    writeMode("0");
    QVERIFY(!ConfigManager(tempDir.path()).floatingWindow());

    // Unknown values must not enable the floating window
    writeMode("2");
    QVERIFY(!ConfigManager(tempDir.path()).floatingWindow());
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

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"
