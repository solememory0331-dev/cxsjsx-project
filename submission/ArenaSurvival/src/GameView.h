#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QSet>
#include <QPointF>

class Player;

class GameView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GameView(QGraphicsScene *scene, QWidget *parent = nullptr);

    // Input state accessors (polled by Game each tick)
    const QSet<int> &keysPressed() const { return m_keysPressed; }
    QPointF mouseScenePos()    const { return m_mouseScenePos; }
    bool    isMouseHeld()      const { return m_mouseHeld; }
    bool    isMouseInView()    const { return m_mouseInView; }

    void setPlayer(Player *player);

    // Mouse click for menus (cleared after read)
    bool    mouseClicked()     const { return m_mouseClicked; }
    QPointF clickPos()         const { return m_clickPos; }       // scene coords
    QPointF clickViewPos()     const { return m_clickViewPos; }   // viewport coords
    void    clearMouseClick()        { m_mouseClicked = false; }

    // ESC key for pause (cleared after read)
    bool    escPressed()       const { return m_escPressed; }
    void    clearEsc()               { m_escPressed = false; }

    // Upgrade selection (polled by Game when paused)
    int  upgradeKeyPressed() const { return m_upgradeKeyPressed; }
    void clearUpgradeKey()         { m_upgradeKeyPressed = 0; }

    // Restart request (polled by Game when game over)
    bool restartRequested()  const { return m_restartRequested; }
    void clearRestart()            { m_restartRequested = false; }

    // These are set by Game for HUD rendering
    void setWaveInfo(int currentWave, int enemiesRemaining, int enemiesAlive);
    void setGameOver(bool gameOver, int finalScore);
    void setCurrentStateName(const QString &name) { m_stateName = name; }
    QString currentStateName() const { return m_stateName; }
    void setHudSettings(bool useArrowKeys, const QString &trackName,
                        float volume) {
        m_hudUseArrowKeys = useArrowKeys;
        m_hudTrackName = trackName;
        m_hudVolume = volume;
    }
    void setHudUpgradeNames(const QStringList &names) {
        m_hudUpgradeNames = names;
    }
    void setHudDifficultyIndex(int idx) { m_hudDifficultyIndex = idx; }
    void setHudHighScores(const int scores[4]) {
        for (int i = 0; i < 4; i++) m_hudHighScores[i] = scores[i];
    }
    void setNewHighScore(bool val) { m_newHighScore = val; }

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QSet<int> m_keysPressed;
    QPointF   m_mouseScenePos;
    bool      m_mouseHeld;
    bool      m_mouseInView;
    Player   *m_player;

    // Mouse click
    bool    m_mouseClicked;
    QPointF m_clickPos;       // scene coordinates (for gameplay)
    QPointF m_clickViewPos;   // viewport coordinates (for menu hit test)

    // ESC
    bool m_escPressed;

    // HUD data
    int     m_currentWave;
    int     m_enemiesRemaining;
    int     m_enemiesAlive;
    bool    m_gameOver;
    int     m_finalScore;
    QString m_stateName;

    // Upgrade selection
    int  m_upgradeKeyPressed;
    bool m_restartRequested;

    // HUD menu data (set by Game)
    bool        m_hudUseArrowKeys;
    QString     m_hudTrackName;
    float       m_hudVolume;
    QStringList m_hudUpgradeNames;
    int         m_hudDifficultyIndex = 1;
    int         m_hudHighScores[4] = {0, 0, 0, 0};
    bool        m_newHighScore = false;
};

#endif // GAMEVIEW_H
