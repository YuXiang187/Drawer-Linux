#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include "platform/single_instance.h"

class TestSingleInstance : public QObject
{
    Q_OBJECT

private slots:
    void testLockSuccess();
    void testSecondLockFails();
    void testUnlockThenLock();
};

void TestSingleInstance::testLockSuccess()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SingleInstance si(tempDir.path());
    QVERIFY(si.tryLock());
    QVERIFY(si.isLocked());
    si.unlock();
}

void TestSingleInstance::testSecondLockFails()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SingleInstance si1(tempDir.path());
    QVERIFY(si1.tryLock());

    SingleInstance si2(tempDir.path());
    QVERIFY(!si2.tryLock());
    QVERIFY(!si2.isLocked());

    si1.unlock();
}

void TestSingleInstance::testUnlockThenLock()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SingleInstance si1(tempDir.path());
    QVERIFY(si1.tryLock());
    si1.unlock();

    SingleInstance si2(tempDir.path());
    QVERIFY(si2.tryLock());
    QVERIFY(si2.isLocked());
    si2.unlock();
}

QTEST_MAIN(TestSingleInstance)
#include "test_single_instance.moc"
