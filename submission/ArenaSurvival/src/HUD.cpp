#include "HUD.h"
#include "Player.h"
#include "Constants.h"

#include <QFont>
#include <QPen>
#include <QPixmap>
#include <QCoreApplication>
#include <QFileInfo>

// ═══════════════════════════════════════════════════════════════
// Main entry
// ═══════════════════════════════════════════════════════════════

void HUD::draw(QPainter *p, const QRectF &vr, const Player *player,
               int wave, int rem, int alive,
               bool gameOver, int finalScore,
               const QString &stateName,
               bool useArrowKeys,
               const QString &trackName, float volume,
               const QStringList &upgradeNames,
               int difficultyIndex,
               const int highScores[4],
               bool isNewHighScore)
{
    if (!player) return;

    // ── Main Menu ────────────────────────────────────────────
    if (stateName == "mainmenu") {
        drawMainMenu(p, vr, useArrowKeys, trackName, volume,
                     difficultyIndex, highScores);
        return;
    }

    if (stateName == "gameover") {
        drawGameOver(p, vr, finalScore, highScores, difficultyIndex,
                     isNewHighScore);
        return;
    }

    // Always draw gameplay HUD underneath
    drawHealthBar(p, 12, 8, 200, 18, player->hp(), player->maxHp());
    drawScore(p, vr, player->score());
    drawWaveInfo(p, vr, wave, rem, alive);
    drawXPBar(p, 12, vr.height() - 28, 200, 14,
              player->xp(), player->xpToNextLevel());

    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 10, QFont::Bold));
    p->drawText(QRectF(12, vr.height() - 48, 200, 20),
                Qt::AlignLeft | Qt::AlignVCenter,
                QString("Level %1").arg(player->level()));

    // Menu overlays
    if (stateName == "paused")
        drawPauseMenu(p, vr);
    else if (stateName == "settings")
        drawSettingsMenu(p, vr, useArrowKeys, trackName, volume);
    else if (stateName == "upgrading")
        drawUpgradeMenu(p, vr, upgradeNames);
}

// ═══════════════════════════════════════════════════════════════
// Hit testing
// ═══════════════════════════════════════════════════════════════

