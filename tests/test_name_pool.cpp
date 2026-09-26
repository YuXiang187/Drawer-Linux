#include <QTest>
#include "core/name_pool.h"

class TestNamePool : public QObject
{
    Q_OBJECT

private slots:
    void testDrawReturnsNameFromInitPool();
    void testPoolKeepsAllNames();
    void testDrawnNameMovesToEnd();
    void testInitPoolUntouchedByDraw();
    void testGaussianPrefersFront();
    void testSetNamesPreservesHistory();
    void testSetNamesPrependsNewNames();
    void testSetStateRepairsIncompletePool();
    void testEmptyPool();
    void testSingleName();
};

void TestNamePool::testDrawReturnsNameFromInitPool()
{
    NamePool pool;
    const QStringList names{"Alice", "Bob", "Carol", "Dave"};
    pool.setNames(names);

    for (int i = 0; i < 200; ++i)
        QVERIFY(names.contains(pool.draw()));
}

void TestNamePool::testPoolKeepsAllNames()
{
    NamePool pool;
    const QStringList names{"Alice", "Bob", "Carol", "Dave"};
    pool.setNames(names);

    // Unlike a draw-without-replacement model, the Gaussian pool must still
    // hold every name after any number of draws.
    for (int i = 0; i < 100; ++i) {
        pool.draw();
        const QStringList current = pool.pool();
        QCOMPARE(current.size(), names.size());
        for (const QString &name : names)
            QVERIFY(current.contains(name));
    }
}

void TestNamePool::testDrawnNameMovesToEnd()
{
    NamePool pool;
    pool.setNames(QStringList{"Alice", "Bob", "Carol", "Dave"});

    for (int i = 0; i < 50; ++i) {
        const QString drawn = pool.draw();
        QCOMPARE(pool.pool().last(), drawn);
        QCOMPARE(pool.pool().size(), 4);
    }
}

void TestNamePool::testInitPoolUntouchedByDraw()
{
    NamePool pool;
    const QStringList names{"Alice", "Bob", "Carol", "Dave"};
    pool.setNames(names);

    for (int i = 0; i < 100; ++i)
        pool.draw();

    // The init pool keeps the user-edited order and is never reordered.
    QCOMPARE(pool.names(), names);
}

void TestNamePool::testGaussianPrefersFront()
{
    // samples index = |Normal(0, len / 3.0)|, so the front of the pool
    // must be picked far more often than the end.
    const QStringList names{"N0", "N1", "N2", "N3", "N4",
                            "N5", "N6", "N7", "N8", "N9"};
    QVector<int> counts(names.size(), 0);

    NamePool pool;
    const int iterations = 4000;
    for (int i = 0; i < iterations; ++i) {
        // Reset the history before each draw so every sample is taken from
        // the same ordering.
        pool.setState(names, names);
        const QString drawn = pool.draw();
        counts[names.indexOf(drawn)]++;
    }

    // With len = 10 and factor 3.0, index 0 is expected in ~23.6% of the
    // draws and index 9 in ~0.4%.
    QVERIFY(counts.first() > counts.last() * 5);
    QVERIFY(counts.first() * 5 > iterations);
}

void TestNamePool::testSetNamesPreservesHistory()
{
    NamePool pool;
    pool.setNames(QStringList{"Alice", "Bob", "Carol"});

    // Simulate a history where Carol was drawn most recently.
    pool.setState(QStringList{"Alice", "Bob", "Carol"},
                  QStringList{"Alice", "Bob", "Carol"});
    pool.draw(); // moves whichever name was picked to the end

    const QStringList history = pool.pool();
    pool.setNames(QStringList{"Alice", "Bob", "Carol"});
    QCOMPARE(pool.pool(), history);
}

void TestNamePool::testSetNamesPrependsNewNames()
{
    NamePool pool;
    pool.setNames(QStringList{"Alice", "Bob", "Carol"});
    pool.setState(pool.names(), QStringList{"Bob", "Carol", "Alice"});

    // Remove a name: it disappears from the pool, relative history stays.
    pool.setNames(QStringList{"Bob", "Carol"});
    QCOMPARE(pool.pool(), QStringList({"Bob", "Carol"}));
    QCOMPARE(pool.names(), QStringList({"Bob", "Carol"}));

    // Add a new name: it goes to the front (never drawn = highest chance),
    // the existing history order is preserved.
    pool.setNames(QStringList{"Dave", "Bob", "Carol"});
    QCOMPARE(pool.pool(), QStringList({"Dave", "Bob", "Carol"}));
    QCOMPARE(pool.names(), QStringList({"Dave", "Bob", "Carol"}));
}

void TestNamePool::testSetStateRepairsIncompletePool()
{
    NamePool pool;
    // Configs written by the old draw-without-replacement model stored only
    // the remaining names in the pool; loading must restore the full list.
    pool.setState(QStringList{"Alice", "Bob", "Carol"}, QStringList{"Carol"});

    const QStringList restored = pool.pool();
    QCOMPARE(restored.size(), 3);
    for (const QString &name : QStringList{"Alice", "Bob", "Carol"})
        QVERIFY(restored.contains(name));
    // The persisted history entry keeps its place at the end.
    QCOMPARE(restored.last(), QString("Carol"));

    QVERIFY(!pool.draw().isEmpty());
}

void TestNamePool::testEmptyPool()
{
    NamePool pool;
    QVERIFY(pool.isEmpty());
    QVERIFY(pool.draw().isEmpty());

    pool.setNames(QStringList{});
    QVERIFY(pool.draw().isEmpty());
    QVERIFY(pool.pool().isEmpty());
}

void TestNamePool::testSingleName()
{
    NamePool pool;
    pool.setNames(QStringList{"Alice"});

    for (int i = 0; i < 20; ++i) {
        QCOMPARE(pool.draw(), QString("Alice"));
        QCOMPARE(pool.pool(), QStringList({"Alice"}));
    }
}

QTEST_MAIN(TestNamePool)
#include "test_name_pool.moc"
