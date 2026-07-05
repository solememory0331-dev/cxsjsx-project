#include "Player.h"
#include "Bullet.h"
#include "Laser.h"
#include "Constants.h"
#include "TextureManager.h"

#include <QPainter>
#include <QDateTime>
#include <QtMath>

Player::Player(QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_speed(GameConstants::PLAYER_SPEED)
    , m_hp(GameConstants::PLAYER_MAX_HP)
    , m_maxHp(GameConstants::PLAYER_MAX_HP)
    , m_damage(GameConstants::PLAYER_BASE_DAMAGE)
    , m_invincible(false)
    , m_invincibleUntil(0)
    , m_fireCooldownMs(GameConstants::PLAYER_FIRE_COOLDOWN)
    , m_lastFireTime(0)
    , m_score(0)
    , m_xp(0)
    , m_level(1)
    , m_radius(GameConstants::PLAYER_SIZE)
    , m_color(60, 180, 255)
    , m_flashTimer(new QTimer(this))
    , m_flashVisible(true)
    , m_useArrowKeys(false)
    , m_hasLaser(false)
    , m_laserBeamWidth(GameConstants::LASER_BEAM_WIDTH)
    , m_hasMagicCircle(false)
{
    setFlag(QGraphicsItem::ItemIsFocusable);

    // Flash timer for invincibility visual
    connect(m_flashTimer, &QTimer::timeout, this, &Player::toggleFlash);
}

Player::~Player()
{
}

void Player::applyDifficulty(const DifficultyParams &dp)
{
    m_maxHp  = static_cast<int>(GameConstants::PLAYER_MAX_HP * dp.playerHpMult);
    m_hp     = m_maxHp;
    m_damage = static_cast<int>(GameConstants::PLAYER_BASE_DAMAGE * dp.playerDamageMult);
    m_speed  = GameConstants::PLAYER_SPEED * dp.playerSpeedMult;
    m_fireCooldownMs = static_cast<int>(GameConstants::PLAYER_FIRE_COOLDOWN * dp.playerFireRateMult);
    if (m_fireCooldownMs < 50) m_fireCooldownMs = 50;
    m_xpMult = dp.xpNeededMult;
}

// ── Input ────────────────────────────────────────────────────────

void Player::setKeysHeld(const QSet<int> &keys)
{
    m_keysHeld = keys;
}

// ── Movement ─────────────────────────────────────────────────────

void Player::updateMovement()
{
    double dx = 0.0, dy = 0.0;

    const int up    = m_useArrowKeys ? Qt::Key_Up    : Qt::Key_W;
    const int down  = m_useArrowKeys ? Qt::Key_Down  : Qt::Key_S;
    const int left  = m_useArrowKeys ? Qt::Key_Left  : Qt::Key_A;
    const int right = m_useArrowKeys ? Qt::Key_Right : Qt::Key_D;

    if (m_keysHeld.contains(up))    dy -= 1.0;
    if (m_keysHeld.contains(down))  dy += 1.0;
    if (m_keysHeld.contains(left))  dx -= 1.0;
    if (m_keysHeld.contains(right)) dx += 1.0;

    // Normalize diagonal movement
    if (dx != 0.0 && dy != 0.0) {
        double inv = 1.0 / qSqrt(dx * dx + dy * dy);
        dx *= inv;
        dy *= inv;
    }

    double newX = pos().x() + dx * m_speed;
    double newY = pos().y() + dy * m_speed;

    // Clamp to scene bounds
    newX = qBound(m_radius, newX,
                   GameConstants::SCENE_WIDTH - m_radius);
    newY = qBound(m_radius, newY,
                   GameConstants::SCENE_HEIGHT - m_radius);

    setPos(newX, newY);
}

void Player::updateAiming(QPointF mouseScenePos)
{
    m_aimPoint = mouseScenePos;
}

void Player::tryFire()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Laser has a separate, slower cooldown
    if (m_hasLaser) {
        int laserCooldown = static_cast<int>(m_fireCooldownMs
                              * GameConstants::LASER_FIRE_COOLDOWN_MULT);
        if (now - m_lastFireTime < laserCooldown)
            return;
        m_lastFireTime = now;

        QPointF startPos = pos();
        int laserDmg = static_cast<int>(m_damage * GameConstants::LASER_DAMAGE_MULT);
        Laser *laser = new Laser(startPos, m_aimPoint, laserDmg, m_laserBeamWidth);
        emit laserCreated(laser);
        return;
    }

    // Normal bullet fire
    if (now - m_lastFireTime < m_fireCooldownMs)
        return;

    m_lastFireTime = now;

    QPointF startPos = pos();
    Bullet *bullet = new Bullet(startPos, m_aimPoint, m_damage);
    emit bulletCreated(bullet);
}

// ── Invincibility ────────────────────────────────────────────────

