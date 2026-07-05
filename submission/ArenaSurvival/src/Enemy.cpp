#include "Enemy.h"
#include "Player.h"
#include "Constants.h"
#include "TextureManager.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>

// ═══════════════════════════════════════════════════════════════
// Enemy base class
// ═══════════════════════════════════════════════════════════════

Enemy::Enemy(Player *target, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_target(target)
    , m_hp(GameConstants::ENEMY_BASE_HP)
    , m_maxHp(GameConstants::ENEMY_BASE_HP)
    , m_speed(GameConstants::ENEMY_BASE_SPEED)
    , m_damage(GameConstants::ENEMY_BASE_DAMAGE)
    , m_scoreValue(GameConstants::ENEMY_SCORE_VALUE)
    , m_xpValue(GameConstants::XP_PER_KILL)
    , m_radius(GameConstants::ENEMY_BASE_SIZE)
    , m_color(220, 60, 60)   // default red
    , m_type(EnemyType::Normal)
{
}

Enemy::~Enemy()
{
}

void Enemy::advance(int phase)
{
    if (phase == 0 || !m_target) return;

    // Safety: if target was removed from scene, stop moving
    if (!m_target->scene()) return;

    // Direction toward player
    QPointF toPlayer = m_target->pos() - pos();
    double distance = qSqrt(toPlayer.x() * toPlayer.x()
                            + toPlayer.y() * toPlayer.y());

    if (distance < 1.0) return;  // already at player

    QPointF direction = toPlayer / distance;

    // Separation from nearby enemies (anti-clumping)
    QPointF separation(0, 0);
    int neighborCount = 0;

    QList<QGraphicsItem*> nearby = scene()->collidingItems(this);
    for (auto *item : nearby) {
        if (item->type() == GameConstants::EnemyType) {
            Enemy *other = static_cast<Enemy*>(item);
            QPointF away = pos() - other->pos();
            double dist = qSqrt(away.x() * away.x() + away.y() * away.y());
            if (dist < m_radius * 3.0 && dist > 0.01) {
                separation += away / dist;
                neighborCount++;
            }
        }
    }

    // Blend: 85% toward player, 15% separation
    QPointF finalDir = direction;
    if (neighborCount > 0) {
        separation /= neighborCount;
        finalDir = direction * 0.85 + separation * 0.15;
        double len = qSqrt(finalDir.x() * finalDir.x()
                           + finalDir.y() * finalDir.y());
        if (len > 0.01) finalDir /= len;
    }

    // Apply velocity
    setPos(pos() + finalDir * m_speed);
}

void Enemy::takeDamage(int amount)
{
    m_hp -= amount;
    if (m_hp <= 0) {
        m_hp = 0;
        emit enemyDied(this);
    }
}

// ── Setters (for Boss override) ──────────────────────────────

void Enemy::setHP(int hp)
{
    m_hp = hp;
    m_maxHp = hp;
}

void Enemy::setDamageVal(int dmg)
{
    m_damage = dmg;
}

void Enemy::setSpeedVal(double spd)
{
    m_speed = spd;
}

void Enemy::setScoreValue(int val)
{
    m_scoreValue = val;
}

void Enemy::setXPValue(int val)
{
    m_xpValue = val;
}

void Enemy::setScale(qreal scale)
{
    // Resize the item (radius-based)
    m_radius *= scale;
    // QGraphicsItem::setScale handles visual transform
    QGraphicsItem::setScale(scale);
}

// ── QGraphicsItem overrides ──────────────────────────────────

QRectF Enemy::boundingRect() const
{
    qreal d = m_radius * 2.0;
    return QRectF(-m_radius, -m_radius, d, d);
}

QPainterPath Enemy::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void Enemy::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem * /*option*/,
                  QWidget * /*widget*/)
{
    QRectF r = boundingRect();

    // ── Texture (fallback to solid circle) ────────────────
    QString texName;
    switch (m_type) {
    case EnemyType::Fast:  texName = "enemy_fast.png";  break;
    case EnemyType::Tank:  texName = "enemy_tank.png";  break;
    case EnemyType::Boss:  texName = "enemy_tank.png";  break;  // boss reuses tank texture
    default:               texName = "enemy_normal.png"; break;
    }

    const QPixmap &tex = TextureManager::instance().texture(texName);
    if (!tex.isNull()) {
        painter->drawPixmap(r.topLeft(), tex.scaled(
            static_cast<int>(r.width()), static_cast<int>(r.height()),
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Body (original solid circle)
        painter->setBrush(m_color);
        painter->setPen(QPen(m_color.darker(180), 2));
        painter->drawEllipse(r);
    }

    // HP bar above enemy
    if (m_hp < m_maxHp) {
        qreal barWidth = m_radius * 2.0;
        qreal barHeight = 4.0;
        qreal barX = -barWidth / 2.0;
        qreal barY = -m_radius - 10.0;

        // Background
        painter->fillRect(QRectF(barX, barY, barWidth, barHeight),
                          QColor(40, 0, 0));
        // Fill
        double pct = static_cast<double>(m_hp) / m_maxHp;
        QColor barColor = pct > 0.5 ? QColor(60, 200, 60) :
                          pct > 0.25 ? QColor(220, 180, 20) :
                                       QColor(220, 40, 40);
        painter->fillRect(QRectF(barX, barY, barWidth * pct, barHeight),
                          barColor);
    }
}

// ═══════════════════════════════════════════════════════════════
// NormalEnemy
// ═══════════════════════════════════════════════════════════════

NormalEnemy::NormalEnemy(Player *target, int wave, QGraphicsItem *parent)
    : Enemy(target, parent)
{
    m_type = EnemyType::Normal;

    double hpScale  = 1.0 + (wave - 1) * GameConstants::HP_SCALE_PER_WAVE;
    double spdScale = 1.0 + (wave - 1) * GameConstants::SPEED_SCALE_PER_WAVE;

    m_hp     = static_cast<int>(GameConstants::ENEMY_BASE_HP * hpScale);
    m_maxHp  = m_hp;
    m_speed  = GameConstants::ENEMY_BASE_SPEED * spdScale;
    m_radius = GameConstants::ENEMY_BASE_SIZE;
    m_color  = QColor(220, 60, 60);  // red
}

// ═══════════════════════════════════════════════════════════════
// FastEnemy
// ═══════════════════════════════════════════════════════════════

FastEnemy::FastEnemy(Player *target, int wave, QGraphicsItem *parent)
    : Enemy(target, parent)
{
    m_type = EnemyType::Fast;

    double hpScale  = 1.0 + (wave - 1) * GameConstants::HP_SCALE_PER_WAVE;
    double spdScale = 1.0 + (wave - 1) * GameConstants::SPEED_SCALE_PER_WAVE;

    m_hp     = static_cast<int>(GameConstants::ENEMY_BASE_HP
                                * hpScale * GameConstants::FAST_ENEMY_HP_MUL);
    m_maxHp  = m_hp;
    m_speed  = GameConstants::ENEMY_BASE_SPEED
               * spdScale * GameConstants::FAST_ENEMY_SPEED_MUL;
    m_radius = GameConstants::ENEMY_BASE_SIZE * GameConstants::FAST_ENEMY_SIZE_MUL;
    m_color  = QColor(220, 120, 40);  // orange
    m_scoreValue = static_cast<int>(GameConstants::ENEMY_SCORE_VALUE * 1.5);
}

// ═══════════════════════════════════════════════════════════════
// TankEnemy
// ═══════════════════════════════════════════════════════════════

TankEnemy::TankEnemy(Player *target, int wave, QGraphicsItem *parent)
    : Enemy(target, parent)
{
    m_type = EnemyType::Tank;

    double hpScale  = 1.0 + (wave - 1) * GameConstants::HP_SCALE_PER_WAVE;
    double spdScale = 1.0 + (wave - 1) * GameConstants::SPEED_SCALE_PER_WAVE;

    m_hp     = static_cast<int>(GameConstants::ENEMY_BASE_HP
                                * hpScale * GameConstants::TANK_ENEMY_HP_MUL);
    m_maxHp  = m_hp;
    m_speed  = GameConstants::ENEMY_BASE_SPEED
               * spdScale * GameConstants::TANK_ENEMY_SPEED_MUL;
    m_radius = GameConstants::ENEMY_BASE_SIZE * GameConstants::TANK_ENEMY_SIZE_MUL;
    m_color  = QColor(140, 40, 140);  // purple
    m_scoreValue = static_cast<int>(GameConstants::ENEMY_SCORE_VALUE * 2.5);
}
