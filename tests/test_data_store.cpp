#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include "data/data_store.h"

class TestDataStore : public QObject
{
    Q_OBJECT

private slots:
    void testSaveAndLoad();
    void testChineseNames();
    void testSpecialCharacters();
    void testEmptyList();
    void testFileNotExist();
    void testCorruptedFile();
};

void TestDataStore::testSaveAndLoad()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DataStore store(tempDir.path());

    PoolData data;
    data.initPool = QStringList{"Alice", "Bob", "Carol"};
    data.pool = QStringList{"Alice", "Carol"};

    QVERIFY(store.save(data));

    PoolData loaded = store.load();
    QCOMPARE(loaded.version, 1);
    QCOMPARE(loaded.initPool, QStringList({"Alice", "Bob", "Carol"}));
    QCOMPARE(loaded.pool, QStringList({"Alice", "Carol"}));
}

void TestDataStore::testChineseNames()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DataStore store(tempDir.path());

    PoolData data;
    data.initPool = QStringList{
        QString::fromUtf8("张三"),
        QString::fromUtf8("李四"),
        QString::fromUtf8("王五")
    };

    QVERIFY(store.save(data));

    PoolData loaded = store.load();
    QCOMPARE(loaded.initPool, data.initPool);
}

void TestDataStore::testSpecialCharacters()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DataStore store(tempDir.path());

    PoolData data;
    data.initPool = QStringList{"Name,With,Comma", "Alice & Bob", "Test (1)"};

    QVERIFY(store.save(data));

    PoolData loaded = store.load();
    QCOMPARE(loaded.initPool, data.initPool);
}

void TestDataStore::testEmptyList()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DataStore store(tempDir.path());

    PoolData data;
    data.initPool.clear();
    data.pool.clear();

    QVERIFY(store.save(data));

    PoolData loaded = store.load();
    QVERIFY(loaded.initPool.isEmpty());
    QVERIFY(loaded.pool.isEmpty());
    QCOMPARE(loaded.version, 1);
}

void TestDataStore::testFileNotExist()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DataStore store(tempDir.path());

    PoolData loaded = store.load();
    QVERIFY(loaded.initPool.isEmpty());
    QVERIFY(loaded.pool.isEmpty());
    QCOMPARE(loaded.version, 1);
}

void TestDataStore::testCorruptedFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QFile badFile(tempDir.path() + "/drawer_data.json");
    QVERIFY(badFile.open(QIODevice::WriteOnly | QIODevice::Text));
    badFile.write("not json at all {{{");
    badFile.close();

    DataStore store(tempDir.path());
    PoolData loaded = store.load();
    QVERIFY(loaded.initPool.isEmpty());
    QVERIFY(loaded.pool.isEmpty());
    QCOMPARE(loaded.version, 1);
}

QTEST_MAIN(TestDataStore)
#include "test_data_store.moc"
