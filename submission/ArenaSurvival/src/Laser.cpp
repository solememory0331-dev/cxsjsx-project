#include "Laser.h"
#include "Enemy.h"

#include <QPainter>
#include <QDateTime>
#include <QtMath>

Laser::Laser(QPointF startPos, QPointF aimPoint, int damage, double beamWidth,
             QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
    , m_damage(damage)
    , m_beamWidth(beamWidth)
    , m_createdAt(QDateTime::currentMSecsSinceEpoch())
    , m_dead(false)
{
    // Compute direction toward aim point
    QPointF delta = aimPoint - startPos;
    double len = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    if (len < 1.0) {
        // Default: fire right
        delta = QPointF(1.0, 0.0);
        len = 1.0;
    }

    double dx = delta.x() / len;
    double dy = delta.y() / len;

    // Laser beam: from startPos to max distance in aim direction
    QPointF endPos(startPos.x() + dx * GameConstants::BULLET_MAX_DISTANCE,
                   startPos.y() + dy * GameConstants::BULLET_MAX_DISTANCE);

    setLine(QLineF(startPos, endPos));
}

bool Laser::isExpired() const
{
    if (m_dead) return true;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    return (now - m_createdAt) > GameConstants::LASER_VISUAL_DURATION_MS;
}

void Laser::checkHit(const QList<Enemy*> &enemies)
{
    if (m_dead) return;

    QLineF beam = line();
    QPointF p1 = beam.p1();
    QPointF p2 = beam.p2();

    for (auto *enemy : enemies) {
        if (enemy->isDead()) continue;

        // Check if laser beam intersects enemy's bounding circle
        double enemyRadius = enemy->boundingRect().width() / 2.0;
        double dist = pointToSegmentDist(enemy->pos(), p1, p2);
        if (dist <= enemyRadius) {
            enemy->takeDamage(m_damage);
        }
    }

    // Laser pierces all enemies — stays alive for visual duration
}

QRectF Laser::boundingRect() const
{
    QLineF beam = line();
    double halfW = m_beamWidth / 2.0 + 1.0;

    QPointF topLeft(qMin(beam.x1(), beam.x2()) - halfW,
                    qMin(beam.y1(), beam.y2()) - halfW);
    QPointF bottomRight(qMax(beam.x1(), beam.x2()) + halfW,
                        qMax(beam.y1(), beam.y2()) + halfW);

    return QRectF(topLeft, bottomRight);
}

QPainterPath Laser::shape() const
{
    // Thick line shape for collision (unused since we do instant hit,
    // but needed for QGraphicsItem interface)
    QPainterPath path;
    QLineF beam = line();
    QPointF dir = beam.p2() - beam.p1();
    double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
    if (len < 1.0) return path;

    double hw = m_beamWidth / 2.0;
    double nx = -dir.y() / len * hw;
    double ny =  dir.x() / len * hw;

    QPolygonF poly;
    poly << beam.p1() + QPointF(nx, ny)
         << beam.p2() + QPointF(nx, ny)
         << beam.p2() - QPointF(nx, ny)
         << beam.p1() - QPointF(nx, ny);
    path.addPolygon(poly);
    return path;
}

void Laser::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem * /*option*/,
                  QWidget * /*widget*/)
{
    // Fade out near end of visual duration
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_createdAt;
    int duration = GameConstants::LASER_VISUAL_DURATION_MS;
    int alpha = 255;
    if (elapsed > duration / 2) {
        double fade = 1.0 - static_cast<double>(elapsed - duration / 2)
                           / (duration / 2);
        alpha = static_cast<int>(255 * qBound(0.0, fade, 1.0));
    }

    QColor beamColor(255, 255, 255, alpha);        // white core
    QColor glowColor(200, 220, 255, alpha / 2);    // blue-ish outer glow

    QLineF beam = line();
    double hw = m_beamWidth;

    // Outer glow (wider)
    QPen glowPen(glowColor, hw * 3.0, Qt::SolidLine, Qt::RoundCap);
    painter->setPen(glowPen);
    painter->drawLine(beam);

    // Inner core
    QPen corePen(beamColor, hw, Qt::SolidLine, Qt::RoundCap);
    painter->setPen(corePen);
    painter->drawLine(beam);
}

double Laser::pointToSegmentDist(QPointF p, QPointF a, QPointF b)
{
    QPointF ab = b - a;
    QPointF ap = p - a;
    double abLen2 = ab.x() * ab.x() + ab.y() * ab.y();

    if (abLen2 < 1e-9)
        return qSqrt(ap.x() * ap.x() + ap.y() * ap.y()); // a==b

    double t = (ap.x() * ab.x() + ap.y() * ab.y()) / abLen2;
    t = qBound(0.0, t, 1.0);

    QPointF closest = a + ab * t;
    QPointF diff = p - closest;
    return qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
}
