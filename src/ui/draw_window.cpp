#include "draw_window.h"
#include "core/name_pool.h"

#include <QApplication>
#include <QCursor>
#include <QFont>
#include <QGuiApplication>
#include <QLabel>
#include <QPalette>
#include <QProgressBar>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kFlashFrames = 8;
constexpr int kFlashIntervalMs = 60;
constexpr int kProgressSteps = 100;
constexpr int kFadeIntervalMs = 14;
const QColor kFlashColor(128, 128, 128);
} // namespace

DrawWindow::DrawWindow(NamePool *pool, QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_pool(pool)
    , m_flashFrame(0)
    , m_progressValue(kProgressSteps)
    , m_isFlashing(false)
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
    m_progressBar->setRange(0, kProgressSteps);
    m_progressBar->setValue(kProgressSteps);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(10);
    layout->addWidget(m_progressBar);

    m_flashTimer = new QTimer(this);
    connect(m_flashTimer, &QTimer::timeout, this, &DrawWindow::onFlashTick);

    m_fadeTimer = new QTimer(this);
    connect(m_fadeTimer, &QTimer::timeout, this, &DrawWindow::onFadeTick);
}

void DrawWindow::start()
{
    if (m_isFlashing)
        return;

    m_fadeTimer->stop();

    m_isFlashing = true;
    m_flashFrame = 0;
    m_progressValue = kProgressSteps;
    m_progressBar->setValue(m_progressValue);
    setLabelColor(kFlashColor);
    m_label->setText(m_pool->randomName());

    centerOnScreen();
    show();
    raise();
    activateWindow();

    emit animationStarted();
    m_flashTimer->start(kFlashIntervalMs);
}

void DrawWindow::onFlashTick()
{
    ++m_flashFrame;

    // preview frames
    if (m_flashFrame < kFlashFrames) {
        m_label->setText(m_pool->randomName());
        return;
    }

    // last frame
    if (m_flashFrame == kFlashFrames) {
        m_label->setText(m_pool->draw());
        setLabelColor(resultColor());
        return;
    }

    m_flashTimer->stop();
    m_isFlashing = false;

    emit drawFinished();

    m_fadeTimer->start(kFadeIntervalMs);
}

void DrawWindow::onFadeTick()
{
    m_progressBar->setValue(--m_progressValue);

    if (m_progressValue > 0)
        return;

    m_fadeTimer->stop();
    hide();
}

void DrawWindow::setLabelColor(const QColor &color)
{
    QPalette palette = m_label->palette();
    palette.setColor(QPalette::WindowText, color);
    m_label->setPalette(palette);
}

QColor DrawWindow::resultColor() const
{
    QPalette palette = this->palette();
    palette.setCurrentColorGroup(QPalette::Active);
    return palette.color(QPalette::WindowText);
}

void DrawWindow::centerOnScreen()
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    resize(450, 250);

    QRect geo = screen->availableGeometry();
    int x = geo.center().x() - width() / 2;
    int y = geo.center().y() - height() / 2;
    move(x, y);
}
