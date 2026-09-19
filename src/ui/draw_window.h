#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;
class QTimer;

class DrawWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DrawWindow(QWidget *parent = nullptr);
    void showName(const QString &name);

private slots:
    void onTimerTick();

private:
    void centerOnScreen();

    QLabel *m_label;
    QProgressBar *m_progressBar;
    QTimer *m_timer;
    int m_progressValue;
    bool m_isRunning;
};
