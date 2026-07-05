cmake_minimum_required(VERSION 3.16)
project(BrotatoArenaSurvival VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/GameView.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Bullet.cpp
    src/ExperienceOrb.cpp
    src/HUD.cpp
    src/WaveManager.cpp
    src/CollisionManager.cpp
    src/UpgradeSystem.cpp
    src/Laser.cpp
    src/TextureManager.cpp
    src/MagicCircle.cpp
)

set(HEADERS
    src/Constants.h
    src/Game.h
    src/GameView.h
    src/Player.h
    src/Enemy.h
    src/Bullet.h
    src/Laser.h
    src/TextureManager.h
    src/MagicCircle.h
    src/ExperienceOrb.h
    src/HUD.h
    src/WaveManager.h
    src/CollisionManager.h
    src/UpgradeSystem.h
)

qt_add_executable(BrotatoArenaSurvival ${SOURCES} ${HEADERS})

target_include_directories(BrotatoArenaSurvival PRIVATE src)

target_link_libraries(BrotatoArenaSurvival PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)
