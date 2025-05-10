
//
// Created by David Burchill on 2023-09-27.
//

#include <fstream>
#include <iostream>

#include "Scene_BulletNinja.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include <random>

void Scene_BulletNinja::spawnTile(sf::Vector2f pos, sf::Vector2f size) {
    auto enemy = _entityManager.addEntity("tile");
    enemy->addComponent<CTransform>(pos);
    enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("tile"));
    enemy->addComponent<CBoundingBox>(size);

}

void Scene_BulletNinja::spawnTiles() {
    while (!_spawnTilePoints.empty()) {
        const auto& point = _spawnTilePoints.top();
        spawnTile(sf::Vector2f{ point.x, point.y }, sf::Vector2f{ point.width, point.height });
        _spawnTilePoints.pop();
    }
}