#pragma once

#include <QDialog>

class QLineEdit;

class PasswordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PasswordDialog(const QString &title, const QString &label, QWidget *parent = nullptr);

    QString password() const;

private:
    QLineEdit *m_lineEdit;
};
