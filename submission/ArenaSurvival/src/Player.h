#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsEllipseItem>
#include <QSet>
#include <QPointF>
#include <QTimer>
#include <QObject>
#include "Constants.h"

class Bullet;
class Laser;

class Player : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    explicit Player(QGraphicsItem *parent = nullptr);
    ~Player();

    // ── Per-frame update (called by Game::processInput) ──────
    void setKeysHeld(const QSet<int> &keys);
    void updateMovement();
    void updateAiming(QPointF mouseScenePos);
    void tryFire();
    void updateInvincibility();

    // ── Key binding ──────────────────────────────────────────
    void setUseArrowKeys(bool use) { m_useArrowKeys = use; }
    bool useArrowKeys() const      { return m_useArrowKeys; }

    // ── Difficulty ──────────────────────────────────────────
    void applyDifficulty(const DifficultyParams &dp);

    // ── Damage / healing ─────────────────────────────────────
    void takeDamage(int amount);
    void heal(int amount);
    bool isInvincible() const;
    void activateInvincibility();

    // ── Stats ────────────────────────────────────────────────
    int  hp()      const { return m_hp; }
    int  maxHp()   const { return m_maxHp; }
    int  damage()  const { return m_damage; }
    int  score()   const { return m_score; }
    int  xp()      const { return m_xp; }
    int  level()   const { return m_level; }
    int  xpToNextLevel() const;

    void addScore(int points);
    void addXP(int amount);

    // ── Upgrade application (Phase 10) ───────────────────────
    void increaseMaxHP(int amount);
    void increaseDamage(int amount);
    void increaseSpeed(double amount);
    void reduceFireCooldown(int ms);

    // ── Laser upgrade ───────────────────────────────────────
    void enableLaser();
    bool hasLaser() const { return m_hasLaser; }
    void increaseLaserWidth(double amount);

    // ── Magic Circle upgrade ────────────────────────────────
    void enableMagicCircle();
    bool hasMagicCircle() const { return m_hasMagicCircle; }

    // ── QGraphicsItem overrides ──────────────────────────────
    enum { Type = GameConstants::PlayerType };
    int type() const override { return Type; }
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

signals:
    void bulletCreated(Bullet *bullet);
    void laserCreated(Laser *laser);
    void playerDied();

private slots:
    void toggleFlash();

private:
    // Movement
    QSet<int> m_keysHeld;
    double    m_speed;
    QPointF   m_aimPoint;
    bool      m_useArrowKeys;

    // Combat
    int    m_hp;
    int    m_maxHp;
    int    m_damage;
    bool   m_invincible;
    qint64 m_invincibleUntil;
    int    m_fireCooldownMs;
    qint64 m_lastFireTime;

    // Progression
    int    m_score;
    int    m_xp;
    int    m_level;
    double m_xpMult = 1.0;

    // Laser
    bool   m_hasLaser;
    double m_laserBeamWidth;

    // Magic Circle
    bool   m_hasMagicCircle;

    // Visual
    qreal  m_radius;
    QColor m_color;
    QTimer *m_flashTimer;
    bool   m_flashVisible;
};

#endif // PLAYER_H
