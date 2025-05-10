
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

void Scene_BulletNinja::spawnFlowerEnemy(sf::Vector2f pos, sf::Vector2f size) {
    auto enemy = _entityManager.addEntity("flowerEnemy");
    enemy->addComponent<CTransform>(pos, sf::Vector2f(.5f, 0));
    enemy->addComponent<CState>();
    enemy->addComponent<CHitEffect>(sf::seconds(0.3f));
    auto& anim = enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("flowerattack"));
    enemy->addComponent<CBoundingBox>(size);
    enemy->addComponent<CScore>(1);
    enemy->addComponent<CRespawn>(10);

    enemy -> getComponent<CState>().set(CState::isAttacking);


    anim.onAnimationEnd = [this, enemy]() {
        if (!enemy->isActive()) return;

        auto& state = enemy->getComponent<CState>();
        auto& anim = enemy->getComponent<CAnimation>();

        anim.setAnimation();
        
        };
    
}

void Scene_BulletNinja::spawnFlowerEnemies() {
    while (!_spawnFlowerEnemyPoints.empty()) {
        const auto& point = _spawnFlowerEnemyPoints.top();
        spawnFlowerEnemy(sf::Vector2f{ point.x, m_ground.pos.y - point.height -10.f }, sf::Vector2f{ point.width, point.height });
        _spawnFlowerEnemyPoints.pop();
    }
}