MenuAction HUD::hitTest(QPointF clickPos, const QRectF &vr,
                         const QString &stateName,
                         const QStringList &upgradeNames,
                         const QString & /*trackName*/,
                         float /*volume*/,
                         int difficultyIndex,
                         const int * /*highScores*/)
{
    // ── Game Over ──────────────────────────────────────
    if (stateName == "gameover") {
        double cx = vr.center().x();
        double cy = vr.center().y();
        const double BTN_START_Y = cy + 36;
        const double BTN_H       = 38;
        const double BTN_GAP     = 46;
        const double BTN_W       = 260;

        double by = BTN_START_Y;
        // Restart
        if (QRectF(cx - BTN_W / 2, by, BTN_W, BTN_H).contains(clickPos))
            return MenuAction::GameOverRestart;
        by += BTN_GAP;
        // Settings
        if (QRectF(cx - BTN_W / 2, by, BTN_W, BTN_H).contains(clickPos))
            return MenuAction::GameOverSettings;
        by += BTN_GAP;
        // Difficulty buttons
        double dbtnW = 56;
        double dspacing = 6;
        double dtotalW = dbtnW * 4 + dspacing * 3;
        double dstartX = cx - dtotalW / 2.0;
        double dY = by + 18;
        for (int i = 0; i < 4; i++) {
            QRectF db(dstartX + i * (dbtnW + dspacing), dY, dbtnW, 28);
            if (db.contains(clickPos)) {
                return static_cast<MenuAction>(
                    static_cast<int>(MenuAction::GameOverSelectEasy) + i);
            }
        }
        by += BTN_GAP + 4;
        // Exit to Menu
        if (QRectF(cx - BTN_W / 2, by, BTN_W, BTN_H).contains(clickPos))
            return MenuAction::GameOverExit;
        return MenuAction::None;
    }
    // ── Main Menu ───────────────────────────────────────
    if (stateName == "mainmenu") {
        double cx = vr.center().x();
        // Start Game button
        if (QRectF(cx - 110, 125, 220, 50).contains(clickPos))
            return MenuAction::StartGame;
        // Controls toggle
        if (QRectF(cx - 180, 200, 360, 42).contains(clickPos))
            return MenuAction::ToggleKeys;
        // Prev track
        if (QRectF(cx - 180, 268, 70, 40).contains(clickPos))
            return MenuAction::PrevTrack;
        // Next track
        if (QRectF(cx + 110, 268, 70, 40).contains(clickPos))
            return MenuAction::NextTrack;
        // Volume bar
        QRectF vb = volumeBarRectMainMenu(vr);
        if (vb.adjusted(0, -8, 0, 8).contains(clickPos))
            return MenuAction::VolumeBar;
        // Difficulty buttons
        double diffY = 440;
        double btnW = 130;
        double totalW = btnW * 4 + 16 * 3;
        double diffStartX = cx - totalW / 2.0;
        for (int i = 0; i < 4; i++) {
            QRectF btn(diffStartX + i * (btnW + 16), diffY, btnW, 42);
            if (btn.contains(clickPos)) {
                return static_cast<MenuAction>(
                    static_cast<int>(MenuAction::SelectEasy) + i);
            }
        }
        // Quit Game button
        if (QRectF(cx - 110, 555, 220, 42).contains(clickPos))
            return MenuAction::QuitGame;
        return MenuAction::None;
    }
    // Common: check upgrade buttons when in upgrading state
    if (stateName == "upgrading") {
        double startY = vr.center().y() - 80;
        for (int i = 0; i < upgradeNames.size() && i < 3; i++) {
            QRectF btn(vr.center().x() - 150, startY + i * 55 - 22, 300, 44);
            if (btn.contains(clickPos))
                return static_cast<MenuAction>(
                    static_cast<int>(MenuAction::Upgrade1) + i);
        }
        return MenuAction::None;
    }

    if (stateName == "paused") {
        double cy = vr.center().y();
        double cx = vr.center().x();
        // Resume
        if (QRectF(cx - 120, cy - 60, 240, 42).contains(clickPos))
            return MenuAction::Resume;
        // Settings
        if (QRectF(cx - 120, cy - 8, 240, 42).contains(clickPos))
            return MenuAction::Settings;
        // Exit
        if (QRectF(cx - 120, cy + 44, 240, 42).contains(clickPos))
            return MenuAction::ExitGame;
        return MenuAction::None;
    }

    if (stateName == "settings") {
        double cy = vr.center().y();
        double cx = vr.center().x();
        // Toggle keys
        if (QRectF(cx - 180, cy - 85, 360, 42).contains(clickPos))
            return MenuAction::ToggleKeys;
        // Prev track
        if (QRectF(cx - 180, cy - 10, 70, 40).contains(clickPos))
            return MenuAction::PrevTrack;
        // Next track
        if (QRectF(cx + 110, cy - 10, 70, 40).contains(clickPos))
            return MenuAction::NextTrack;
        // Volume bar
        QRectF vb = volumeBarRect(vr);
        if (vb.adjusted(0, -8, 0, 8).contains(clickPos))
            return MenuAction::VolumeBar;
        // Back
        if (QRectF(cx - 120, cy + 100, 240, 42).contains(clickPos))
            return MenuAction::Back;
        return MenuAction::None;
    }

    return MenuAction::None;
}

QRectF HUD::volumeBarRect(const QRectF &vr)
{
    double cx = vr.center().x();
    double cy = vr.center().y() + 68;
    return QRectF(cx - 140, cy, 280, 16);
}

// ═══════════════════════════════════════════════════════════════
// Gameplay HUD
// ═══════════════════════════════════════════════════════════════

