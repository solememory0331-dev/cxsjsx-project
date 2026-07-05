#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QStringList>
#include "Constants.h"

class GameView;
class Player;
class Enemy;
class Bullet;
class Laser;
class MagicCircle;
class ExperienceOrb;
class WaveManager;
class UpgradeSystem;
class QMediaPlayer;
class QAudioOutput;

class Game : public QObject
{
    Q_OBJECT

public:
    explicit Game(QObject *parent = nullptr);
    ~Game();

    void start();
    void restart();
    void startNewGame();  // called from main menu

private slots:
    void tick();
    void playNextSong();

private:
    // ── Game loop phases ───────────────────────────────────
    void processInput();
    void updateGameObjects();
    void handleCollisions();
    void cleanupDeadItems();
    void checkWaveTransition();
    void startNextWave();
    void onPlayerDied();

    // ── State management ───────────────────────────────────
    void enterState(GameState s);
    void handleMenuClick(QPointF scenePos);
    void handleVolumeDrag(QPointF scenePos);
    void syncHudSettings();

    // ── Item management ────────────────────────────────────
    void onBulletCreated(Bullet *bullet);
    void onLaserCreated(Laser *laser);
    void onEnemyDied(Enemy *enemy);

    // ── Core components ────────────────────────────────────
    QGraphicsScene *m_scene;
    GameView       *m_view;
    QTimer         *m_gameTimer;
    Player         *m_player;
    WaveManager    *m_waveManager;
    UpgradeSystem  *m_upgradeSystem;
    MagicCircle    *m_magicCircle = nullptr;

    // Item containers
    QList<Bullet*>         m_bullets;
    QList<Laser*>          m_lasers;
    QList<Enemy*>          m_enemies;
    QList<ExperienceOrb*>  m_orbs;

    // Game state
    GameState m_state;
    int       m_currentWave;
    int       m_score;

    // Background music
    QMediaPlayer *m_bgmPlayer;
    QAudioOutput *m_bgmOutput;
    QStringList   m_musicFiles;
    int           m_currentTrack;

    // Settings
    GameSettings    m_settings;
    QString         m_currentTrackName;
    int             m_difficultyIndex = 1;  // 0=Easy, 1=Normal, 2=Hard, 3=Extreme
    DifficultyParams m_diffParams;
    bool            m_draggingVolume = false;
    bool            m_settingsFromGameOver = false;

    // High scores per difficulty
    int             m_highScores[4] = {0, 0, 0, 0};
    bool            m_isNewHighScore = false;
    void loadHighScores();
    void saveHighScores();
};

#endif // GAME_H
