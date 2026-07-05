#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtGlobal>
#include <QString>
#include <QGraphicsItem>

namespace GameConstants {

// ── Window / Scene ──────────────────────────────────────────
constexpr int   SCENE_WIDTH  = 3840;   // world size
constexpr int   SCENE_HEIGHT = 2160;   // world size
constexpr int   VIEW_WIDTH   = 1280;   // window size
constexpr int   VIEW_HEIGHT  = 720;    // window size

// ── Player defaults ─────────────────────────────────────────
constexpr double PLAYER_SPEED          = 5.0;
constexpr int    PLAYER_MAX_HP         = 100;
constexpr double PLAYER_SIZE           = 30.0;
constexpr int    PLAYER_BASE_DAMAGE    = 25;
constexpr int    PLAYER_FIRE_COOLDOWN  = 250;
constexpr int    INVINCIBILITY_MS      = 1500;
constexpr double PLAYER_TEXTURE_SCALE  = 1.8;   // visual scale factor for player texture

// ── Bullet defaults ─────────────────────────────────────────
constexpr double BULLET_SPEED          = 12.0;
constexpr double BULLET_SIZE           = 10.0;
constexpr double BULLET_MAX_DISTANCE   = 800.0;
constexpr int    BULLET_LIFETIME_MS    = 2000;

// ── Laser defaults ──────────────────────────────────────────
constexpr double LASER_FIRE_COOLDOWN_MULT = 1.3;   // slower than bullets
constexpr double LASER_BEAM_WIDTH         = 25.0;   // same as bullet diameter
constexpr double LASER_DAMAGE_MULT        = 1.5;   // laser deals 1.5x bullet damage
constexpr int    LASER_VISUAL_DURATION_MS = 150;

// ── Magic Circle defaults ───────────────────────────────────
constexpr double MAGIC_CIRCLE_RADIUS_MULT  = 4.0;   // × player collision radius (visual is ~×3 scale)
constexpr int    MAGIC_CIRCLE_BASE_DAMAGE  = 5;
constexpr int    MAGIC_CIRCLE_DMG_INTERVAL = 100;   // ms between damage ticks

// ── Enemy defaults ──────────────────────────────────────────
constexpr double ENEMY_BASE_SPEED      = 2.5;
constexpr int    ENEMY_BASE_HP         = 50;
constexpr int    ENEMY_BASE_DAMAGE     = 20;
constexpr double ENEMY_BASE_SIZE       = 16.0;
constexpr int    ENEMY_SCORE_VALUE     = 10;

// ── Enemy subtype multipliers ───────────────────────────────
constexpr double FAST_ENEMY_SPEED_MUL  = 1.8;
constexpr double FAST_ENEMY_HP_MUL     = 0.5;
constexpr double FAST_ENEMY_SIZE_MUL   = 0.75;
constexpr double TANK_ENEMY_SPEED_MUL  = 0.5;
constexpr double TANK_ENEMY_HP_MUL     = 3.0;
constexpr double TANK_ENEMY_SIZE_MUL   = 1.5;

// ── Wave scaling ────────────────────────────────────────────
constexpr double HP_SCALE_PER_WAVE     = 0.10;
constexpr double SPEED_SCALE_PER_WAVE  = 0.01;
constexpr int    BASE_ENEMIES_PER_WAVE = 5;
constexpr int    EXTRA_ENEMIES_PER_WAVE = 3;

// ── Spawning ────────────────────────────────────────────────
constexpr int    BASE_SPAWN_INTERVAL   = 800;
constexpr int    MIN_SPAWN_INTERVAL    = 200;
constexpr double SPAWN_MARGIN          = 60.0;
constexpr int    BOSS_WAVE_INTERVAL    = 5;

// ── Experience / Orbs ───────────────────────────────────────
constexpr int    XP_PER_KILL           = 10;
constexpr double ORB_MAGNET_RANGE      = 80.0;
constexpr double ORB_SPEED             = 4.0;
constexpr int    XP_BASE_PER_LEVEL     = 50;
constexpr double XP_PER_LEVEL_SCALE    = 1.3;

// ── Game loop ───────────────────────────────────────────────
constexpr int    TICK_INTERVAL_MS      = 16;

// ── Custom QGraphicsItem type IDs ───────────────────────────
enum ItemType {
    PlayerType      = QGraphicsItem::UserType + 1,
    EnemyType       = QGraphicsItem::UserType + 2,
    BulletType      = QGraphicsItem::UserType + 3,
    ExperienceOrbType = QGraphicsItem::UserType + 4,
    LaserType       = QGraphicsItem::UserType + 5,
    MagicCircleType = QGraphicsItem::UserType + 6
};

} // namespace GameConstants

// ── Difficulty ──────────────────────────────────────────────
enum class Difficulty {
    Easy,
    Normal,
    Hard,
    Extreme
};

struct DifficultyParams {
    double playerHpMult;
    double playerDamageMult;
    double playerSpeedMult;
    double playerFireRateMult;   // >1 = slower fire rate (cooldown × this)
    double enemyHpMult;
    double enemySpeedMult;
    double enemyDamageMult;
    double enemyCountMult;
    double xpNeededMult;
};

inline DifficultyParams difficultyParams(Difficulty d)
{
    using D = Difficulty;
    switch (d) {
    //                    PlyHP  PlyDmg PlySpd FireRt EnemyHP EnemySpd EnemyDmg EnemyCnt XPneed
    case D::Easy:    return { 1.30,  1.20,  1.10,  0.88,  0.70,   0.85,    0.70,    0.80,   0.85 };
    case D::Normal:  return { 1.00,  1.00,  1.00,  1.00,  1.00,   1.00,    1.00,    1.00,   1.00 };
    case D::Hard:    return { 0.80,  0.88,  0.96,  1.12,  1.30,   1.15,    1.30,    1.20,   1.15 };
    case D::Extreme: return { 0.60,  0.72,  0.90,  1.28,  1.80,   1.35,    1.80,    1.50,   1.30 };
    }
    return { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
}

// ── Game State Machine ──────────────────────────────────────
enum class GameState {
    MainMenu,
    Playing,
    Paused,
    Settings,
    Upgrading,
    GameOver
};

// ── Runtime Settings ────────────────────────────────────────
struct GameSettings {
    bool       useArrowKeys = false;   // false=WASD, true=Arrow keys
    float      musicVolume  = 0.3f;    // 0.0 ~ 1.0
    int        currentTrack = -1;      // index into music list
    Difficulty difficulty   = Difficulty::Normal;

    int upKey()    const { return useArrowKeys ? Qt::Key_Up    : Qt::Key_W; }
    int downKey()  const { return useArrowKeys ? Qt::Key_Down  : Qt::Key_S; }
    int leftKey()  const { return useArrowKeys ? Qt::Key_Left  : Qt::Key_A; }
    int rightKey() const { return useArrowKeys ? Qt::Key_Right : Qt::Key_D; }
};

#endif // CONSTANTS_H