void HUD::drawHealthBar(QPainter *p, qreal x, qreal y,
                         qreal w, qreal h, int cur, int max)
{
    p->fillRect(QRectF(x, y, w, h), QColor(40, 0, 0, 200));
    double pct = qBound(0.0, (double)cur / max, 1.0);
    QColor fc;
    if (pct > 0.5)
        fc = QColor::fromRgbF(qMin(1.0, (1.0 - pct) * 2.0), 1.0, 0.0);
    else
        fc = QColor::fromRgbF(1.0, pct * 2.0, 0.0);
    if (cur > 0) p->fillRect(QRectF(x, y, w * pct, h), fc);
    p->setPen(QPen(QColor(180, 180, 180), 1));
    p->drawRect(QRectF(x, y, w, h));
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 9, QFont::Bold));
    p->drawText(QRectF(x, y, w, h), Qt::AlignCenter,
                QString("HP: %1/%2").arg(cur).arg(max));
}

void HUD::drawXPBar(QPainter *p, qreal x, qreal y,
                     qreal w, qreal h, int cur, int needed)
{
    p->fillRect(QRectF(x, y, w, h), QColor(0, 0, 40, 200));
    double pct = qBound(0.0, (double)cur / needed, 1.0);
    if (cur > 0) p->fillRect(QRectF(x, y, w * pct, h), QColor(60, 120, 220));
    p->setPen(QPen(QColor(120, 160, 220), 1));
    p->drawRect(QRectF(x, y, w, h));
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 8, QFont::Bold));
    p->drawText(QRectF(x, y, w, h), Qt::AlignCenter,
                QString("XP: %1/%2").arg(cur).arg(needed));
}

void HUD::drawScore(QPainter *p, const QRectF &r, int score)
{
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 12, QFont::Bold));
    p->drawText(QRectF(0, 8, r.width() - 12, 20),
                Qt::AlignRight | Qt::AlignVCenter,
                QString("Score: %1").arg(score));
}

void HUD::drawWaveInfo(QPainter *p, const QRectF &r,
                        int wave, int rem, int alive)
{
    p->setPen(QColor(255, 220, 60));
    p->setFont(QFont("Arial", 11, QFont::Bold));
    p->drawText(QRectF(0, 8, r.width(), 20), Qt::AlignCenter,
                QString("WAVE %1").arg(wave));
    p->setPen(QColor(200, 200, 200));
    p->setFont(QFont("Arial", 8));
    p->drawText(QRectF(0, 26, r.width(), 16), Qt::AlignCenter,
                QString("Enemies: %1 left  |  %2 alive").arg(rem).arg(alive));
}

