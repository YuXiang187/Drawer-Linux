#include <QTest>
#include "core/random_engine.h"

class TestRandomEngine : public QObject
{
    Q_OBJECT

private slots:
    void testValueWithinRange();
    void testInclusiveBounds();
    void testInvalidRange();
    void testRepeatedConstruction();
};

void TestRandomEngine::testValueWithinRange()
{
    RandomEngine engine;
    for (int i = 0; i < 1000; ++i) {
        const int value = engine.nextInt(0, 9);
        QVERIFY(value >= 0);
        QVERIFY(value <= 9);
    }
}

void TestRandomEngine::testInclusiveBounds()
{
    RandomEngine engine;
    bool sawMin = false;
    bool sawMax = false;

    for (int i = 0; i < 1000; ++i) {
        const int value = engine.nextInt(1, 2);
        sawMin = sawMin || (value == 1);
        sawMax = sawMax || (value == 2);
    }

    QVERIFY(sawMin);
    QVERIFY(sawMax);
}

void TestRandomEngine::testInvalidRange()
{
    RandomEngine engine;
    QCOMPARE(engine.nextInt(5, 5), 5);
    QCOMPARE(engine.nextInt(7, 3), 7);
}

void TestRandomEngine::testRepeatedConstruction()
{
    for (int i = 0; i < 100; ++i) {
        RandomEngine engine;
        QCOMPARE(engine.nextInt(1, 1), 1);
    }
}

QTEST_MAIN(TestRandomEngine)
#include "test_random_engine.moc"
