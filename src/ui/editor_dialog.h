#pragma once

#include <QDialog>
#include <QStringList>

#include <functional>

class ApplicationController;
class QListView;
class QPlainTextEdit;
class QLineEdit;
class QComboBox;
class QStringListModel;
class QLabel;
class QAction;

class EditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditorDialog(ApplicationController *controller, QWidget *parent = nullptr);

private slots:
    void onImport();
    void onExport();
    void onFind();
    void onClear();
    void onChangePassword();
    void onAdd();
    void onRemove();
    void onApply();
    void onTextChanged();
    void onSelectionChanged();
    void onFindNext();
    void onGroupSelected(int index);
    void onGroupAdd();
    void onGroupRename();
    void onGroupDelete();

private:
    void loadGroup(const QString &groupId);
    void refreshGroupComboBox();
    void syncTextToModel();
    void updateStatus();
    bool isListModified() const;
    void withDiscardConfirmation(const std::function<void()> &action);
    bool validateGroupName(const QString &name, const QString &excludeId);

    ApplicationController *m_controller;
    QStringListModel *m_model;
    QListView *m_listView;
    QPlainTextEdit *m_textEdit;
    QLineEdit *m_addEdit;
    QLineEdit *m_findEdit;
    QComboBox *m_groupCombo;
    QLabel *m_statusLabel;
    QStringList m_groupIds;
    QString m_currentGroupId;
    QString m_loadedContent;
    int m_findIndex;
};
