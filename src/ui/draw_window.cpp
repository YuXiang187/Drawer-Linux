#include "draw_window.h"

#include <QApplication>
#include <QCursor>
#include <QFont>
#include <QGuiApplication>
#include <QLabel>
#include <QProgressBar>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

DrawWindow::DrawWindow(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_progressValue(100)
    , m_isRunning(false)
{
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    setAttribute(Qt::WA_TranslucentBackground, false);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 10);
    layout->setSpacing(6);

    m_label = new QLabel("?", this);
    m_label->setAlignment(Qt::AlignCenter);

    QFont font;
    font.setPointSize(60);
    font.setBold(true);
    m_label->setFont(font);

    layout->addWidget(m_label, 1, Qt::AlignCenter);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(10);
    layout->addWidget(m_progressBar);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DrawWindow::onTimerTick);
}

void DrawWindow::showName(const QString &name)
{
    if (m_isRunning) {
        m_timer->stop();
    }

    m_isRunning = true;
    m_label->setText(name);
    m_progressValue = 100;
    m_progressBar->setValue(100);

    centerOnScreen();
    show();
    raise();
    activateWindow();

    m_timer->start(14);
}

void DrawWindow::centerOnScreen()
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    resize(450, 250);

    QRect geo = screen->availableGeometry();
    int x = geo.center().x() - width() / 2;
    int y = geo.center().y() - height() / 2;
    move(x, y);
}

void DrawWindow::onTimerTick()
{
    m_progressValue--;
    m_progressBar->setValue(m_progressValue);

    if (m_progressValue <= 0) {
        m_timer->stop();
        hide();
        m_isRunning = false;
    }
}
