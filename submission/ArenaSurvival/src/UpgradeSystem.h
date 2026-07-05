#ifndef UPGRADESYSTEM_H
#define UPGRADESYSTEM_H

#include <QObject>
#include <QList>
#include <QStringList>

class QGraphicsScene;
class QGraphicsItem;
class Player;

struct UpgradeDef
{
    QString name;
    QString description;
    enum class Stat { MaxHP, Damage, Speed, FireRate,
                      LaserUpgrade, LaserWidth,
                      MagicCircleUpgrade, MagicCircleRadius, MagicCircleDamage } stat;
    double value;
};

class UpgradeSystem : public QObject
{
    Q_OBJECT

public:
    explicit UpgradeSystem(QGraphicsScene *scene, Player *player,
                           QObject *parent = nullptr);

    void setPlayer(Player *player) { m_player = player; }

    void showUpgradeSelection();
    void hideUpgradeSelection();
    void applyUpgrade(int index);

    // For HUD to draw upgrade option names
    QStringList currentUpgradeNames() const;
    UpgradeDef::Stat currentUpgradeStat(int index) const;
    double currentUpgradeValue(int index) const;

signals:
    void upgradeSelected();

private:
    QList<UpgradeDef> generateRandomUpgrades() const;

    QGraphicsScene      *m_scene;
    Player              *m_player;
    QList<UpgradeDef>    m_currentOptions;
    QList<QGraphicsItem*> m_upgradeUIItems;
};

#endif // UPGRADESYSTEM_H
