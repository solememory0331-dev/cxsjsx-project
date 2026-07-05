#include "Game.h"
#include "GameView.h"
#include "Player.h"
#include "Bullet.h"
#include "Laser.h"
#include "MagicCircle.h"
#include "Enemy.h"
#include "ExperienceOrb.h"
#include "WaveManager.h"
#include "CollisionManager.h"
#include "UpgradeSystem.h"
#include "HUD.h"
#include "Constants.h"
#include "TextureManager.h"

#include <QPainter>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDir>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QApplication>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

Game::Game(QObject *parent)
    : QObject(parent)
    , m_scene(new QGraphicsScene(this))
    , m_view(nullptr)
    , m_gameTimer(new QTimer(this))
    , m_player(nullptr)
    , m_waveManager(nullptr)
    , m_upgradeSystem(nullptr)
    , m_state(GameState::Playing)
    , m_currentWave(0)
    , m_score(0)
    , m_bgmPlayer(nullptr)
    , m_bgmOutput(nullptr)
    , m_currentTrack(-1)
    , m_isNewHighScore(false)
{
    // ── Scene ───────────────────────────────────────────────
    m_scene->setSceneRect(0, 0,
                          GameConstants::SCENE_WIDTH,
                          GameConstants::SCENE_HEIGHT);

    // ── View ────────────────────────────────────────────────
    m_view = new GameView(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setWindowTitle("Arena Survivor");
    m_view->resize(GameConstants::VIEW_WIDTH + 2,
                   GameConstants::VIEW_HEIGHT + 2);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFixedSize(GameConstants::VIEW_WIDTH + 2,
                         GameConstants::VIEW_HEIGHT + 2);

    // ── Player ──────────────────────────────────────────────
    m_player = new Player();
    m_scene->addItem(m_player);
    m_player->setPos(GameConstants::SCENE_WIDTH / 2.0,
                     GameConstants::SCENE_HEIGHT / 2.0);
    m_view->setPlayer(m_player);

    connect(m_player, &Player::bulletCreated,
            this, &Game::onBulletCreated);
    connect(m_player, &Player::laserCreated,
            this, &Game::onLaserCreated);
    connect(m_player, &Player::playerDied,
            this, &Game::onPlayerDied);

    // ── Wave Manager ────────────────────────────────────────
    m_waveManager = new WaveManager(m_scene, m_player, this);
    connect(m_waveManager, &WaveManager::enemySpawned,
            this, [this](Enemy *enemy) {
                m_enemies.append(enemy);
                connect(enemy, &Enemy::enemyDied,
                        this, &Game::onEnemyDied);
            });

    // ── Upgrade System ──────────────────────────────────────
    m_upgradeSystem = new UpgradeSystem(m_scene, m_player, this);
    connect(m_upgradeSystem, &UpgradeSystem::upgradeSelected,
            this, [this]() {
                m_upgradeSystem->hideUpgradeSelection();
                enterState(GameState::Playing);
                startNextWave();
            });

    // ── Background Music ────────────────────────────────────
    m_bgmOutput = new QAudioOutput(this);
    m_bgmOutput->setVolume(m_settings.musicVolume);

    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmPlayer->setAudioOutput(m_bgmOutput);

    QDir musicDir(QCoreApplication::applicationDirPath() + "/Musics");
    if (!musicDir.exists())
        musicDir = QDir(QCoreApplication::applicationDirPath() + "/../Musics");
    if (!musicDir.exists())
        musicDir = QDir(QCoreApplication::applicationDirPath() + "/../../Musics");
    if (!musicDir.exists())
        musicDir = QDir("Musics");

    if (musicDir.exists()) {
        QStringList filters;
        filters << "*.mp3" << "*.wav" << "*.ogg" << "*.flac";
        m_musicFiles = musicDir.entryList(filters, QDir::Files);
        for (QString &f : m_musicFiles)
            f = musicDir.absoluteFilePath(f);
    }

    connect(m_bgmPlayer, &QMediaPlayer::mediaStatusChanged,
            this, [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia)
                    playNextSong();
            });

    // ── Game Loop ───────────────────────────────────────────
    connect(m_gameTimer, &QTimer::timeout,
            this, &Game::tick);

    loadHighScores();
}

Game::~Game()
{
}

// ═══════════════════════════════════════════════════════════════
// State machine
// ═══════════════════════════════════════════════════════════════

