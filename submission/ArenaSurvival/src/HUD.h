#ifndef HUD_H
#define HUD_H

#include <QPainter>
#include <QRectF>
#include <QString>

class Player;

// Identifies which menu element was clicked
enum class MenuAction {
    None,
    StartGame, SelectEasy, SelectNormal, SelectHard, SelectExtreme,
    Resume, Settings, ExitGame, QuitGame,
    ToggleKeys, PrevTrack, NextTrack, VolumeBar,
    Back,
    Upgrade1, Upgrade2, Upgrade3,
    GameOverRestart, GameOverSettings, GameOverDifficulty,
    GameOverSelectEasy, GameOverSelectNormal, GameOverSelectHard, GameOverSelectExtreme,
    GameOverExit
};

class HUD
{
public:
    // ── Main draw entry ─────────────────────────────────────
    static void draw(QPainter *painter, const QRectF &viewRect,
                     const Player *player,
                     int currentWave, int enemiesRemaining, int enemiesAlive,
                     bool gameOver, int finalScore,
                     const QString &stateName,
                     bool useArrowKeys,
                     const QString &trackName, float volume,
                     const QStringList &upgradeNames,
                     int difficultyIndex = 1,
                     const int highScores[4] = nullptr,
                     bool isNewHighScore = false);

    // ── Hit testing for mouse clicks ─────────────────────────
    static MenuAction hitTest(QPointF clickScenePos, const QRectF &viewRect,
                              const QString &stateName,
                              const QStringList &upgradeNames,
                              const QString &currentTrackName,
                              float volume,
                              int difficultyIndex = 1,
                              const int highScores[4] = nullptr);

    // ── Volume bar position (for drag) ───────────────────────
    static QRectF volumeBarRect(const QRectF &viewRect);
    static QRectF volumeBarRectMainMenu(const QRectF &viewRect);

private:
    // Gameplay HUD
    static void drawHealthBar(QPainter *p, qreal x, qreal y,
                              qreal w, qreal h, int cur, int max);
    static void drawXPBar(QPainter *p, qreal x, qreal y,
                          qreal w, qreal h, int cur, int needed);
    static void drawScore(QPainter *p, const QRectF &r, int score);
    static void drawWaveInfo(QPainter *p, const QRectF &r,
                             int wave, int rem, int alive);
    static void drawGameOver(QPainter *p, const QRectF &r, int score,
                              const int highScores[4], int currentDifficulty,
                              bool isNewHighScore);

    // Menu helpers
    static void drawMenuOverlay(QPainter *p, const QRectF &r);
    static void drawMenuTitle(QPainter *p, const QRectF &r,
                              const QString &title);
    static QRectF drawMenuButton(QPainter *p, const QRectF &r,
                                 int index, const QString &text,
                                 bool highlight);
    static void drawMainMenu(QPainter *p, const QRectF &r,
                              bool useArrowKeys,
                              const QString &trackName, float volume,
                              int difficultyIndex,
                              const int highScores[4]);
    static void drawPauseMenu(QPainter *p, const QRectF &r);
    static void drawSettingsMenu(QPainter *p, const QRectF &r,
                                  bool useArrowKeys,
                                  const QString &trackName,
                                  float volume);
    static void drawUpgradeMenu(QPainter *p, const QRectF &r,
                                const QStringList &names);
};

#endif // HUD_H
