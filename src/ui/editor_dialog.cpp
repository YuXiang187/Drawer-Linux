#include "editor_dialog.h"
#include "application/application_controller.h"
#include "password_dialog.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
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
#include <QSignalBlocker>
#include <QSplitter>
#include <QStringListModel>
#include <QStyle>
#include <QTextStream>
#include <QToolBar>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>

namespace {

// Group names are limited to 30 characters, like in the Windows version.
constexpr int kMaxGroupNameLength = 30;

bool promptGroupName(QWidget *parent, const QString &title, const QString &label,
                     const QString &initialText, QString *name)
{
    QInputDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue(initialText);
    dialog.setOkButtonText("确定");
    dialog.setCancelButtonText("取消");

    if (QLineEdit *edit = dialog.findChild<QLineEdit *>())
        edit->setMaxLength(kMaxGroupNameLength);

    if (dialog.exec() != QDialog::Accepted)
        return false;

    *name = dialog.textValue();
    return true;
}

} // namespace

EditorDialog::EditorDialog(ApplicationController *controller, QWidget *parent)
    : QDialog(parent)
    , m_controller(controller)
    , m_findIndex(0)
{
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    setWindowTitle("YuXiang Drawer");
    resize(800, 540);

    auto *mainLayout = new QVBoxLayout(this);

    // Group toolbar
    auto *groupBar = new QToolBar(this);
    mainLayout->addWidget(groupBar);

    auto *groupLabel = new QLabel("分组", groupBar);
    groupLabel->setContentsMargins(4, 0, 8, 0);
    groupBar->addWidget(groupLabel);

    m_groupCombo = new QComboBox(groupBar);
    m_groupCombo->setFixedWidth(200);
    groupBar->addWidget(m_groupCombo);
    groupBar->addSeparator();

    QAction *actGroupAdd = groupBar->addAction("添加分组");
    QAction *actGroupRename = groupBar->addAction("重命名分组");
    QAction *actGroupDelete = groupBar->addAction("删除分组");

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
    QAction *actPassword = toolBar->addAction("密码");
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
    loadGroup(m_controller->activeGroupId());

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

    connect(m_groupCombo, qOverload<int>(&QComboBox::activated),
            this, &EditorDialog::onGroupSelected);
    connect(actGroupAdd, &QAction::triggered, this, &EditorDialog::onGroupAdd);
    connect(actGroupRename, &QAction::triggered, this, &EditorDialog::onGroupRename);
    connect(actGroupDelete, &QAction::triggered, this, &EditorDialog::onGroupDelete);
}

void EditorDialog::loadGroup(const QString &groupId)
{
    QString id = groupId;
    if (id.isEmpty())
        id = m_controller->activeGroupId();
    if (id.isEmpty()) {
        // Cannot happen: the config always contains at least one group.
        id = m_controller->addGroup(QString());
    }

    m_currentGroupId = id;
    m_loadedContent = m_controller->groupInitNames(id).join(',');

    {
        // Fill the list model below instead of relying on textChanged, so the
        // dialog is usable before the signals are connected.
        const QSignalBlocker blocker(m_textEdit);
        m_textEdit->setPlainText(m_loadedContent);
    }
    syncTextToModel();

    setWindowTitle(QString("YuXiang Drawer - %1").arg(m_controller->groupName(id)));
    refreshGroupComboBox();
}

void EditorDialog::refreshGroupComboBox()
{
    // The edited group can disappear when it is removed, fall back to the
    // active one.
    if (!m_controller->groupIds().contains(m_currentGroupId)) {
        loadGroup(m_controller->activeGroupId());
        return;
    }

    m_groupIds = m_controller->groupIds();

    const QSignalBlocker blocker(m_groupCombo);
    m_groupCombo->clear();
    for (const QString &id : m_groupIds)
        m_groupCombo->addItem(m_controller->groupName(id));

    const int index = m_groupIds.indexOf(m_currentGroupId);
    if (index >= 0)
        m_groupCombo->setCurrentIndex(index);
}

bool EditorDialog::isListModified() const
{
    return m_textEdit->toPlainText() != m_loadedContent;
}