void Game::enterState(GameState s)
{
    m_state = s;

    switch (s) {
    case GameState::MainMenu:
        m_view->setCurrentStateName("mainmenu");
        break;
    case GameState::Playing:
        m_view->setCurrentStateName("playing");
        break;
    case GameState::Paused:
        m_view->setCurrentStateName("paused");
        break;
    case GameState::Settings:
        m_view->setCurrentStateName("settings");
        break;
    case GameState::Upgrading:
        m_view->setCurrentStateName("upgrading");
        break;
    case GameState::GameOver:
        m_view->setCurrentStateName("gameover");
        break;
    }

    // Always update HUD with current settings
    m_view->setHudSettings(m_settings.useArrowKeys,
                           m_currentTrackName, m_settings.musicVolume);
    m_view->setHudUpgradeNames(m_upgradeSystem->currentUpgradeNames());
    m_view->setHudDifficultyIndex(m_difficultyIndex);
    m_view->setHudHighScores(m_highScores);
}

// ═══════════════════════════════════════════════════════════════
// Start / Restart
// ═══════════════════════════════════════════════════════════════

void Game::start()
{
    // Generate default textures once (won't overwrite user textures)
    TextureManager::instance().generateDefaultTextures();

    m_view->show();

    m_score       = 0;
    m_currentWave = 0;
    m_view->setGameOver(false, 0);

    // Sync player key binding
    m_player->setUseArrowKeys(m_settings.useArrowKeys);

    // Init difficulty index from settings
    m_difficultyIndex = static_cast<int>(m_settings.difficulty);

    playNextSong();

    enterState(GameState::MainMenu);
    m_gameTimer->start(GameConstants::TICK_INTERVAL_MS);
}

void Game::startNewGame()
{
    // Clear any leftover items from a previous game
    m_gameTimer->stop();

    for (auto *e : m_enemies) {
        m_scene->removeItem(e);
        delete e;
    }
    m_enemies.clear();

    for (auto *b : m_bullets) {
        m_scene->removeItem(b);
        delete b;
    }
    m_bullets.clear();

    for (auto *l : m_lasers) {
        m_scene->removeItem(l);
        delete l;
    }
    m_lasers.clear();

    if (m_magicCircle) {
        m_scene->removeItem(m_magicCircle);
        delete m_magicCircle;
        m_magicCircle = nullptr;
    }

    for (auto *o : m_orbs) {
        m_scene->removeItem(o);
        delete o;
    }
    m_orbs.clear();

    m_upgradeSystem->hideUpgradeSelection();

    // Apply difficulty params
    m_diffParams = difficultyParams(m_settings.difficulty);

    m_score       = 0;
    m_currentWave = 0;

    // Recreate player with difficulty params
    m_scene->removeItem(m_player);
    m_player->deleteLater();

    m_player = new Player();
    m_player->applyDifficulty(m_diffParams);
    m_scene->addItem(m_player);
    m_player->setPos(GameConstants::SCENE_WIDTH / 2.0,
                     GameConstants::SCENE_HEIGHT / 2.0);
    m_player->setUseArrowKeys(m_settings.useArrowKeys);
    m_view->setPlayer(m_player);

    m_waveManager->setPlayer(m_player);
    m_waveManager->setDifficultyParams(m_diffParams);
    m_upgradeSystem->setPlayer(m_player);

    connect(m_player, &Player::bulletCreated,
            this, &Game::onBulletCreated);
    connect(m_player, &Player::laserCreated,
            this, &Game::onLaserCreated);
    connect(m_player, &Player::playerDied,
            this, &Game::onPlayerDied);

    m_view->setGameOver(false, 0);
    m_view->setNewHighScore(false);

    enterState(GameState::Playing);
    startNextWave();
    m_gameTimer->start(GameConstants::TICK_INTERVAL_MS);
}

