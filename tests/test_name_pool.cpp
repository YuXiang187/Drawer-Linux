#include <QTest>
#include <QTemporaryDir>
#include <QStringList>
#include "core/name_pool.h"
#include "data/config_manager.h"

class TestNamePool : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testSetNames();
    void testSetState();
    void testAddName();
    void testRemoveName();
    void testDraw();
    void testDrawReducesRemaining();
    void testAutoResetWhenEmpty();
    void testEmptyPool();
    void testSingleName();
    void testDuplicateNames();
    void testDrawnNamesRemainRemovedAfterRestart();
    void testEmptyPoolStateAfterRestart();
};

void TestNamePool::initTestCase()
{
    // Nothing to initialize
}

void TestNamePool::testSetNames()
{
    NamePool pool;
    QStringList names = {"Alice", "Bob", "Carol"};
    pool.setNames(names);

    QCOMPARE(pool.size(), 3);
    QCOMPARE(pool.remainingSize(), 3);
    QCOMPARE(pool.isEmpty(), false);
    QCOMPARE(pool.names(), names);
}

void TestNamePool::testSetState()
{
    NamePool pool;
    pool.setState({"Alice", "Bob", "Carol"}, {"Alice", "Carol"});

    QCOMPARE(pool.names(), QStringList({"Alice", "Bob", "Carol"}));
    QCOMPARE(pool.remainingNames(), QStringList({"Alice", "Carol"}));
    QCOMPARE(pool.size(), 3);
    QCOMPARE(pool.remainingSize(), 2);
    QVERIFY(!pool.remainingNames().contains("Bob"));
}

void TestNamePool::testAddName()
{
    NamePool pool;
    pool.setNames({"Alice", "Bob"});

    pool.addName("Carol");
    QCOMPARE(pool.size(), 3);
    QCOMPARE(pool.remainingSize(), 3);
    QVERIFY(pool.hasName("Carol"));
}

void TestNamePool::testRemoveName()
{
    NamePool pool;
    pool.setNames({"Alice", "Bob", "Carol"});

    pool.removeName("Bob");
    QCOMPARE(pool.size(), 2);
    QCOMPARE(pool.remainingSize(), 2);
    QVERIFY(!pool.hasName("Bob"));
}

void TestNamePool::testDraw()
{
    NamePool pool;
    QStringList names = {"Alice", "Bob", "Carol"};
    pool.setNames(names);

    QString result = pool.draw();
    QVERIFY(names.contains(result));
}

void TestNamePool::testDrawReducesRemaining()
{
    NamePool pool;
    pool.setNames({"Alice", "Bob", "Carol"});

    QCOMPARE(pool.remainingSize(), 3);
    pool.draw();
    QCOMPARE(pool.remainingSize(), 2);
    pool.draw();
    QCOMPARE(pool.remainingSize(), 1);
    pool.draw();
    QCOMPARE(pool.remainingSize(), 0);
}

void TestNamePool::testAutoResetWhenEmpty()
{
    NamePool pool;
    pool.setNames({"Alice", "Bob"});

    pool.draw(); // remaining = 1
    pool.draw(); // remaining = 0
    QCOMPARE(pool.remainingSize(), 0);

    // Auto-reset happens on next draw
    QString result = pool.draw();
    QVERIFY(!result.isEmpty());
    QCOMPARE(pool.remainingSize(), 1);
}

void TestNamePool::testEmptyPool()
{
    NamePool pool;
    pool.setNames({});

    QCOMPARE(pool.isEmpty(), true);
    QCOMPARE(pool.size(), 0);
    QCOMPARE(pool.remainingSize(), 0);

    QString result = pool.draw();
    QCOMPARE(result, QString());
}

void TestNamePool::testSingleName()
{
    NamePool pool;
    pool.setNames({"Alice"});

    QCOMPARE(pool.size(), 1);
    QCOMPARE(pool.remainingSize(), 1);

    QString result = pool.draw();
    QCOMPARE(result, QString("Alice"));
    QCOMPARE(pool.remainingSize(), 0);

    // Auto-reset on next draw
    result = pool.draw();
    QCOMPARE(result, QString("Alice"));
    QCOMPARE(pool.remainingSize(), 0);
}

void TestNamePool::testDuplicateNames()
{
    NamePool pool;
    pool.setNames({"Alice", "Alice", "Bob"});

    QCOMPARE(pool.size(), 3);
    QCOMPARE(pool.remainingSize(), 3);

    // Draw all three
    QString r1 = pool.draw();
    QString r2 = pool.draw();
    QString r3 = pool.draw();

    // All results should be valid names
    QVERIFY(!r1.isEmpty());
    QVERIFY(!r2.isEmpty());
    QVERIFY(!r3.isEmpty());

    QCOMPARE(pool.remainingSize(), 0);
}

void TestNamePool::testDrawnNamesRemainRemovedAfterRestart()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString drawn;
    {
        ConfigManager config(tempDir.path());
        config.setInitPool({"Alice", "Bob", "Carol"});
        config.setPool({"Alice", "Bob", "Carol"});
        config.sync();

        NamePool pool;
        pool.setState(config.initPool(), config.pool());
        drawn = pool.draw();
        QVERIFY(!drawn.isEmpty());

        config.setInitPool(pool.names());
        config.setPool(pool.remainingNames());
        config.sync();
    }

    {
        ConfigManager config(tempDir.path());
        QVERIFY(config.fileExists());

        NamePool pool;
        pool.setState(config.initPool(), config.pool());

        QCOMPARE(pool.names(), QStringList({"Alice", "Bob", "Carol"}));
        QCOMPARE(pool.remainingSize(), 2);
        QVERIFY(!pool.remainingNames().contains(drawn));
    }
}

void TestNamePool::testEmptyPoolStateAfterRestart()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    {
        ConfigManager config(tempDir.path());
        config.setInitPool({});
        config.setPool({});
        config.sync();
    }

    ConfigManager config(tempDir.path());
    QVERIFY(config.fileExists());
    QVERIFY(config.initPool().isEmpty());
    QVERIFY(config.pool().isEmpty());

    NamePool pool;
    pool.setState(config.initPool(), config.pool());
    QVERIFY(pool.isEmpty());
    QCOMPARE(pool.remainingSize(), 0);
    QCOMPARE(pool.draw(), QString());
}

QTEST_MAIN(TestNamePool)
#include "test_name_pool.moc"
