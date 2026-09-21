#include "floating_window.h"

#include <QApplication>
#include <QCloseEvent>
#include <QCursor>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPushButton>
#include <QScreen>
#include <QStyle>
#include <QVBoxLayout>
#include <QWindow>

namespace {
constexpr int kWindowSize = 40;
constexpr int kIconSize = 24;
} // namespace

FloatingWindow::FloatingWindow(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_button(new QPushButton(this))
    , m_isDragging(false)
{
    setWindowTitle(QStringLiteral("浮窗"));
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    // Showing the window must not steal the focus from the application in use.
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setFixedSize(kWindowSize, kWindowSize);

    m_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    m_button->setIconSize(QSize(kIconSize, kIconSize));
    m_button->setToolTip(QStringLiteral("抽取"));
    m_button->setCursor(Qt::PointingHandCursor);
    // Focus stays enabled on purpose: clicking the button activates the window,
    // which is what makes Alt+F4 / Alt+F3 address it.
    m_button->installEventFilter(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_button);

    connect(m_button, &QPushButton::clicked, this, &FloatingWindow::drawRequested);
}

void FloatingWindow::showCentered()
{
    centerOnScreen();
    show();
    raise();
}

void FloatingWindow::centerOnScreen()
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    const QRect geo = screen->availableGeometry();
    move(geo.center().x() - width() / 2, geo.center().y() - height() / 2);
}

void FloatingWindow::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    if (event->isAccepted())
        emit closedByUser();
}

bool FloatingWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_button)
        return QWidget::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            beginMove(mouseEvent);
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (m_isDragging && (mouseEvent->buttons() & Qt::RightButton)) {
            move(mouseEvent->globalPosition().toPoint() - m_dragOffset);
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease:
        m_isDragging = false;
        break;
    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

void FloatingWindow::beginMove(const QMouseEvent *event)
{
    m_isDragging = false;

    // Let the window manager handle the drag when the platform supports it.
    if (QWindow *handle = windowHandle(); handle && handle->startSystemMove())
        return;

    // Fallback for platforms without system move support.
    m_isDragging = true;
    m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
}