void Game::restart()
{
    m_gameTimer->stop();

    // Recalculate difficulty params in case changed from game over
    m_diffParams = difficultyParams(m_settings.difficulty);

    // Clear enemies, bullets, orbs
    for (auto *e : m_enemies) {
        m_scene->removeItem(e);
        delete e;
    }
    m_enemies.clear();

    for (auto *b : m_bullets) {
        m_scene->removeItem(b);
        delete b;
    }
    m_bullets.clear();

    for (auto *l : m_lasers) {
        m_scene->removeItem(l);
        delete l;
    }
    m_lasers.clear();

    if (m_magicCircle) {
        m_scene->removeItem(m_magicCircle);
        delete m_magicCircle;
        m_magicCircle = nullptr;
    }

    for (auto *o : m_orbs) {
        m_scene->removeItem(o);
        delete o;
    }
    m_orbs.clear();

    m_upgradeSystem->hideUpgradeSelection();

    // Recreate player
    m_scene->removeItem(m_player);
    m_player->deleteLater();

    m_player = new Player();
    m_player->applyDifficulty(m_diffParams);
    m_scene->addItem(m_player);
    m_player->setPos(GameConstants::SCENE_WIDTH / 2.0,
                     GameConstants::SCENE_HEIGHT / 2.0);
    m_player->setUseArrowKeys(m_settings.useArrowKeys);
    m_view->setPlayer(m_player);

    m_waveManager->setPlayer(m_player);
    m_waveManager->setDifficultyParams(m_diffParams);
    m_upgradeSystem->setPlayer(m_player);

    connect(m_player, &Player::bulletCreated,
            this, &Game::onBulletCreated);
    connect(m_player, &Player::laserCreated,
            this, &Game::onLaserCreated);
    connect(m_player, &Player::playerDied,
            this, &Game::onPlayerDied);

    m_currentWave = 0;
    m_score       = 0;
    m_view->setGameOver(false, 0);
    m_view->setNewHighScore(false);

    enterState(GameState::Playing);
    startNextWave();
    m_gameTimer->start(GameConstants::TICK_INTERVAL_MS);
}

// ═══════════════════════════════════════════════════════════════
// Main tick
// ═══════════════════════════════════════════════════════════════

void Game::tick()
{
    // ── Always check R for restart (works in any state) ─────
    if (m_view->restartRequested()) {
        m_view->clearRestart();
        if (m_state == GameState::GameOver) {
            restart();
            return;
        }
    }

    // ── Game Over ───────────────────────────────────────────
    if (m_state == GameState::GameOver) {
        if (m_view->mouseClicked()) {
            QPointF vpos = m_view->clickViewPos();
            m_view->clearMouseClick();
            handleMenuClick(vpos);
            if (m_state != GameState::GameOver) return;
        }
        m_view->viewport()->update();  // force full redraw
        return;
    }

    // ── Check ESC ───────────────────────────────────────────
    if (m_view->escPressed()) {
        m_view->clearEsc();
        if (m_state == GameState::MainMenu) {
            // ESC does nothing on main menu
            m_view->viewport()->update();
            return;
        }
        if (m_state == GameState::Settings) {
            if (m_settingsFromGameOver) {
                m_settingsFromGameOver = false;
                enterState(GameState::GameOver);
            } else {
                enterState(GameState::Paused);
            }
            m_view->viewport()->update();
            return;
        }
        if (m_state == GameState::Playing) {
            enterState(GameState::Paused);
            m_view->viewport()->update();
            return;
        }
        if (m_state == GameState::Paused) {
            enterState(GameState::Playing);
            m_view->viewport()->update();
            return;
        }
    }

    // ── Mouse clicks for menus ──────────────────────────────
    if (m_view->mouseClicked()) {
        QPointF vpos = m_view->clickViewPos();  // viewport coords for menus
        m_view->clearMouseClick();

        // Check if click is on volume bar
        bool onVolumeBar = false;
        if (m_state == GameState::MainMenu || m_state == GameState::Settings) {
            QRectF bar = (m_state == GameState::MainMenu)
                ? HUD::volumeBarRectMainMenu(m_view->viewport()->rect())
                : HUD::volumeBarRect(m_view->viewport()->rect());
            onVolumeBar = bar.adjusted(0, -8, 0, 8).contains(vpos);
        }

        if (onVolumeBar) {
            handleVolumeDrag(vpos);
            m_draggingVolume = true;
        } else if (m_state == GameState::MainMenu ||
                   m_state == GameState::Paused ||
                   m_state == GameState::Settings ||
                   m_state == GameState::Upgrading) {
            handleMenuClick(vpos);
        }
    }

    // ── Non-playing states: just render ─────────────────────
    if (m_state != GameState::Playing) {
        // Continuous volume drag while mouse held (viewport coords)
        if (m_draggingVolume && m_view->isMouseHeld()) {
            QPointF vp = m_view->mouseScenePos();
            QPoint vpi = m_view->mapFromScene(vp);
            handleVolumeDrag(QPointF(vpi));
        }
        if (!m_view->isMouseHeld()) {
            m_draggingVolume = false;
        }
        m_view->viewport()->update();
        return;
    }

    // ── Playing state: normal game loop ─────────────────────
    processInput();
    updateGameObjects();
    handleCollisions();
    cleanupDeadItems();
    checkWaveTransition();
    m_view->viewport()->update();
}