void EditorDialog::withDiscardConfirmation(const std::function<void()> &action)
{
    if (!isListModified()) {
        action();
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, "分组",
        QString("是否丢弃对“%1”的修改？").arg(m_controller->groupName(m_currentGroupId)));
    if (answer == QMessageBox::Yes)
        action();

    // The combo already shows the clicked group; restore the edited one when
    // the action was cancelled.
    refreshGroupComboBox();
}

bool EditorDialog::validateGroupName(const QString &name, const QString &excludeId)
{
    if (name.isEmpty()) {
        QMessageBox::warning(this, "分组", "组名不能为空。");
        return false;
    }
    if (name.size() > kMaxGroupNameLength) {
        QMessageBox::warning(this, "分组", "组名不能超过30个字符。");
        return false;
    }

    const QString existing = m_controller->findGroupIdByName(name);
    if (!existing.isEmpty() && existing != excludeId) {
        QMessageBox::warning(this, "分组", QString("分组“%1”已存在。").arg(name));
        return false;
    }

    return true;
}

void EditorDialog::onGroupSelected(int index)
{
    if (index < 0 || index >= m_groupIds.size())
        return;

    const QString selected = m_groupIds.at(index);
    if (selected == m_currentGroupId)
        return;

    withDiscardConfirmation([this, selected]() { loadGroup(selected); });
}

void EditorDialog::onGroupAdd()
{
    QString name;
    if (!promptGroupName(this, "添加分组", "组名：", QString(), &name))
        return;

    name = name.trimmed();
    if (!validateGroupName(name, QString()))
        return;

    if (isListModified()) {
        const QString currentName = m_controller->groupName(m_currentGroupId);
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this, "添加分组",
            QString("是否保存对“%1”的修改并切换到新分组？").arg(currentName),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Cancel)
            return;
        if (answer == QMessageBox::Yes)
            m_controller->saveGroupNames(m_currentGroupId, m_model->stringList());
    }

    loadGroup(m_controller->addGroup(name));
}

void EditorDialog::onGroupRename()
{
    const QString currentName = m_controller->groupName(m_currentGroupId);

    QString name;
    if (!promptGroupName(this, "重命名分组", "新组名：", currentName, &name))
        return;

    name = name.trimmed();
    if (name == currentName)
        return;
    if (!validateGroupName(name, m_currentGroupId))
        return;

    m_controller->renameGroup(m_currentGroupId, name);
    setWindowTitle(QString("YuXiang Drawer - %1").arg(name));
    refreshGroupComboBox();
}

void EditorDialog::onGroupDelete()
{
    withDiscardConfirmation([this]() {
        if (m_controller->groupCount() <= 1) {
            QMessageBox::warning(this, "删除分组", "至少需要保留一个分组。");
            return;
        }

        const QString name = m_controller->groupName(m_currentGroupId);
        if (QMessageBox::question(this, "删除分组",
                                  QString("是否删除分组“%1”？\n该分组内的名单将一并删除。").arg(name))
            != QMessageBox::Yes) {
            return;
        }

        m_controller->deleteGroup(m_currentGroupId);
        loadGroup(m_controller->activeGroupId());
    });
}

void EditorDialog::syncTextToModel()
{
    QString text = m_textEdit->toPlainText();
    text.replace("\r", "").replace("\n", ",").replace("\t", " ");
    const QString &cleaned = text;
    {
        const QSignalBlocker blocker(m_textEdit);
        if (m_textEdit->toPlainText() != cleaned) {
            m_textEdit->setPlainText(cleaned);
            QTextCursor cursor = m_textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            m_textEdit->setTextCursor(cursor);
        }
    }

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

    PasswordDialog dlg3("修改密码", "确认新密码：", this);
    if (dlg3.exec() != QDialog::Accepted)
        return;

    if (dlg2.password() != dlg3.password()) {
        QMessageBox::warning(this, "修改密码", "两次输入的新密码不一致。");
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
    // Spaces next to a separating comma are not part of the names, and the
    // same goes for a space at either end of the list.
    QStringList names;
    const QStringList entries = m_model->stringList();
    names.reserve(entries.size());
    for (const QString &entry : entries) {
        const QString name = entry.trimmed();
        if (!name.isEmpty())
            names.append(name);
    }

    if (names.isEmpty()) {
        QMessageBox::warning(this, "应用", "列表不能为空。");
        return;
    }

    m_controller->saveGroupNames(m_currentGroupId, names);
    m_controller->activateGroup(m_currentGroupId);
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
