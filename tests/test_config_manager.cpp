#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "data/config_manager.h"
#include "data/encryptor.h"
#include "data/group_store.h"

namespace {

void writeConfig(const QString &path, const QString &content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QTextStream out(&file);
    out << content;
}

QString readConfig(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    QTextStream in(&file);
    return in.readAll();
}

} // namespace

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
    void testLegacyConfigMigration();
    void testLegacyEmptyListMigration();
    void testUnknownKeysArePreserved();
    void testGroupPersistence();
};

void TestConfigManager::testDefaults()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    QCOMPARE(cm.version(), 2);
    QCOMPARE(cm.autoLaunch(), false);
    QCOMPARE(cm.password(), QString("123456"));

    // A fresh config already holds the group the application draws from.
    QCOMPARE(cm.groups()->count(), 1);
    QCOMPARE(cm.groups()->ids(), QStringList({"g0"}));
    QCOMPARE(cm.groups()->name("g0"), GroupStore::DefaultGroupName);
    QCOMPARE(cm.groups()->activeId(), QString("g0"));
    QCOMPARE(cm.initPool(), QStringList({"Item1", "Item2", "Item3", "Item4", "Item5"}));
    QCOMPARE(cm.pool(), cm.initPool());
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

    QString content = readConfig(tempDir.path() + "/Drawer.config");
    QVERIFY(content.contains("Mode:0"));
    QVERIFY(content.contains("Hotkey:F8"));
    QVERIFY(content.contains("isAutoLaunch:false"));
    QVERIFY(content.contains("Groups:g0"));
    QVERIFY(content.contains("CurrentGroup:g0"));
    QVERIFY(content.contains("name@g0:"));
    QVERIFY(content.contains("initPool@g0:"));
    QVERIFY(content.contains("pool@g0:"));

    // Enabling the floating window writes Mode:1
    cm.setFloatingWindow(true);
    cm.sync();

    content = readConfig(tempDir.path() + "/Drawer.config");
    QVERIFY(content.contains("Mode:1"));
}

void TestConfigManager::testPasswordEncryption()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    cm.setPassword("mypassword");
    cm.sync();

    const QString content = readConfig(tempDir.path() + "/Drawer.config");

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

void TestConfigManager::testLegacyConfigMigration()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Encryptor encryptor;
    const QString path = tempDir.path() + "/Drawer.config";
    writeConfig(path, QString("Mode:1\n"
                              "isAutoLaunch:true\n"
                              "Key:%1\n"
                              "initPool:%2\n"
                              "pool:%3\n")
                          .arg(encryptor.encrypt("123456"),
                               encryptor.encrypt("Alice,Bob"),
                               encryptor.encrypt("Bob,Alice")));

    {
        ConfigManager cm(tempDir.path());

        // The single list of the old format becomes the default group, with
        // its draw history intact.
        QCOMPARE(cm.groups()->count(), 1);
        QCOMPARE(cm.groups()->ids(), QStringList({"g0"}));
        QCOMPARE(cm.groups()->name("g0"), GroupStore::DefaultGroupName);
        QCOMPARE(cm.groups()->activeId(), QString("g0"));
        QCOMPARE(cm.initPool(), QStringList({"Alice", "Bob"}));
        QCOMPARE(cm.pool(), QStringList({"Bob", "Alice"}));
        QVERIFY(cm.floatingWindow());
        QVERIFY(cm.autoLaunch());
        QCOMPARE(cm.password(), QString("123456"));

        cm.sync();
    }

    const QString content = readConfig(path);
    QVERIFY(!content.contains("initPool:"));
    QVERIFY(!content.contains("pool:"));
    QVERIFY(content.contains("initPool@g0:"));
    QVERIFY(content.contains("pool@g0:"));

    // The migrated names survive a restart.
    ConfigManager reloaded(tempDir.path());
    QCOMPARE(reloaded.initPool(), QStringList({"Alice", "Bob"}));
    QCOMPARE(reloaded.pool(), QStringList({"Bob", "Alice"}));
}

void TestConfigManager::testLegacyEmptyListMigration()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // An empty list is a valid configuration and must stay empty.
    writeConfig(tempDir.path() + "/Drawer.config", "initPool:\npool:\n");

    ConfigManager cm(tempDir.path());
    QCOMPARE(cm.groups()->count(), 1);
    QVERIFY(cm.initPool().isEmpty());
    QVERIFY(cm.pool().isEmpty());
}

void TestConfigManager::testUnknownKeysArePreserved()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString path = tempDir.path() + "/Drawer.config";
    writeConfig(path, "Custom:value\nMode:1\nisAutoLaunch:false\n");

    ConfigManager cm(tempDir.path());
    cm.sync();

    // Keys this version does not know about must survive a rewrite, so a
    // config shared with another version is not stripped.
    const QString content = readConfig(path);
    QVERIFY(content.contains("Custom:value"));
    QVERIFY(content.contains("Mode:1"));
    QVERIFY(content.contains("Groups:g0"));
}

void TestConfigManager::testGroupPersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager cm(tempDir.path());
        const QString id = cm.groups()->add("Class A");
        cm.groups()->setInit(id, QStringList{"Alice", "Bob"});
        cm.groups()->setActiveId(id);
        cm.sync();
    }

    {
        ConfigManager cm(tempDir.path());
        QCOMPARE(cm.groups()->count(), 2);
        QCOMPARE(cm.groups()->activeId(), QString("g1"));
        QCOMPARE(cm.groups()->name("g1"), QString("Class A"));
        QCOMPARE(cm.initPool(), QStringList({"Alice", "Bob"}));
        QCOMPARE(cm.pool(), QStringList({"Alice", "Bob"}));
    }
}

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"
