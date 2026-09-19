#pragma once

#include <QDialog>
#include <QStringList>

class ApplicationController;
class QListView;
class QPlainTextEdit;
class QLineEdit;
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

private:
    void loadNames();
    void syncTextToModel();
    void updateStatus();

    ApplicationController *m_controller;
    QStringListModel *m_model;
    QListView *m_listView;
    QPlainTextEdit *m_textEdit;
    QLineEdit *m_addEdit;
    QLineEdit *m_findEdit;
    QLabel *m_statusLabel;
    int m_findIndex;
};
