#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "data/config_manager.h"

class TestDataStore : public QObject
{
    Q_OBJECT

private slots:
    void testSaveAndLoad();
    void testChineseNames();
    void testEmptyList();
    void testNoPlaintextInFile();
    void testPasswordRecovery();
    void testPoolRecovery();
    void testIsAutoLaunch();
};

void TestDataStore::testSaveAndLoad()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager store(tempDir.path());
    store.setInitPool(QStringList{"Alice", "Bob", "Carol"});
    store.setPool(QStringList{"Alice", "Carol"});
    store.sync();

    ConfigManager loaded(tempDir.path());
    QCOMPARE(loaded.initPool(), QStringList({"Alice", "Bob", "Carol"}));
    QCOMPARE(loaded.pool(), QStringList({"Alice", "Carol"}));
}

void TestDataStore::testChineseNames()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QStringList names{
        QString::fromUtf8("张三"),
        QString::fromUtf8("李四"),
        QString::fromUtf8("王五")
    };

    ConfigManager store(tempDir.path());
    store.setInitPool(names);
    store.setPool(names);
    store.sync();

    ConfigManager loaded(tempDir.path());
    QCOMPARE(loaded.initPool(), names);
    QCOMPARE(loaded.pool(), names);
}

void TestDataStore::testEmptyList()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager store(tempDir.path());
    store.setInitPool(QStringList{});
    store.setPool(QStringList{});
    store.sync();

    ConfigManager loaded(tempDir.path());
    QVERIFY(loaded.initPool().isEmpty());
    QVERIFY(loaded.pool().isEmpty());
}

void TestDataStore::testNoPlaintextInFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString password = QStringLiteral("Secret123");
    QStringList initNames{QStringLiteral("Alice"), QStringLiteral("Bob")};
    QStringList poolNames{QStringLiteral("Charlie")};

    ConfigManager store(tempDir.path());
    store.setPassword(password);
    store.setInitPool(initNames);
    store.setPool(poolNames);
    store.sync();

    QFile file(tempDir.path() + "/Drawer.config");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    QString content = in.readAll();

    // No plaintext password
    QVERIFY(!content.contains(password));
    // No plaintext names
    for (const QString &name : initNames) {
        QVERIFY(!content.contains(name));
    }
    for (const QString &name : poolNames) {
        QVERIFY(!content.contains(name));
    }
}

void TestDataStore::testPasswordRecovery()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    ConfigManager store(tempDir.path());
    store.setPassword("MyPass123");
    store.sync();

    ConfigManager loaded(tempDir.path());
    QCOMPARE(loaded.password(), QString("MyPass123"));
}

void TestDataStore::testPoolRecovery()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QStringList initNames{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")};
    QStringList poolNames{QStringLiteral("A"), QStringLiteral("C")};

    ConfigManager store(tempDir.path());
    store.setInitPool(initNames);
    store.setPool(poolNames);
    store.sync();

    ConfigManager loaded(tempDir.path());
    QCOMPARE(loaded.initPool(), initNames);
    QCOMPARE(loaded.pool(), poolNames);
}

void TestDataStore::testIsAutoLaunch()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager store(tempDir.path());
        store.setAutoLaunch(true);
        store.setInitPool(QStringList{"Test"});
        store.sync();
    }

    {
        ConfigManager loaded(tempDir.path());
        QVERIFY(loaded.autoLaunch());
        QCOMPARE(loaded.initPool(), QStringList{"Test"});
    }
}

QTEST_MAIN(TestDataStore)
#include "test_data_store.moc"
