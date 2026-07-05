#include <QApplication>
#include "Game.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Arena Survivor");

    Game game;
    game.start();

    return app.exec();
}
