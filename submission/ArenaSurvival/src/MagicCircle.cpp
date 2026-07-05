#include "MagicCircle.h"
#include "Player.h"
#include "Enemy.h"
#include "TextureManager.h"

#include <QPainter>
#include <QDateTime>

MagicCircle::MagicCircle(Player *player, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_player(player)
    , m_radius(player->boundingRect().width() / 2.0
               * GameConstants::PLAYER_TEXTURE_SCALE   // visual size
               * GameConstants::MAGIC_CIRCLE_RADIUS_MULT)
    , m_damage(GameConstants::MAGIC_CIRCLE_BASE_DAMAGE)
    , m_lastDamageTime(0)
    , m_dead(false)
{
    setZValue(-1);  // render below player
    updatePosition();
}

void MagicCircle::updatePosition()
{
    if (!m_player) return;
    QPointF p = m_player->pos();
    setPos(p);
}

void MagicCircle::checkDamage(const QList<Enemy*> &enemies)
{
    if (m_dead) return;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastDamageTime < GameConstants::MAGIC_CIRCLE_DMG_INTERVAL)
        return;
    m_lastDamageTime = now;

    for (auto *enemy : enemies) {
        if (enemy->isDead()) continue;

        QPointF diff = enemy->pos() - pos();
        double dist = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
        double enemyRadius = enemy->boundingRect().width() / 2.0;

        // Enemy touches the circle area
        if (dist - enemyRadius <= m_radius) {
            enemy->takeDamage(m_damage);
        }
    }
}

void MagicCircle::increaseRadius(double amount)
{
    m_radius += amount;
    prepareGeometryChange();
}

void MagicCircle::increaseDamage(int amount)
{
    m_damage += amount;
}

QRectF MagicCircle::boundingRect() const
{
    double d = m_radius * 2.0;
    return QRectF(-m_radius, -m_radius, d, d);
}

void MagicCircle::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem * /*option*/,
                        QWidget * /*widget*/)
{
    QRectF r = boundingRect();

    const QPixmap &tex = TextureManager::instance().texture("magic_circle.png");
    if (!tex.isNull()) {
        painter->drawPixmap(r.topLeft(), tex.scaled(
            static_cast<int>(r.width()), static_cast<int>(r.height()),
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Fallback: semi-transparent ring
        QPen ringPen(QColor(100, 200, 255, 120), 3.0);
        painter->setPen(ringPen);
        painter->setBrush(QColor(100, 200, 255, 25));
        painter->drawEllipse(r);

        // Inner glow ring
        QPen innerPen(QColor(150, 220, 255, 80), 1.5);
        painter->setPen(innerPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(r.adjusted(4, 4, -4, -4));
    }
}
