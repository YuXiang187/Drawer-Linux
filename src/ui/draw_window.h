#pragma once

#include <QStringList>
#include <QWidget>

class QColor;
class QLabel;
class QProgressBar;
class QScreen;
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
    QScreen *targetScreen() const;
    // Widens the window and shrinks the font until the widest of the sampled
    // names fits, instead of measuring all names of a large list.
    void fitToNames(QScreen *screen);
    int widestName(const QStringList &names, double pointSize) const;
    void centerOnScreen(QScreen *screen);
    void setLabelColor(const QColor &color);
    QColor resultColor() const;

    NamePool *m_pool;
    QLabel *m_label;
    QProgressBar *m_progressBar;
    QTimer *m_flashTimer;
    QTimer *m_fadeTimer;
    QString m_drawnName;
    int m_flashFrame;
    int m_progressValue;
    bool m_isFlashing;
};
