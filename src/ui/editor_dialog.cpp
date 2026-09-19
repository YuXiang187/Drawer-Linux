#include "editor_dialog.h"
#include "application/application_controller.h"
#include "password_dialog.h"

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStringListModel>
#include <QStyle>
#include <QTextStream>
#include <QToolBar>
#include <QVBoxLayout>

EditorDialog::EditorDialog(ApplicationController *controller, QWidget *parent)
    : QDialog(parent)
    , m_controller(controller)
    , m_findIndex(0)
{
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    setWindowTitle("YuXiang Drawer");
    resize(800, 500);

    auto *mainLayout = new QVBoxLayout(this);

    // Toolbar
    auto *toolBar = new QToolBar(this);
    mainLayout->addWidget(toolBar);

    QAction *actImport = toolBar->addAction("导入");
    QAction *actExport = toolBar->addAction("导出");
    toolBar->addSeparator();

    m_findEdit = new QLineEdit(this);
    m_findEdit->setPlaceholderText("查找...");
    m_findEdit->setFixedWidth(150);
    toolBar->addWidget(m_findEdit);

    QAction *actFind = toolBar->addAction("查找");
    toolBar->addSeparator();

    QAction *actClear = toolBar->addAction("清空");
    QAction *actPassword = toolBar->addAction("修改密码");
    toolBar->addSeparator();

    QAction *actApply = toolBar->addAction("应用");
    QAction *actCancel = toolBar->addAction("取消");

    // Splitter
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter, 1);

    m_model = new QStringListModel(this);
    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    splitter->addWidget(m_listView);

    m_textEdit = new QPlainTextEdit(this);
    splitter->addWidget(m_textEdit);
    splitter->setSizes({250, 550});

    // Bottom
    auto *bottomLayout = new QHBoxLayout();
    mainLayout->addLayout(bottomLayout);

    m_addEdit = new QLineEdit(this);
    m_addEdit->setPlaceholderText("输入名称...");
    bottomLayout->addWidget(m_addEdit, 1);

    auto *btnAdd = new QPushButton("添加", this);
    bottomLayout->addWidget(btnAdd);

    auto *btnRemove = new QPushButton("删除", this);
    bottomLayout->addWidget(btnRemove);

    m_statusLabel = new QLabel("总数：0  选中：0", this);
    mainLayout->addWidget(m_statusLabel);

    // Load data
    loadNames();

    // Connections
    connect(actImport, &QAction::triggered, this, &EditorDialog::onImport);
    connect(actExport, &QAction::triggered, this, &EditorDialog::onExport);
    connect(actFind, &QAction::triggered, this, &EditorDialog::onFind);
    connect(actClear, &QAction::triggered, this, &EditorDialog::onClear);
    connect(actPassword, &QAction::triggered, this, &EditorDialog::onChangePassword);
    connect(actApply, &QAction::triggered, this, &EditorDialog::onApply);
    connect(actCancel, &QAction::triggered, this, &QDialog::reject);
    connect(btnAdd, &QPushButton::clicked, this, &EditorDialog::onAdd);
    connect(btnRemove, &QPushButton::clicked, this, &EditorDialog::onRemove);
    connect(m_addEdit, &QLineEdit::returnPressed, this, &EditorDialog::onAdd);
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, &EditorDialog::onTextChanged);
    connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &EditorDialog::onSelectionChanged);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &EditorDialog::onFindNext);
}

void EditorDialog::loadNames()
{
    const QStringList names = m_controller->currentNames();
    m_model->setStringList(names);
    m_textEdit->setPlainText(names.join(","));
    updateStatus();
}

void EditorDialog::syncTextToModel()
{
    QString text = m_textEdit->toPlainText();
    text.replace(" ", "").replace("\t", "").replace("\n", ",").replace("\r", "");
    const QString &cleaned = text;
    m_textEdit->blockSignals(true);
    if (m_textEdit->toPlainText() != cleaned) {
        m_textEdit->setPlainText(cleaned);
        QTextCursor cursor = m_textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_textEdit->setTextCursor(cursor);
    }
    m_textEdit->blockSignals(false);

    const QStringList list = cleaned.split(',', Qt::SkipEmptyParts);
    m_model->setStringList(list);
    updateStatus();
}

