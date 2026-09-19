#include "password_dialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

PasswordDialog::PasswordDialog(const QString &title, const QString &label, QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    setWindowTitle(title);
    setMinimumWidth(260);

    auto *layout = new QVBoxLayout(this);

    auto *lbl = new QLabel(label, this);
    layout->addWidget(lbl);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_lineEdit);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString PasswordDialog::password() const
{
    return m_lineEdit->text();
}
