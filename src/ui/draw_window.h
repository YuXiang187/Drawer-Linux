#pragma once

#include <QWidget>

class QColor;
class QLabel;
class QProgressBar;
class QTimer;
class NamePool;

class DrawWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DrawWindow(NamePool *pool, QWidget *parent = nullptr);

    void start();

signals:
    void animationStarted();
    void drawFinished();

private slots:
    void onFlashTick();
    void onFadeTick();

private:
    void centerOnScreen();
    void setLabelColor(const QColor &color);
    QColor resultColor() const;

    NamePool *m_pool;
    QLabel *m_label;
    QProgressBar *m_progressBar;
    QTimer *m_flashTimer;
    QTimer *m_fadeTimer;
    int m_flashFrame;
    int m_progressValue;
    bool m_isFlashing;
};
