#include "UpgradeSystem.h"
#include "Player.h"
#include "Constants.h"

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QRandomGenerator>

UpgradeSystem::UpgradeSystem(QGraphicsScene *scene, Player *player,
                             QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_player(player)
{
}

void UpgradeSystem::showUpgradeSelection()
{
    m_currentOptions = generateRandomUpgrades();
    // HUD now draws the upgrade menu, so we don't create UI items
}

void UpgradeSystem::hideUpgradeSelection()
{
    // Clean up any leftover UI items
    for (auto *item : m_upgradeUIItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_upgradeUIItems.clear();
    m_currentOptions.clear();
}

void UpgradeSystem::applyUpgrade(int index)
{
    if (index < 0 || index >= m_currentOptions.size()) return;

    const UpgradeDef &up = m_currentOptions.at(index);

    switch (up.stat) {
    case UpgradeDef::Stat::MaxHP:
        m_player->increaseMaxHP(static_cast<int>(up.value));
        break;
    case UpgradeDef::Stat::Damage:
        m_player->increaseDamage(static_cast<int>(up.value));
        break;
    case UpgradeDef::Stat::Speed:
        m_player->increaseSpeed(up.value);
        break;
    case UpgradeDef::Stat::FireRate:
        m_player->reduceFireCooldown(static_cast<int>(up.value));
        break;
    case UpgradeDef::Stat::LaserUpgrade:
        m_player->enableLaser();
        break;
    case UpgradeDef::Stat::LaserWidth:
        m_player->increaseLaserWidth(up.value);
        break;
    case UpgradeDef::Stat::MagicCircleUpgrade:
        m_player->enableMagicCircle();
        emit upgradeSelected();
        // MagicCircle is created by Game — signal tells Game to create it
        return;
    case UpgradeDef::Stat::MagicCircleRadius:
        // handled in Game via m_magicCircle->increaseRadius()
        break;
    case UpgradeDef::Stat::MagicCircleDamage:
        // handled in Game via m_magicCircle->increaseDamage()
        break;
    }

    emit upgradeSelected();
}

QStringList UpgradeSystem::currentUpgradeNames() const
{
    QStringList names;
    for (const auto &u : m_currentOptions)
        names.append(u.name + "\n" + u.description);
    return names;
}

UpgradeDef::Stat UpgradeSystem::currentUpgradeStat(int index) const
{
    if (index < 0 || index >= m_currentOptions.size())
        return UpgradeDef::Stat::MaxHP;  // fallback
    return m_currentOptions.at(index).stat;
}

double UpgradeSystem::currentUpgradeValue(int index) const
{
    if (index < 0 || index >= m_currentOptions.size())
        return 0.0;
    return m_currentOptions.at(index).value;
}

QList<UpgradeDef> UpgradeSystem::generateRandomUpgrades() const
{
    QList<UpgradeDef> pool;

    // ── Laser upgrade (only before laser is enabled) ────────
    if (!m_player->hasLaser()) {
        pool.append({"Laser Beam!",
                     "Transform bullets into white piercing laser",
                     UpgradeDef::Stat::LaserUpgrade, 0.0});
    }

    // ── Magic Circle (always available, independent of laser) ──
    if (!m_player->hasMagicCircle()) {
        pool.append({"Magic Circle!",
                     "Summon a damage aura around you",
                     UpgradeDef::Stat::MagicCircleUpgrade, 0.0});
    }

    // ── Common upgrades ─────────────────────────────────────
    pool.append({"Max HP +20",   "Increase max health by 20",
                 UpgradeDef::Stat::MaxHP, 20.0});
    pool.append({"Max HP +35",   "Increase max health by 35",
                 UpgradeDef::Stat::MaxHP, 35.0});

    // ── Bullet/Laser damage (removed after laser, laser has fixed damage=bullet) ──
    if (!m_player->hasLaser()) {
        pool.append({"Damage +5",    "Increase damage by 5",
                     UpgradeDef::Stat::Damage, 5.0});
        pool.append({"Damage +10",   "Increase damage by 10",
                     UpgradeDef::Stat::Damage, 10.0});
    }

    // ── Speed ───────────────────────────────────────────────
    pool.append({"Speed Up",     "Move 20% faster",
                 UpgradeDef::Stat::Speed, 0.8});
    pool.append({"Swift Boots",  "Move 35% faster",
                 UpgradeDef::Stat::Speed, 1.2});

    // ── Laser width (only after laser is enabled) ───────────
    if (m_player->hasLaser()) {
        pool.append({"Wider Laser",   "Increase laser width by 3",
                     UpgradeDef::Stat::LaserWidth, 3.0});
        pool.append({"Thick Laser",   "Increase laser width by 5",
                     UpgradeDef::Stat::LaserWidth, 5.0});
    }

    // ── Magic Circle upgrades (only after magic circle is enabled) ──
    if (m_player->hasMagicCircle()) {
        pool.append({"Aura Damage +3",  "Increase aura damage by 3",
                     UpgradeDef::Stat::MagicCircleDamage, 3.0});
        pool.append({"Aura Damage +6",  "Increase aura damage by 6",
                     UpgradeDef::Stat::MagicCircleDamage, 6.0});
        pool.append({"Aura Radius +10", "Increase aura radius by 10",
                     UpgradeDef::Stat::MagicCircleRadius, 10.0});
        pool.append({"Aura Radius +20", "Increase aura radius by 20",
                     UpgradeDef::Stat::MagicCircleRadius, 20.0});
    }

    // ── Fire rate (works for both bullet and laser) ─────────
    pool.append({"Faster Fire",  "Reduce fire cooldown by 40ms",
                 UpgradeDef::Stat::FireRate, 40.0});
    pool.append({"Rapid Fire",   "Reduce fire cooldown by 70ms",
                 UpgradeDef::Stat::FireRate, 70.0});
    pool.append({"Vitality +15", "Increase max HP by 15 and heal",
                 UpgradeDef::Stat::MaxHP, 15.0});

    // Shuffle and pick 3
    QList<UpgradeDef> shuffled = pool;
    auto *rng = QRandomGenerator::global();
    for (int i = shuffled.size() - 1; i > 0; i--) {
        int j = rng->bounded(i + 1);
        std::swap(shuffled[i], shuffled[j]);
    }

    QList<UpgradeDef> selected;
    for (int i = 0; i < 3 && i < shuffled.size(); i++)
        selected.append(shuffled[i]);
    return selected;
}
