#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>

// Shows the complete name list in a scrollable, selectable text area; a
// message box would cut a large list off with an ellipsis.
class StatsDialog : public QDialog
{
    Q_OBJECT
public:
    // Brings the open window back instead of opening a second one.
    static void showStats(QWidget *parent, const QString &groupName, const QStringList &names);

private:
    StatsDialog(const QString &groupName, const QStringList &names, QWidget *parent);
};