void Player::updateInvincibility()
{
    if (m_invincible && QDateTime::currentMSecsSinceEpoch() > m_invincibleUntil) {
        m_invincible = false;
        m_flashTimer->stop();
        m_flashVisible = true;
        update();
    }
}

void Player::activateInvincibility()
{
    m_invincible = true;
    m_invincibleUntil = QDateTime::currentMSecsSinceEpoch()
                        + GameConstants::INVINCIBILITY_MS;
    m_flashTimer->start(100);  // toggle every 100ms
}

bool Player::isInvincible() const
{
    return m_invincible;
}

void Player::toggleFlash()
{
    m_flashVisible = !m_flashVisible;
    update();
}

// ── Damage / Healing ─────────────────────────────────────────────

void Player::takeDamage(int amount)
{
    if (m_invincible) return;

    m_hp -= amount;
    if (m_hp < 0) m_hp = 0;

    activateInvincibility();

    if (m_hp <= 0) {
        // Stop flash and make player visible (red, dead)
        m_flashTimer->stop();
        m_flashVisible = true;
        m_invincible = false;
        m_color = QColor(120, 30, 30);  // dark red — dead
        update();
        emit playerDied();
    }
}

void Player::heal(int amount)
{
    m_hp += amount;
    if (m_hp > m_maxHp) m_hp = m_maxHp;
}

// ── Score / XP ───────────────────────────────────────────────────

void Player::addScore(int points)
{
    m_score += points;
}

void Player::addXP(int amount)
{
    m_xp += amount;
    // Check level up
    int needed = xpToNextLevel();
    while (m_xp >= needed) {
        m_xp -= needed;
        m_level++;
        needed = xpToNextLevel();
        // Level-up bonus: heal + permanent stat boost
        heal(15);
        m_maxHp += 3;
        m_hp += 3;
        m_damage += 1;
        m_speed += 0.08;
    }
}

int Player::xpToNextLevel() const
{
    return static_cast<int>(GameConstants::XP_BASE_PER_LEVEL
        * qPow(GameConstants::XP_PER_LEVEL_SCALE, m_level - 1)
        * m_xpMult);
}

// ── Upgrades ─────────────────────────────────────────────────────

void Player::increaseMaxHP(int amount)
{
    m_maxHp += amount;
    m_hp += amount;  // heal by the same amount
}

void Player::increaseDamage(int amount)
{
    m_damage += amount;
}

void Player::increaseSpeed(double amount)
{
    m_speed += amount;
}

void Player::reduceFireCooldown(int ms)
{
    m_fireCooldownMs -= ms;
    if (m_fireCooldownMs < 50)
        m_fireCooldownMs = 50;  // minimum 50ms
}

void Player::enableLaser()
{
    m_hasLaser = true;
}

void Player::increaseLaserWidth(double amount)
{
    m_laserBeamWidth += amount;
}

void Player::enableMagicCircle()
{
    m_hasMagicCircle = true;
}

// ── QGraphicsItem ────────────────────────────────────────────────

QRectF Player::boundingRect() const
{
    qreal d = m_radius * 2.0;
    return QRectF(-m_radius, -m_radius, d, d);
}

QPainterPath Player::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void Player::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem * /*option*/,
                   QWidget * /*widget*/)
{
    // Invincibility flash: skip drawing every other flash tick
    if (m_invincible && !m_flashVisible)
        return;

    constexpr double kTexScale = GameConstants::PLAYER_TEXTURE_SCALE;

    // Visual offset: shift textures without affecting collision position
    painter->save();
    painter->translate(14.0, 5.0);

    // ── Magic circle background (visual only, not collision) ─
    {
        const QPixmap &mcTex = TextureManager::instance().texture("magic_circle.png");
        if (!mcTex.isNull()) {
            double mcSize = m_radius * 2.0 * kTexScale * GameConstants::MAGIC_CIRCLE_RADIUS_MULT;
            QRectF mcRect(-mcSize / 2.0 - 18.0, -mcSize / 2.0, mcSize, mcSize);
            double opacity = m_invincible ? 0.35 : 0.7;
            painter->setOpacity(opacity);
            painter->drawPixmap(mcRect.topLeft(), mcTex.scaled(
                static_cast<int>(mcSize), static_cast<int>(mcSize),
                Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    // ── Player texture (fallback to solid circle) ────────────────
    const QPixmap &tex = TextureManager::instance().texture("player.png");
    if (!tex.isNull()) {
        double size = m_radius * 2.0 * kTexScale;
        QRectF texRect(-size / 2.0, -size / 2.0, size, size);
        painter->setOpacity(m_invincible ? 0.5 : 1.0);
        painter->drawPixmap(texRect.topLeft(), tex.scaled(
            static_cast<int>(size), static_cast<int>(size),
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QRectF r = boundingRect();
        painter->setBrush(m_invincible ? QColor(200, 200, 255, 150) : m_color);
        painter->setPen(QPen(m_color.darker(150), 2));
        painter->drawEllipse(r);
    }

    painter->restore();
    painter->setOpacity(1.0);
}
