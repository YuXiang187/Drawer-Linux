#include <QTest>
#include <QTemporaryDir>
#include "data/config_manager.h"

class TestConfigManager : public QObject
{
    Q_OBJECT

private slots:
    void testDefaults();
    void testSetAndGet();
    void testPersistence();
};

void TestConfigManager::testDefaults()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    QCOMPARE(cm.version(), 1);
    QCOMPARE(cm.autoLaunch(), false);
}

void TestConfigManager::testSetAndGet()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager cm(tempDir.path());
    cm.setAutoLaunch(true);

    QCOMPARE(cm.autoLaunch(), true);
}

void TestConfigManager::testPersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager cm(tempDir.path());
        cm.setAutoLaunch(true);
        cm.setPassword("newpassword");
        cm.sync();
    }

    {
        ConfigManager cm(tempDir.path());
        QCOMPARE(cm.autoLaunch(), true);
        QCOMPARE(cm.password(), QString("newpassword"));
    }
}

QTEST_MAIN(TestConfigManager)
#include "test_config_manager.moc"
