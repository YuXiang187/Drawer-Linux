#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

#include "data/encryptor.h"
#include "data/group_store.h"
#include "data/key_value_store.h"

namespace {

void writeConfig(const QString &path, const QString &content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QTextStream out(&file);
    out << content;
}

} // namespace

class TestGroupStore : public QObject
{
    Q_OBJECT

private slots:
    void testMigrateLegacyList();
    void testMigrateSeedsSampleNames();
    void testMigratePreservesEmptyList();
    void testMigrateOnlyOnce();
    void testAddGroup();
    void testAddGroupSkipsTakenIds();
    void testSetInitRebuildsPool();
    void testRenameGroup();
    void testRemoveGroup();
    void testActiveGroupFallsBack();
    void testFindByName();
    void testPersistence();
};

void TestGroupStore::testMigrateLegacyList()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Encryptor encryptor;
    const QString path = tempDir.path() + "/Drawer.config";
    writeConfig(path, QString("initPool:%1\npool:%2\n")
                          .arg(encryptor.encrypt("Alice,Bob"),
                               encryptor.encrypt("Bob,Alice")));

    KeyValueStore store(path);
    QVERIFY(store.load());

    GroupStore groups(&store);
    QVERIFY(groups.migrateLegacyList());

    QCOMPARE(groups.ids(), QStringList({"g0"}));
    QCOMPARE(groups.activeId(), QString("g0"));
    QCOMPARE(groups.name("g0"), GroupStore::DefaultGroupName);
    QCOMPARE(groups.init("g0"), QStringList({"Alice", "Bob"}));
    QCOMPARE(groups.pool("g0"), QStringList({"Bob", "Alice"}));
    QCOMPARE(store.value("NextId"), QString("1"));

    // The legacy keys are dropped once their content has been migrated.
    QVERIFY(!store.contains("initPool"));
    QVERIFY(!store.contains("pool"));
}

void TestGroupStore::testMigrateSeedsSampleNames()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    QVERIFY(!store.load());

    GroupStore groups(&store);
    QVERIFY(groups.migrateLegacyList());

    QCOMPARE(groups.init("g0"), QStringList({"Item1", "Item2", "Item3", "Item4", "Item5"}));
    QCOMPARE(groups.pool("g0"), groups.init("g0"));
}

void TestGroupStore::testMigratePreservesEmptyList()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString path = tempDir.path() + "/Drawer.config";
    writeConfig(path, "initPool:\npool:\n");

    KeyValueStore store(path);
    QVERIFY(store.load());

    GroupStore groups(&store);
    QVERIFY(groups.migrateLegacyList());

    QCOMPARE(groups.count(), 1);
    QVERIFY(groups.init("g0").isEmpty());
    QVERIFY(groups.pool("g0").isEmpty());
}

void TestGroupStore::testMigrateOnlyOnce()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();

    GroupStore groups(&store);
    QVERIFY(groups.migrateLegacyList());
    QVERIFY(!groups.migrateLegacyList());

    QCOMPARE(groups.ids(), QStringList({"g0"}));
}

void TestGroupStore::testAddGroup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();

    GroupStore groups(&store);
    groups.migrateLegacyList();

    const QString id = groups.add("Class A");
    QCOMPARE(id, QString("g1"));
    QCOMPARE(groups.ids(), QStringList({"g0", "g1"}));
    QCOMPARE(groups.name("g1"), QString("Class A"));
    QCOMPARE(groups.init("g1"), QStringList({"Item1", "Item2", "Item3", "Item4", "Item5"}));
    QCOMPARE(groups.pool("g1"), groups.init("g1"));
    QCOMPARE(store.value("NextId"), QString("2"));

    // Ids are never reused, even after a group in between was removed.
    QCOMPARE(groups.add("Class B"), QString("g2"));
    QVERIFY(groups.remove("g1"));
    QCOMPARE(groups.add("Class C"), QString("g3"));
}