void HUD::drawGameOver(QPainter *p, const QRectF &r, int score,
                        const int highScores[4], int difficultyIndex,
                        bool isNewHighScore)
{
    p->fillRect(r, QColor(0, 0, 0, 210));

    // ── Load meme image (cached) ──────────────────────────
    static QPixmap s_meme;
    static bool s_triedLoad = false;
    if (!s_triedLoad) {
        s_triedLoad = true;
        QStringList paths;
        paths << "pictures/caijiuduolian.jpeg"
              << "../pictures/caijiuduolian.jpeg"
              << QCoreApplication::applicationDirPath() + "/pictures/caijiuduolian.jpeg"
              << QCoreApplication::applicationDirPath() + "/../pictures/caijiuduolian.jpeg"
              << QCoreApplication::applicationDirPath() + "/../../pictures/caijiuduolian.jpeg";
        for (const auto &p : paths) {
            if (QFileInfo::exists(p)) {
                s_meme.load(p);
                break;
            }
        }
    }

    double cx = r.center().x();
    double cy = r.center().y();

    // ── Meme image (centered in upper area) ───────────────
    if (!s_meme.isNull()) {
        double imgH = qMin(200.0, (double)s_meme.height());
        QPixmap scaled = s_meme.scaledToHeight(
            static_cast<int>(imgH), Qt::SmoothTransformation);
        double imgX = cx - scaled.width() / 2.0;
        double imgY = cy - 260;
        p->drawPixmap(static_cast<int>(imgX), static_cast<int>(imgY), scaled);
    }

    // ── Fixed layout from center ──────────────────────────
    const double SCORE_Y     = cy - 40;
    const double HS_Y        = cy - 14;
    const double BTN_START_Y = cy + 36;
    const double BTN_H       = 38;
    const double BTN_GAP     = 46;
    const double BTN_W       = 260;

    // ── New High Score banner ────────────────────────────
    if (isNewHighScore) {
        double bannerY = s_meme.isNull() ? cy - 150 : cy - 295;
        p->setFont(QFont("Arial", 22, QFont::Bold));
        p->setPen(QColor(255, 220, 40));
        p->drawText(QRectF(0, bannerY, r.width(), 30),
                    Qt::AlignCenter, "NEW HIGH SCORE! MAN!");
    }

    // Score
    p->setFont(QFont("Arial", 16, QFont::Bold));
    p->setPen(Qt::white);
    p->drawText(QRectF(0, SCORE_Y, r.width(), 24),
                Qt::AlignCenter,
                QString("Score: %1").arg(score));

    // High Scores
    const QStringList diffNames = {
        QString::fromUtf8("简单"), QString::fromUtf8("普通"),
        QString::fromUtf8("困难"), QString::fromUtf8("逆天")
    };
    p->setFont(QFont("Arial", 8, QFont::Bold));
    p->setPen(QColor(180, 180, 180));
    p->drawText(QRectF(0, HS_Y, r.width(), 16),
                Qt::AlignCenter, "High Scores");
    p->setFont(QFont("Arial", 8));
    for (int i = 0; i < 4; i++) {
        double hx = cx - 260 + i * 130;
        p->setPen(i == difficultyIndex ? QColor(255, 220, 60) : QColor(160, 160, 160));
        p->drawText(QRectF(hx, HS_Y + 16, 130, 16),
                    Qt::AlignCenter,
                    QString("%1: %2").arg(diffNames[i]).arg(highScores[i]));
    }

    // Buttons
    double by = BTN_START_Y;
    drawMenuButton(p, QRectF(cx - BTN_W / 2, by, BTN_W, BTN_H), 0, "Restart", false);

    by += BTN_GAP;
    drawMenuButton(p, QRectF(cx - BTN_W / 2, by, BTN_W, BTN_H), 0, "Settings", false);

    by += BTN_GAP;
    // Difficulty label
    p->setPen(QColor(180, 180, 180));
    p->setFont(QFont("Arial", 10, QFont::Bold));
    p->drawText(QRectF(cx - BTN_W / 2, by, BTN_W, 16),
                Qt::AlignCenter, "Difficulty:");

    // Difficulty buttons
    const QColor diffColors[4] = {
        QColor(60, 180, 60), QColor(220, 180, 40),
        QColor(220, 100, 30), QColor(220, 40, 40)
    };
    double dbtnW = 56;
    double dspacing = 6;
    double dtotalW = dbtnW * 4 + dspacing * 3;
    double dstartX = cx - dtotalW / 2.0;
    double dY = by + 18;
    for (int i = 0; i < 4; i++) {
        QRectF db(dstartX + i * (dbtnW + dspacing), dY, dbtnW, 28);
        bool sel = (i == difficultyIndex);
        p->setBrush(sel ? QColor(60, 60, 100, 220) : QColor(20, 20, 40, 200));
        p->setPen(QPen(sel ? diffColors[i] : QColor(80, 80, 120), sel ? 2 : 1));
        p->drawRoundedRect(db, 4, 4);
        p->setPen(sel ? diffColors[i] : QColor(160, 160, 160));
        p->setFont(QFont("Arial", 8, sel ? QFont::Bold : QFont::Normal));
        p->drawText(db, Qt::AlignCenter, diffNames[i]);
    }

    by += BTN_GAP + 4;
    drawMenuButton(p, QRectF(cx - BTN_W / 2, by, BTN_W, BTN_H), 0, "Exit to Menu", false);

    // R key hint
    p->setPen(QColor(120, 120, 120));
    p->setFont(QFont("Arial", 9));
    p->drawText(QRectF(0, r.height() - 26, r.width(), 18),
                Qt::AlignCenter, "Press R to Restart");
}

// ═══════════════════════════════════════════════════════════════
// Menu helpers
// ═══════════════════════════════════════════════════════════════

void HUD::drawMenuOverlay(QPainter *p, const QRectF &r)
{
    p->fillRect(r, QColor(0, 0, 0, 160));
}

