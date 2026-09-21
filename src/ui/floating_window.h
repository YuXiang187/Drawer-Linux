#pragma once

#include <QPoint>
#include <QWidget>

class QMouseEvent;
class QPushButton;

// Small always-on-top button that floats above the desktop.
// Left click triggers a draw, right click (drag) moves the window.
class FloatingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FloatingWindow(QWidget *parent = nullptr);

    // Shows the window centered on the screen the cursor currently is on.
    void showCentered();

signals:
    void drawRequested();
    // Emitted when the window is closed by the user (e.g. Alt+F4).
    void closedByUser();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void centerOnScreen();
    void beginMove(const QMouseEvent *event);

    QPushButton *m_button;
    bool m_isDragging;
    QPoint m_dragOffset;
};
