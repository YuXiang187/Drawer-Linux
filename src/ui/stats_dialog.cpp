#include "stats_dialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QStyle>
#include <QVBoxLayout>

namespace {

constexpr int kWindowWidth = 430;
constexpr int kWindowHeight = 400;
constexpr int kMinWindowWidth = 330;
constexpr int kMinWindowHeight = 240;

QPointer<StatsDialog> s_current;

} // namespace

void StatsDialog::showStats(QWidget *parent, const QString &groupName, const QStringList &names)
{
    // A click while the window is already up brings it back instead of
    // opening a second one.
    if (s_current) {
        s_current->show();
        s_current->raise();
        s_current->activateWindow();
        return;
    }

    auto *dialog = new StatsDialog(groupName, names, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    s_current = dialog;
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

StatsDialog::StatsDialog(const QString &groupName, const QStringList &names, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("统计");
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    resize(kWindowWidth, kWindowHeight);
    setMinimumSize(kMinWindowWidth, kMinWindowHeight);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 12, 14, 12);

    // The header lives in the same box, so a long group name wraps instead of
    // overflowing, and the whole list stays selectable for copying.
    auto *textEdit = new QPlainTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setUndoRedoEnabled(false);
    textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    textEdit->setPlainText(QString("抽取分组：%1\n抽取数量：%2\n\n%3")
                               .arg(groupName)
                               .arg(names.size())
                               .arg(names.join(", ")));
    layout->addWidget(textEdit, 1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    if (QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close))
        closeButton->setText("关闭");
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);
    layout->addWidget(buttonBox);

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect area = screen->availableGeometry();
        move(area.center().x() - width() / 2, area.center().y() - height() / 2);
    }
}