void EditorDialog::updateStatus()
{
    const int total = m_model->rowCount();
    const int selected = m_listView->selectionModel()->selectedIndexes().size();
    m_statusLabel->setText(QString("总数：%1  选中：%2").arg(total).arg(selected));
}

void EditorDialog::onImport()
{
    QString path = QFileDialog::getOpenFileName(this, "导入", QString(), "文本文档 (*.txt)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString content = in.readAll().trimmed();
    file.close();

    if (content.isEmpty()) {
        QMessageBox::warning(this, "导入", "文件内容为空。");
        return;
    }

    // Replace newlines with commas
    content = content.replace('\n', ',').replace('\r', "");
    m_textEdit->setPlainText(content);
    syncTextToModel();
}

void EditorDialog::onExport()
{
    const QStringList list = m_model->stringList();
    if (list.isEmpty()) {
        QMessageBox::warning(this, "导出", "列表为空。");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this, "导出", "names.txt", "文本文档 (*.txt)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << list.join(",");
    file.close();
}

void EditorDialog::onFind()
{
    onFindNext();
}

void EditorDialog::onFindNext()
{
    const QString text = m_findEdit->text();
    if (text.isEmpty())
        return;

    const QString all = m_textEdit->toPlainText();
    int idx = all.indexOf(text, m_findIndex, Qt::CaseInsensitive);
    if (idx == -1) {
        idx = all.indexOf(text, 0, Qt::CaseInsensitive);
        if (idx == -1) {
            QMessageBox::information(this, "查找", "未找到。");
            return;
        }
    }

    QTextCursor cursor = m_textEdit->textCursor();
    cursor.setPosition(idx);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, text.length());
    m_textEdit->setTextCursor(cursor);
    m_findIndex = idx + text.length();
}

void EditorDialog::onClear()
{
    int ret = QMessageBox::question(this, "清空", "是否清空所有内容？");
    if (ret == QMessageBox::Yes) {
        m_textEdit->clear();
        syncTextToModel();
    }
}

void EditorDialog::onChangePassword()
{
    PasswordDialog dlg1("修改密码", "原密码：", this);
    if (dlg1.exec() != QDialog::Accepted)
        return;

    if (!m_controller->verifyPassword(dlg1.password())) {
        QMessageBox::critical(this, "修改密码", "密码错误。");
        return;
    }

    PasswordDialog dlg2("修改密码", "新密码：", this);
    if (dlg2.exec() != QDialog::Accepted)
        return;

    if (dlg2.password().isEmpty()) {
        QMessageBox::warning(this, "修改密码", "新密码不能为空。");
        return;
    }

    m_controller->changePassword(dlg1.password(), dlg2.password());
    QMessageBox::information(this, "修改密码", "密码已更改。");
}

void EditorDialog::onAdd()
{
    QString name = m_addEdit->text().trimmed();
    if (name.isEmpty())
        return;

    if (name.contains(',')) {
        QMessageBox::warning(this, "添加", "名称不能包含逗号。");
        return;
    }

    QStringList list = m_model->stringList();
    list.append(name);
    m_model->setStringList(list);
    m_textEdit->setPlainText(list.join(","));
    m_addEdit->clear();
    updateStatus();
}

void EditorDialog::onRemove()
{
    const QModelIndexList selected = m_listView->selectionModel()->selectedIndexes();
    if (selected.isEmpty())
        return;

    QStringList list = m_model->stringList();
    // Remove from end to avoid index shift
    QList<int> rows;
    for (const QModelIndex &idx : selected)
        rows.append(idx.row());
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows)
        list.removeAt(row);

    m_model->setStringList(list);
    m_textEdit->setPlainText(list.join(","));
    updateStatus();
}

void EditorDialog::onApply()
{
    QStringList list = m_model->stringList();
    if (list.isEmpty()) {
        QMessageBox::warning(this, "应用", "列表不能为空。");
        return;
    }

    m_controller->setNames(list);
    QMessageBox::information(this, "应用", "已保存。");
    m_controller->showStatistics();
    accept();
}

void EditorDialog::onTextChanged()
{
    syncTextToModel();
}

void EditorDialog::onSelectionChanged()
{
    updateStatus();
}