void HUD::drawMenuTitle(QPainter *p, const QRectF &r, const QString &title)
{
    p->setFont(QFont("Arial", 28, QFont::Bold));
    p->setPen(QColor(255, 220, 60));
    QRectF tr = r;
    tr.setTop(r.center().y() - 150);
    tr.setHeight(40);
    p->drawText(tr, Qt::AlignCenter, title);
}

QRectF HUD::drawMenuButton(QPainter *p, const QRectF &r,
                            int index, const QString &text, bool highlight)
{
    Q_UNUSED(index);
    p->setBrush(highlight ? QColor(60, 60, 100, 200) : QColor(20, 20, 40, 200));
    p->setPen(QPen(highlight ? QColor(120, 140, 220) : QColor(80, 80, 120), 2));
    p->drawRoundedRect(r, 6, 6);
    p->setPen(Qt::white);
    p->setFont(QFont("Arial", 13, QFont::Bold));
    p->drawText(r, Qt::AlignCenter, text);
    return r;
}

// ═══════════════════════════════════════════════════════════════
// Pause Menu
// ═══════════════════════════════════════════════════════════════

void HUD::drawPauseMenu(QPainter *p, const QRectF &vr)
{
    drawMenuOverlay(p, vr);

    double cx = vr.center().x();
    double cy = vr.center().y();

    drawMenuTitle(p, vr, "GAME PAUSED");

    drawMenuButton(p, QRectF(cx - 120, cy - 60, 240, 42), 0, "Resume",    false);
    drawMenuButton(p, QRectF(cx - 120, cy - 8,  240, 42), 1, "Settings",  false);
    drawMenuButton(p, QRectF(cx - 120, cy + 44, 240, 42), 2, "Exit Game", false);

    p->setPen(QColor(160, 160, 160));
    p->setFont(QFont("Arial", 9));
    p->drawText(QRectF(0, vr.height() - 30, vr.width(), 20),
                Qt::AlignCenter, "Press ESC to resume");
}

// ═══════════════════════════════════════════════════════════════
// Settings Menu
// ═══════════════════════════════════════════════════════════════

void HUD::drawSettingsMenu(QPainter *p, const QRectF &vr,
                            bool useArrowKeys,
                            const QString &trackName,
                            float volume)
{
    drawMenuOverlay(p, vr);

    double cx = vr.center().x();
    double cy = vr.center().y();

    drawMenuTitle(p, vr, "SETTINGS");

    // ── Key binding toggle ───────────────────────────────
    QString keyText = useArrowKeys
        ? "Controls: Arrow Keys (click to switch)"
        : "Controls: WASD (click to switch)";
    drawMenuButton(p, QRectF(cx - 180, cy - 85, 360, 42),
                   0, keyText, false);

    // ── Music track ──────────────────────────────────────
    p->setPen(QColor(200, 200, 200));
    p->setFont(QFont("Arial", 11, QFont::Bold));
    p->drawText(QRectF(cx - 180, cy - 38, 360, 18),
                Qt::AlignCenter, "Music Track:");

    p->setFont(QFont("Arial", 9));
    p->setPen(QColor(150, 200, 255));
    p->drawText(QRectF(cx - 80, cy - 10, 160, 40),
                Qt::AlignCenter, trackName.isEmpty() ? "No track" : trackName);

    drawMenuButton(p, QRectF(cx - 180, cy - 10, 70, 40), 0, "<",  false);
    drawMenuButton(p, QRectF(cx + 110, cy - 10, 70, 40), 1, ">", false);

    // ── Volume slider ────────────────────────────────────
    p->setPen(QColor(200, 200, 200));
    p->setFont(QFont("Arial", 12, QFont::Bold));
    p->drawText(QRectF(cx - 180, cy + 42, 360, 22),
                Qt::AlignCenter,
                QString("Volume: %1%").arg((int)(volume * 100)));

    // Slider bar
    QRectF bar = volumeBarRect(vr);
    p->fillRect(bar, QColor(40, 40, 40));
    QRectF fill = bar;
    fill.setWidth(bar.width() * volume);
    p->fillRect(fill, QColor(80, 160, 240));
    p->setPen(QPen(QColor(140, 140, 140), 1));
    p->drawRect(bar);

    // Knob
    double knobX = bar.left() + bar.width() * volume;
    p->setBrush(Qt::white);
    p->drawEllipse(QPointF(knobX, bar.center().y()), 8, 8);

    // ── Back button ──────────────────────────────────────
    drawMenuButton(p, QRectF(cx - 120, cy + 100, 240, 42),
                   0, "Back", false);
}

