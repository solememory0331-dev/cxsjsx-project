#include "Bullet.h"
#include "Constants.h"

#include <QPainter>
#include <QDateTime>
#include <QtMath>
#include "TextureManager.h"

Bullet::Bullet(QPointF startPos, QPointF targetPos, int damage,
               QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_speed(GameConstants::BULLET_SPEED)
    , m_damage(damage)
    , m_createdAt(QDateTime::currentMSecsSinceEpoch())
    , m_startPos(startPos)
    , m_maxDistance(GameConstants::BULLET_MAX_DISTANCE)
    , m_radius(GameConstants::BULLET_SIZE)
    , m_dead(false)
{
    setPos(startPos);

    // Compute direction toward target
    QPointF delta = targetPos - startPos;
    double len = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    if (len < 1.0) {
        // Default: fire right
        m_direction = QVector2D(1.0f, 0.0f);
    } else {
        m_direction = QVector2D(static_cast<float>(delta.x() / len),
                                static_cast<float>(delta.y() / len));
    }
}

void Bullet::advance(int phase)
{
    if (phase == 0 || m_dead) return;

    QPointF newPos = pos() + m_direction.toPointF() * m_speed;
    setPos(newPos);
}

bool Bullet::isExpired() const
{
    if (m_dead) return true;

    // Check lifetime
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_createdAt > GameConstants::BULLET_LIFETIME_MS)
        return true;

    // Check distance
    QPointF delta = pos() - m_startPos;
    double dist = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    if (dist > m_maxDistance)
        return true;

    // Check out of scene bounds
    if (pos().x() < -50 || pos().x() > GameConstants::SCENE_WIDTH + 50 ||
        pos().y() < -50 || pos().y() > GameConstants::SCENE_HEIGHT + 50)
        return true;

    return false;
}

QRectF Bullet::boundingRect() const
{
    qreal d = m_radius * 2.0;
    return QRectF(-m_radius, -m_radius, d, d);
}

QPainterPath Bullet::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void Bullet::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem * /*option*/,
                   QWidget * /*widget*/)
{
    const QPixmap &tex = TextureManager::instance().texture("bullet.png");
    if (!tex.isNull()) {
        // Rotate so the bullet head faces movement direction,
        // tail faces back toward the player.
        // Texture default head faces bottom-right (45° offset).
        double angle = qRadiansToDegrees(
            qAtan2(static_cast<double>(m_direction.y()),
                   static_cast<double>(m_direction.x()))) - 45.0;

        QRectF r = boundingRect();
        QPixmap scaled = tex.scaled(
            static_cast<int>(r.width()), static_cast<int>(r.height()),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);

        painter->save();
        painter->translate(0, 0);
        painter->rotate(angle);
        painter->drawPixmap(QPointF(-scaled.width() / 2.0, -scaled.height() / 2.0), scaled);
        painter->restore();
    } else {
        QRectF r = boundingRect();
        painter->setBrush(QColor(255, 220, 60));  // bright yellow
        painter->setPen(QPen(QColor(255, 160, 20), 1));
        painter->drawEllipse(r);
    }
}