void TestGroupStore::testAddGroupSkipsTakenIds()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();
    store.setValue("Groups", "g0,g1");
    store.setValue("NextId", "1");

    // A hand-edited NextId must not make the new group overwrite an existing
    // one.
    GroupStore groups(&store);
    QCOMPARE(groups.add("Third"), QString("g2"));
    QCOMPARE(groups.ids(), QStringList({"g0", "g1", "g2"}));
}

void TestGroupStore::testSetInitRebuildsPool()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();

    GroupStore groups(&store);
    groups.migrateLegacyList();
    groups.setInit("g0", QStringList{"A", "B", "C"});
    groups.setPool("g0", QStringList{"C", "A", "B"});

    // A new name goes to the front, the drawn history keeps its order.
    groups.setInit("g0", QStringList{"A", "B", "C", "D"});
    QCOMPARE(groups.init("g0"), QStringList({"A", "B", "C", "D"}));
    QCOMPARE(groups.pool("g0"), QStringList({"D", "C", "A", "B"}));

    // Removing names drops them from the pool but keeps the relative order.
    groups.setInit("g0", QStringList{"A", "C"});
    QCOMPARE(groups.pool("g0"), QStringList({"C", "A"}));
}

void TestGroupStore::testRenameGroup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();

    GroupStore groups(&store);
    groups.migrateLegacyList();

    QVERIFY(groups.rename("g0", "Main"));
    QCOMPARE(groups.name("g0"), QString("Main"));
    QVERIFY(!groups.rename("g9", "Missing"));
}

void TestGroupStore::testRemoveGroup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();

    GroupStore groups(&store);
    groups.migrateLegacyList();
    const QString second = groups.add("Second");
    QVERIFY(groups.setActiveId(second));

    QVERIFY(groups.remove(second));
    QCOMPARE(groups.ids(), QStringList({"g0"}));
    QCOMPARE(groups.activeId(), QString("g0"));
    QVERIFY(!store.contains("name@" + second));
    QVERIFY(!store.contains("initPool@" + second));
    QVERIFY(!store.contains("pool@" + second));

    // The last group cannot be removed.
    QVERIFY(!groups.remove("g0"));
    QCOMPARE(groups.count(), 1);
    QVERIFY(!groups.remove("g9"));
}

void TestGroupStore::testActiveGroupFallsBack()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();
    store.setValue("Groups", "g0,g1");
    store.setValue("CurrentGroup", "g9");

    GroupStore groups(&store);

    // A stale CurrentGroup must not leave the application without a group.
    QCOMPARE(groups.activeId(), QString("g0"));
    QVERIFY(!groups.setActiveId("g9"));
    QCOMPARE(groups.activeId(), QString("g0"));
    QVERIFY(groups.setActiveId("g1"));
    QCOMPARE(groups.activeId(), QString("g1"));
}

void TestGroupStore::testFindByName()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    KeyValueStore store(tempDir.path() + "/Drawer.config");
    store.load();

    GroupStore groups(&store);
    groups.migrateLegacyList();
    groups.add("Class A");

    QCOMPARE(groups.findByName("Class A"), QString("g1"));
    QCOMPARE(groups.findByName("  class a  "), QString("g1"));
    QVERIFY(groups.findByName("Missing").isEmpty());
}

void TestGroupStore::testPersistence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString path = tempDir.path() + "/Drawer.config";

    {
        KeyValueStore store(path);
        store.load();

        GroupStore groups(&store);
        groups.migrateLegacyList();
        const QString second = groups.add("Second");
        groups.setInit(second, QStringList{"X", "Y"});
        groups.setActiveId(second);
        QVERIFY(store.save());
    }

    KeyValueStore store(path);
    QVERIFY(store.load());

    GroupStore groups(&store);
    QVERIFY(!groups.migrateLegacyList());
    QCOMPARE(groups.ids(), QStringList({"g0", "g1"}));
    QCOMPARE(groups.activeId(), QString("g1"));
    QCOMPARE(groups.name("g1"), QString("Second"));
    QCOMPARE(groups.init("g1"), QStringList({"X", "Y"}));
    QCOMPARE(groups.pool("g1"), QStringList({"X", "Y"}));
}

QTEST_MAIN(TestGroupStore)
#include "test_group_store.moc"