// ═══════════════════════════════════════════════════════════════
// Menu click handling
// ═══════════════════════════════════════════════════════════════

void Game::handleMenuClick(QPointF scenePos)
{
    QRectF vr = m_view->viewport()->rect();

    // Build upgrade names for hit test
    QStringList upgradeNames;
    if (m_state == GameState::Upgrading) {
        // Get names from UpgradeSystem
        upgradeNames = m_upgradeSystem->currentUpgradeNames();
    }

    MenuAction action = HUD::hitTest(scenePos, vr,
        m_view->currentStateName(), upgradeNames,
        m_currentTrackName, m_settings.musicVolume,
        m_difficultyIndex, m_highScores);

    switch (action) {

    // ── Game Over menu ──────────────────────────────────
    case MenuAction::GameOverRestart:
        restart();
        break;

    case MenuAction::GameOverSettings:
        m_settingsFromGameOver = true;
        enterState(GameState::Settings);
        break;

    case MenuAction::GameOverDifficulty:
        // Toggle difficulty (handled by specific selects below)
        break;

    case MenuAction::GameOverSelectEasy:
        m_difficultyIndex = 0;
        m_settings.difficulty = Difficulty::Easy;
        m_diffParams = difficultyParams(Difficulty::Easy);
        syncHudSettings();
        break;

    case MenuAction::GameOverSelectNormal:
        m_difficultyIndex = 1;
        m_settings.difficulty = Difficulty::Normal;
        m_diffParams = difficultyParams(Difficulty::Normal);
        syncHudSettings();
        break;

    case MenuAction::GameOverSelectHard:
        m_difficultyIndex = 2;
        m_settings.difficulty = Difficulty::Hard;
        m_diffParams = difficultyParams(Difficulty::Hard);
        syncHudSettings();
        break;

    case MenuAction::GameOverSelectExtreme:
        m_difficultyIndex = 3;
        m_settings.difficulty = Difficulty::Extreme;
        m_diffParams = difficultyParams(Difficulty::Extreme);
        syncHudSettings();
        break;

    case MenuAction::GameOverExit:
        // Clean up and go back to main menu
        m_gameTimer->stop();
        for (auto *e : m_enemies) { m_scene->removeItem(e); delete e; }
        m_enemies.clear();
        for (auto *b : m_bullets) { m_scene->removeItem(b); delete b; }
        m_bullets.clear();
        for (auto *o : m_orbs) { m_scene->removeItem(o); delete o; }
        m_orbs.clear();
        m_upgradeSystem->hideUpgradeSelection();
        m_scene->removeItem(m_player);
        m_player->deleteLater();
        m_player = new Player();
        m_player->applyDifficulty(m_diffParams);
        m_scene->addItem(m_player);
        m_player->setPos(GameConstants::SCENE_WIDTH / 2.0,
                         GameConstants::SCENE_HEIGHT / 2.0);
        m_player->setUseArrowKeys(m_settings.useArrowKeys);
        m_view->setPlayer(m_player);
        m_waveManager->setPlayer(m_player);
        m_upgradeSystem->setPlayer(m_player);
        connect(m_player, &Player::bulletCreated, this, &Game::onBulletCreated);
        connect(m_player, &Player::playerDied, this, &Game::onPlayerDied);
        m_view->setGameOver(false, 0);
        m_view->setNewHighScore(false);
        m_settingsFromGameOver = false;
        enterState(GameState::MainMenu);
        m_gameTimer->start(GameConstants::TICK_INTERVAL_MS);
        break;

    // ── Main menu ───────────────────────────────────────
    case MenuAction::StartGame:
        startNewGame();
        break;

    case MenuAction::SelectEasy:
        m_difficultyIndex = 0;
        m_settings.difficulty = Difficulty::Easy;
        syncHudSettings();
        break;

    case MenuAction::SelectNormal:
        m_difficultyIndex = 1;
        m_settings.difficulty = Difficulty::Normal;
        syncHudSettings();
        break;

    case MenuAction::SelectHard:
        m_difficultyIndex = 2;
        m_settings.difficulty = Difficulty::Hard;
        syncHudSettings();
        break;

    case MenuAction::SelectExtreme:
        m_difficultyIndex = 3;
        m_settings.difficulty = Difficulty::Extreme;
        syncHudSettings();
        break;

    case MenuAction::Resume:
        enterState(GameState::Playing);
        break;

    case MenuAction::Settings:
        enterState(GameState::Settings);
        break;

    case MenuAction::ExitGame:
        enterState(GameState::MainMenu);
        break;

    case MenuAction::QuitGame:
        QApplication::quit();
        break;

    case MenuAction::ToggleKeys:
        m_settings.useArrowKeys = !m_settings.useArrowKeys;
        m_player->setUseArrowKeys(m_settings.useArrowKeys);
        syncHudSettings();
        break;

    case MenuAction::PrevTrack:
        if (!m_musicFiles.isEmpty()) {
            m_currentTrack = (m_currentTrack - 1 + m_musicFiles.size())
                             % m_musicFiles.size();
            m_bgmPlayer->setSource(
                QUrl::fromLocalFile(m_musicFiles.at(m_currentTrack)));
            m_bgmPlayer->play();
            QFileInfo fi(m_musicFiles.at(m_currentTrack));
            m_currentTrackName = fi.fileName();
            syncHudSettings();
        }
        break;

    case MenuAction::NextTrack:
        playNextSong();
        syncHudSettings();
        break;

    case MenuAction::Back:
        if (m_settingsFromGameOver) {
            m_settingsFromGameOver = false;
            enterState(GameState::GameOver);
        } else {
            enterState(GameState::Paused);
        }
        break;

    case MenuAction::Upgrade1:
    case MenuAction::Upgrade2:
    case MenuAction::Upgrade3:
        if (m_state == GameState::Upgrading) {
            int idx = static_cast<int>(action)
                      - static_cast<int>(MenuAction::Upgrade1);

            bool hadMagicCircle = m_player->hasMagicCircle();
            auto stat = m_upgradeSystem->currentUpgradeStat(idx);
            double val = m_upgradeSystem->currentUpgradeValue(idx);

            m_upgradeSystem->applyUpgrade(idx);

            // Create magic circle if it was just acquired
            if (!hadMagicCircle && m_player->hasMagicCircle()) {
                if (!m_magicCircle) {
                    m_magicCircle = new MagicCircle(m_player);
                    m_scene->addItem(m_magicCircle);
                }
            }

            // Apply magic circle upgrades to the MagicCircle object
            if (m_magicCircle) {
                if (stat == UpgradeDef::Stat::MagicCircleRadius) {
                    m_magicCircle->increaseRadius(val);
                } else if (stat == UpgradeDef::Stat::MagicCircleDamage) {
                    m_magicCircle->increaseDamage(static_cast<int>(val));
                }
            }

            m_upgradeSystem->hideUpgradeSelection();
            enterState(GameState::Playing);
            startNextWave();
        }
        break;

    default:
        break;
    }
}

