#include "ExperienceOrb.h"
#include "Player.h"
#include "TextureManager.h"

#include <QPainter>
#include <QtMath>

ExperienceOrb::ExperienceOrb(QPointF position, int xpValue, Player *player,
                             QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_xpValue(xpValue)
    , m_player(player)
    , m_magnetRange(GameConstants::ORB_MAGNET_RANGE)
    , m_speed(GameConstants::ORB_SPEED)
    , m_radius(6.0)
    , m_dead(false)
{
    setPos(position);
}

void ExperienceOrb::advance(int phase)
{
    if (phase == 0 || m_dead || !m_player) return;

    // Safety: if player was removed from scene, don't try to follow
    if (!m_player->scene()) return;

    QPointF toPlayer = m_player->pos() - pos();
    double dist = qSqrt(toPlayer.x() * toPlayer.x()
                        + toPlayer.y() * toPlayer.y());

    // Magnet effect: move toward player if within range
    if (dist < m_magnetRange && dist > 0.5) {
        QPointF dir = toPlayer / dist;
        // Faster when closer
        double pullSpeed = m_speed * (m_magnetRange - dist) / m_magnetRange + 0.5;
        setPos(pos() + dir * pullSpeed);
    }
}

QRectF ExperienceOrb::boundingRect() const
{
    qreal d = m_radius * 2.0;
    return QRectF(-m_radius, -m_radius, d, d);
}

QPainterPath ExperienceOrb::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void ExperienceOrb::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem * /*option*/,
                          QWidget * /*widget*/)
{
    QRectF r = boundingRect();

    const QPixmap &tex = TextureManager::instance().texture("experience_orb.png");
    if (!tex.isNull()) {
        painter->drawPixmap(r.topLeft(), tex.scaled(
            static_cast<int>(r.width()), static_cast<int>(r.height()),
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Glowing green orb (original)
        painter->setBrush(QColor(100, 255, 100));
        painter->setPen(QPen(QColor(50, 200, 50), 1.5));
        painter->drawEllipse(r);

        // Small bright center
        painter->setBrush(QColor(200, 255, 200));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), m_radius * 0.4, m_radius * 0.4);
    }
}
