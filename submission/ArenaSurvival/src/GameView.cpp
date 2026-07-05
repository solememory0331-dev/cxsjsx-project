#include "GameView.h"
#include "Player.h"
#include "Constants.h"
#include "HUD.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QEnterEvent>
#include <QPainter>

GameView::GameView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_mouseHeld(false)
    , m_mouseInView(false)
    , m_player(nullptr)
    , m_mouseClicked(false)
    , m_escPressed(false)
    , m_currentWave(0)
    , m_enemiesRemaining(0)
    , m_enemiesAlive(0)
    , m_gameOver(false)
    , m_finalScore(0)
    , m_upgradeKeyPressed(0)
    , m_restartRequested(false)
    , m_clickViewPos(0, 0)
    , m_hudUseArrowKeys(false)
    , m_hudVolume(0.3f)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void GameView::setPlayer(Player *player)
{
    m_player = player;
}

void GameView::setWaveInfo(int currentWave, int enemiesRemaining, int enemiesAlive)
{
    m_currentWave = currentWave;
    m_enemiesRemaining = enemiesRemaining;
    m_enemiesAlive = enemiesAlive;
}

void GameView::setGameOver(bool gameOver, int finalScore)
{
    m_gameOver = gameOver;
    m_finalScore = finalScore;
}

// ── Keyboard ────────────────────────────────────────────────────

void GameView::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    m_keysPressed.insert(key);

    // ESC — pause toggle
    if (key == Qt::Key_Escape) {
        m_escPressed = true;
    }

    // Upgrade keys (1/2/3) — kept for backward compatibility
    if (key == Qt::Key_1 || key == Qt::Key_2 ||
        key == Qt::Key_3) {
        m_upgradeKeyPressed = key - Qt::Key_0;
    }

    // R to restart
    if (key == Qt::Key_R && m_gameOver) {
        m_restartRequested = true;
    }

    // Suppress QGraphicsView scrolling for movement keys
    if (key == Qt::Key_Up || key == Qt::Key_Down ||
        key == Qt::Key_Left || key == Qt::Key_Right ||
        key == Qt::Key_W || key == Qt::Key_A ||
        key == Qt::Key_S || key == Qt::Key_D) {
        event->accept();
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void GameView::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    m_keysPressed.remove(key);

    // Suppress QGraphicsView scrolling for movement keys
    if (key == Qt::Key_Up || key == Qt::Key_Down ||
        key == Qt::Key_Left || key == Qt::Key_Right ||
        key == Qt::Key_W || key == Qt::Key_A ||
        key == Qt::Key_S || key == Qt::Key_D) {
        event->accept();
        return;
    }

    QGraphicsView::keyReleaseEvent(event);
}

// ── Mouse ───────────────────────────────────────────────────────

void GameView::mouseMoveEvent(QMouseEvent *event)
{
    m_mouseScenePos = mapToScene(event->pos());
    QGraphicsView::mouseMoveEvent(event);
}

void GameView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mouseHeld = true;
        m_mouseClicked = true;
        m_clickPos = mapToScene(event->pos());
        m_clickViewPos = event->pos();  // viewport coords for menu hit test
    }
    QGraphicsView::mousePressEvent(event);
}

void GameView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mouseHeld = false;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GameView::leaveEvent(QEvent *event)
{
    m_mouseInView = false;
    QGraphicsView::leaveEvent(event);
}

void GameView::enterEvent(QEnterEvent *event)
{
    m_mouseInView = true;
    QGraphicsView::enterEvent(event);
}

void GameView::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}

// ── Background ──────────────────────────────────────────────────

void GameView::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QColor(30, 30, 35));

    // Grid
    painter->setPen(QPen(QColor(45, 45, 50), 1));
    const int gridSize = 64;
    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    qreal top  = int(rect.top())  - (int(rect.top())  % gridSize);
    for (qreal x = left; x < rect.right(); x += gridSize)
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += gridSize)
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}

// ── Foreground (HUD + Menus) ────────────────────────────────────

void GameView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    if (!m_player) return;

    // Reset to viewport coordinates so HUD stays fixed on screen
    painter->save();
    painter->resetTransform();

    QRectF vr = viewport()->rect();

    HUD::draw(painter, vr, m_player,
              m_currentWave, m_enemiesRemaining, m_enemiesAlive,
              m_gameOver, m_finalScore, m_stateName,
              m_hudUseArrowKeys, m_hudTrackName, m_hudVolume,
              m_hudUpgradeNames, m_hudDifficultyIndex,
              m_hudHighScores, m_newHighScore);

    painter->restore();
}