void Game::handleVolumeDrag(QPointF scenePos)
{
    QRectF bar;
    if (m_state == GameState::MainMenu)
        bar = HUD::volumeBarRectMainMenu(m_view->viewport()->rect());
    else
        bar = HUD::volumeBarRect(m_view->viewport()->rect());

    double pct = (scenePos.x() - bar.left()) / bar.width();
    pct = qBound(0.0, pct, 1.0);
    m_settings.musicVolume = (float)pct;
    m_bgmOutput->setVolume(pct);
    syncHudSettings();
}

void Game::syncHudSettings()
{
    m_view->setHudSettings(m_settings.useArrowKeys,
                           m_currentTrackName, m_settings.musicVolume);
    m_view->setHudDifficultyIndex(m_difficultyIndex);
}

// ═══════════════════════════════════════════════════════════════
// Game loop phases (Playing state only)
// ═══════════════════════════════════════════════════════════════

void Game::processInput()
{
    m_player->setKeysHeld(m_view->keysPressed());
    m_player->updateMovement();
    m_player->updateAiming(m_view->mouseScenePos());

    // Camera follows player, clamped to world bounds
    m_view->centerOn(m_player->pos());

    if (m_view->isMouseHeld()) {
        m_player->tryFire();
    }

    m_player->updateInvincibility();
}

void Game::updateGameObjects()
{
    for (auto *b : m_bullets) b->advance(1);
    // Lasers are instant-hit — no advance needed
    for (auto *e : m_enemies) e->advance(1);
    for (auto *o : m_orbs)    o->advance(1);

    // Magic circle follows player + deals damage
    if (m_magicCircle) {
        m_magicCircle->updatePosition();
        m_magicCircle->checkDamage(m_enemies);
    }

    m_waveManager->update();

    int alive = 0;
    for (auto *e : m_enemies)
        if (!e->isDead()) alive++;
    m_view->setWaveInfo(m_currentWave,
        m_waveManager->enemiesRemainingToSpawn() + alive, alive);
}