// ═══════════════════════════════════════════════════════════════
// Upgrade Menu
// ═══════════════════════════════════════════════════════════════

void HUD::drawUpgradeMenu(QPainter *p, const QRectF &vr,
                           const QStringList &names)
{
    drawMenuOverlay(p, vr);
    drawMenuTitle(p, vr, "CHOOSE UPGRADE");

    double cx = vr.center().x();
    double startY = vr.center().y() - 80;
    const double spacing = 55;

    for (int i = 0; i < names.size() && i < 3; i++) {
        QRectF btn(cx - 150, startY + i * spacing - 22, 300, 44);
        drawMenuButton(p, btn, i, names[i], false);
    }

    p->setPen(QColor(160, 160, 160));
    p->setFont(QFont("Arial", 9));
    p->drawText(QRectF(0, vr.height() - 30, vr.width(), 20),
                Qt::AlignCenter, "Click an upgrade to select");
}

// ═══════════════════════════════════════════════════════════════
// Main Menu
// ═══════════════════════════════════════════════════════════════

void HUD::drawMainMenu(QPainter *p, const QRectF &vr,
                       bool useArrowKeys,
                       const QString &trackName, float volume,
                       int difficultyIndex,
                       const int highScores[4])
{
    drawMenuOverlay(p, vr);

    double cx = vr.center().x();

    // ── Title ────────────────────────────────────────────
    p->setFont(QFont("Arial", 40, QFont::Bold));
    p->setPen(QColor(255, 220, 60));
    p->drawText(QRectF(0, 20, vr.width(), 60),
                Qt::AlignCenter, "ARENA SURVIVOR");

    // ── Start Game ───────────────────────────────────────
    {
        QRectF btn(cx - 110, 125, 220, 50);
        p->setBrush(QColor(30, 100, 30, 220));
        p->setPen(QPen(QColor(60, 200, 60), 3));
        p->drawRoundedRect(btn, 8, 8);
        p->setFont(QFont("Arial", 16, QFont::Bold));
        p->setPen(Qt::white);
        p->drawText(btn, Qt::AlignCenter, "START GAME");
    }

    // ── Controls toggle ──────────────────────────────────
    QString keyText = useArrowKeys
        ? "Controls: Arrow Keys  (click to switch)"
        : "Controls: WASD  (click to switch)";
    drawMenuButton(p, QRectF(cx - 180, 200, 360, 42), 0, keyText, false);

    // ── Music track ──────────────────────────────────────
    p->setPen(QColor(200, 200, 200));
    p->setFont(QFont("Arial", 11, QFont::Bold));
    p->drawText(QRectF(cx - 180, 248, 360, 18),
                Qt::AlignCenter, "Music Track");

    // Track name + prev/next on one row
    p->setFont(QFont("Arial", 9));
    p->setPen(QColor(150, 200, 255));
    p->drawText(QRectF(cx - 80, 268, 160, 40),
                Qt::AlignCenter, trackName.isEmpty() ? "No track" : trackName);

    drawMenuButton(p, QRectF(cx - 180, 268, 70, 40), 0, "<",  false);
    drawMenuButton(p, QRectF(cx + 110, 268, 70, 40), 1, ">", false);

    // ── Volume slider ────────────────────────────────────
    p->setPen(QColor(200, 200, 200));
    p->setFont(QFont("Arial", 12, QFont::Bold));
    p->drawText(QRectF(cx - 140, 315, 280, 22),
                Qt::AlignCenter,
                QString("Volume: %1%").arg((int)(volume * 100)));

    QRectF bar = volumeBarRectMainMenu(vr);
    p->fillRect(bar, QColor(40, 40, 40));
    QRectF fill = bar;
    fill.setWidth(bar.width() * volume);
    p->fillRect(fill, QColor(80, 160, 240));
    p->setPen(QPen(QColor(140, 140, 140), 1));
    p->drawRect(bar);

    // Knob
    double knobX = bar.left() + bar.width() * volume;
    p->setBrush(Qt::white);
    p->drawEllipse(QPointF(knobX, bar.center().y()), 8, 8);

    // ── Difficulty selection ─────────────────────────────
    p->setPen(QColor(200, 200, 200));
    p->setFont(QFont("Arial", 13, QFont::Bold));
    p->drawText(QRectF(cx - 140, 400, 280, 24),
                Qt::AlignCenter, "Difficulty");

    const QStringList diffNames = {
        QString::fromUtf8("简单"),    // 简单 Easy
        QString::fromUtf8("普通"),    // 普通 Normal
        QString::fromUtf8("困难"),    // 困难 Hard
        QString::fromUtf8("逆天")     // 逆天 Extreme
    };
    const QColor diffColors[4] = {
        QColor(60, 180, 60),   // Easy — green
        QColor(220, 180, 40),  // Normal — yellow
        QColor(220, 100, 30),  // Hard — orange
        QColor(220, 40, 40)    // Extreme — red
    };

    double btnW = 130;
    double spacing = 16;
    double totalW = btnW * 4 + spacing * 3;
    double startX = cx - totalW / 2.0;
    double diffY = 440;

    for (int i = 0; i < 4; i++) {
        QRectF btn(startX + i * (btnW + spacing), diffY, btnW, 42);
        bool selected = (i == difficultyIndex);
        p->setBrush(selected ? QColor(60, 60, 100, 220)
                             : QColor(20, 20, 40, 200));
        p->setPen(QPen(selected ? diffColors[i] : QColor(80, 80, 120),
                       selected ? 3 : 2));
        p->drawRoundedRect(btn, 6, 6);
        p->setPen(selected ? diffColors[i] : QColor(180, 180, 180));
        p->setFont(QFont("Arial", 11, selected ? QFont::Bold : QFont::Normal));
        p->drawText(btn, Qt::AlignCenter, diffNames[i]);
    }

    // ── High Scores ──────────────────────────────────────
    p->setPen(QColor(200, 200, 200));
    p->setFont(QFont("Arial", 11, QFont::Bold));
    p->drawText(QRectF(cx - 140, 494, 280, 20),
                Qt::AlignCenter, "High Scores");
    p->setFont(QFont("Arial", 9));
    for (int i = 0; i < 4; i++) {
        double hsx = cx - 260 + i * 130;
        bool isCurrentDiff = (i == difficultyIndex);
        p->setPen(isCurrentDiff ? QColor(255, 220, 60) : QColor(160, 160, 160));
        p->drawText(QRectF(hsx, 516, 130, 18),
                    Qt::AlignCenter,
                    QString("%1: %2").arg(diffNames[i]).arg(highScores[i]));
    }

    // ── Quit Game ────────────────────────────────────────
    {
        QRectF btn(cx - 110, 555, 220, 42);
        p->setBrush(QColor(40, 20, 20, 200));
        p->setPen(QPen(QColor(200, 60, 60), 2));
        p->drawRoundedRect(btn, 6, 6);
        p->setFont(QFont("Arial", 13, QFont::Bold));
        p->setPen(QColor(220, 100, 100));
        p->drawText(btn, Qt::AlignCenter, "Quit Game");
    }

    // ── Hint ─────────────────────────────────────────────
    p->setPen(QColor(120, 120, 120));
    p->setFont(QFont("Arial", 9));
    p->drawText(QRectF(0, vr.height() - 28, vr.width(), 20),
                Qt::AlignCenter, "Click buttons to interact  |  ESC to quit");
}

QRectF HUD::volumeBarRectMainMenu(const QRectF &vr)
{
    double cx = vr.center().x();
    return QRectF(cx - 140, 340, 280, 16);
}
