#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "platform/auto_launch.h"

class TestAutoLaunch : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultDisabled();
    void testEnableCreatesDesktopFile();
    void testDesktopFileContent();
    void testDisableRemovesFile();
};

void TestAutoLaunch::testDefaultDisabled()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    AutoLaunch al(tempDir.path());
    QVERIFY(!al.isEnabled());
}

void TestAutoLaunch::testEnableCreatesDesktopFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    AutoLaunch al(tempDir.path());
    QVERIFY(al.enable());
    QVERIFY(al.isEnabled());
    QVERIFY(QFile::exists(tempDir.path() + "/autostart/drawer.desktop"));
}

void TestAutoLaunch::testDesktopFileContent()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    AutoLaunch al(tempDir.path());
    QVERIFY(al.enable());

    QFile file(tempDir.path() + "/autostart/drawer.desktop");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    QString content = in.readAll();

    QVERIFY(content.contains("[Desktop Entry]"));
    QVERIFY(content.contains("Name=YuXiang Drawer"));
    QVERIFY(content.contains("Type=Application"));
    QVERIFY(content.contains("Terminal=false"));
    QVERIFY(content.contains("Exec="));
}

void TestAutoLaunch::testDisableRemovesFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    AutoLaunch al(tempDir.path());
    QVERIFY(al.enable());
    QVERIFY(al.isEnabled());

    QVERIFY(al.disable());
    QVERIFY(!al.isEnabled());
    QVERIFY(!QFile::exists(tempDir.path() + "/autostart/drawer.desktop"));
}

QTEST_MAIN(TestAutoLaunch)
#include "test_auto_launch.moc"