void Game::handleCollisions()
{
    CollisionManager::processCollisions(
        m_scene, m_player, m_bullets, m_enemies, m_orbs, m_score);
}

void Game::cleanupDeadItems()
{
    for (int i = m_enemies.size() - 1; i >= 0; i--) {
        Enemy *e = m_enemies[i];
        if (e->isDead()) {
            ExperienceOrb *orb = new ExperienceOrb(
                e->pos(), e->xpValue(), m_player);
            m_scene->addItem(orb);
            m_orbs.append(orb);
            m_score += e->scoreValue();
            m_player->addScore(e->scoreValue());
            m_scene->removeItem(e);
            m_enemies.removeAt(i);
            delete e;
        }
    }
    for (int i = m_bullets.size() - 1; i >= 0; i--) {
        Bullet *b = m_bullets[i];
        if (b->isDead() || b->isExpired()) {
            m_scene->removeItem(b);
            m_bullets.removeAt(i);
            delete b;
        }
    }
    for (int i = m_lasers.size() - 1; i >= 0; i--) {
        Laser *l = m_lasers[i];
        if (l->isDead() || l->isExpired()) {
            m_scene->removeItem(l);
            m_lasers.removeAt(i);
            delete l;
        }
    }
    for (int i = m_orbs.size() - 1; i >= 0; i--) {
        ExperienceOrb *o = m_orbs[i];
        if (o->isDead()) {
            m_scene->removeItem(o);
            m_orbs.removeAt(i);
            delete o;
        }
    }
}

void Game::checkWaveTransition()
{
    int alive = 0;
    for (auto *e : m_enemies)
        if (!e->isDead()) alive++;

    if (m_waveManager->isWaveComplete() && alive == 0) {
        if (m_currentWave >= 2 && m_currentWave % 2 == 0) {
            m_upgradeSystem->showUpgradeSelection();
            enterState(GameState::Upgrading);
        } else {
            startNextWave();
        }
    }
}

void Game::startNextWave()
{
    m_currentWave++;
    m_waveManager->startWave(m_currentWave);
}

void Game::onPlayerDied()
{
    // Check/save high score
    m_isNewHighScore = false;
    if (m_score > m_highScores[m_difficultyIndex]) {
        m_highScores[m_difficultyIndex] = m_score;
        saveHighScores();
        m_isNewHighScore = true;
    }
    m_view->setHudHighScores(m_highScores);
    m_view->setNewHighScore(m_isNewHighScore);
    m_view->setGameOver(true, m_score);
    enterState(GameState::GameOver);
}

void Game::onBulletCreated(Bullet *bullet)
{
    m_scene->addItem(bullet);
    m_bullets.append(bullet);
}

void Game::onLaserCreated(Laser *laser)
{
    m_scene->addItem(laser);
    // Instant hit: check collision against all enemies
    laser->checkHit(m_enemies);
    m_lasers.append(laser);
}

void Game::onEnemyDied(Enemy *enemy)
{
    Q_UNUSED(enemy);
}

// ═══════════════════════════════════════════════════════════════
// High scores
// ═══════════════════════════════════════════════════════════════

void Game::loadHighScores()
{
    QFile f("highscores.txt");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    for (int i = 0; i < 4 && !in.atEnd(); i++) {
        in >> m_highScores[i];
    }
    f.close();
}

void Game::saveHighScores()
{
    QFile f("highscores.txt");
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&f);
    for (int i = 0; i < 4; i++) {
        out << m_highScores[i];
        if (i < 3) out << " ";
    }
    f.close();
}

// ═══════════════════════════════════════════════════════════════
// Music
// ═══════════════════════════════════════════════════════════════

void Game::playNextSong()
{
    if (m_musicFiles.isEmpty()) return;

    // Sequential cycle
    m_currentTrack = (m_currentTrack + 1) % m_musicFiles.size();

    m_bgmPlayer->setSource(QUrl::fromLocalFile(m_musicFiles.at(m_currentTrack)));
    m_bgmPlayer->play();

    QFileInfo fi(m_musicFiles.at(m_currentTrack));
    m_currentTrackName = fi.fileName();
}
