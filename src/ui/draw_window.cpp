#include "draw_window.h"
#include "core/name_pool.h"

#include <QApplication>
#include <QCursor>
#include <QFont>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QLabel>
#include <QPalette>
#include <QProgressBar>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace {
constexpr int kFlashFrames = 8;
constexpr int kFlashIntervalMs = 60;
constexpr int kProgressSteps = 100;
constexpr int kFadeIntervalMs = 14;
const QColor kFlashColor(128, 128, 128);

constexpr int kBaseWidth = 450;
constexpr int kBaseHeight = 250;
constexpr int kMarginX = 12;
constexpr double kFontSize = 60.0;
constexpr double kMinFontSize = 10.0;
// names measured to find the widest one; a large list is estimated from a
// sample instead of measuring every entry
constexpr int kSampleCount = 64;
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

    // Pick the result before the animation runs so the window can be sized to
    // it; the preview frames below deliberately show unrelated names.
    m_drawnName = m_pool->draw();
    m_label->setText(m_pool->randomName());

    QScreen *screen = targetScreen();
    fitToNames(screen);
    centerOnScreen(screen);

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
        m_label->setText(m_drawnName);
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

QScreen *DrawWindow::targetScreen() const
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    return screen ? screen : QGuiApplication::primaryScreen();
}

void DrawWindow::fitToNames(QScreen *screen)
{
    QStringList names = m_pool->sample(kSampleCount);
    if (!m_drawnName.isEmpty())
        names.append(m_drawnName);

    const int limit = screen ? screen->availableGeometry().width() - kMarginX * 2
                             : kBaseWidth - kMarginX * 2;

    double size = kFontSize;
    int required = widestName(names, size);
    while (required > limit && required > 0 && size > kMinFontSize) {
        size = std::max(kMinFontSize, size * limit / required);
        required = widestName(names, size);
    }

    QFont font = m_label->font();
    font.setPointSizeF(size);
    m_label->setFont(font);

    int width = std::max(kBaseWidth, required + kMarginX * 2);
    if (screen)
        width = std::min(width, screen->availableGeometry().width());
    resize(width, kBaseHeight);
}

int DrawWindow::widestName(const QStringList &names, double pointSize) const
{
    QFont probe = m_label->font();
    probe.setPointSizeF(pointSize);
    const QFontMetricsF metrics(probe);

    int widest = 0;
    for (const QString &name : names)
        widest = std::max(widest, static_cast<int>(std::ceil(metrics.horizontalAdvance(name))));
    return widest;
}

void DrawWindow::centerOnScreen(QScreen *screen)
{
    if (!screen)
        return;

    const QRect geo = screen->availableGeometry();
    move(geo.center().x() - width() / 2, geo.center().y() - height() / 2);
}
