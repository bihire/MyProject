
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



void Scene_BulletNinja::spawnEnemy(sf::Vector2f pos) {
    auto enemy = _entityManager.addEntity("enemy");
    enemy->addComponent<CTransform>(pos).scale = sf::Vector2f(4.f, 4.f);
    enemy->addComponent<CState>();
    enemy->addComponent<CBoundingBox>(sf::Vector2f(52.f, 102.f));
    enemy->addComponent<CInput>();
    enemy->addComponent<CHitEffect>(sf::seconds(0.3f));
    enemy->addComponent<CScore>(1, 30);
    enemy->addComponent<CHealthBarIndicator>();
    auto& anim = enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));


    anim.onAnimationEnd = [this, enemy]() {
        if (!enemy->isActive()) return;

        auto& state = enemy->getComponent<CState>();
        auto& anim = enemy->getComponent<CAnimation>();

        if (state.test(CState::isAttackSword) ) {

            state.unSet(CState::isAttacking);
            state.unSet(CState::isAttackSword);
            state.unSet(CState::isAttackSpear);


            anim.setAnimation(Assets::getInstance().getAnimation("SamuraiIdle"));
        }
        };


    enemy->getComponent<CTransform>().vel = sf::Vector2f(0, 0);
}


void Scene_BulletNinja::spawnNinjaEnemies() {
    while (!_spawnEnemyNinjaPoints.empty()) {
        const auto& point = _spawnEnemyNinjaPoints.top();
        spawnEnemy(sf::Vector2f{ point.x, m_ground.pos.y - 200.f });
        _spawnEnemyNinjaPoints.pop();
    }
}



void Scene_BulletNinja::enemyMovement(sf::Time dt){
    if (!m_player) return;

    const sf::Vector2f playerPos = m_player->getComponent<CTransform>().pos;

    for (auto& enemy : _entityManager.getEntities("enemy")) {
        if (!enemy->isActive()) continue;

        handleEnemyState(enemy, playerPos);
        //updateEnemyAnimation(enemy);
    }
}

void Scene_BulletNinja::handleEnemyState(std::shared_ptr<Entity> enemy, const sf::Vector2f& playerPos) {
    auto& transform = enemy->getComponent<CTransform>();
    auto& state = enemy->getComponent<CState>();

    transform.vel.x = 0; // reset vel

    if (!shouldProcessEnemy(enemy)) {
        setEnemyIdle(enemy);
        return;
    }

    const float xDistance = playerPos.x - transform.pos.x;

    if (shouldAttack(enemy, xDistance)) {
        setEnemyAttackState(enemy);
    }
    else if (shouldChase(enemy, xDistance)) {
        setEnemyChaseState(enemy, xDistance);
    }
    else {
        setEnemyIdle(enemy);
    }
}

bool Scene_BulletNinja::shouldProcessEnemy(std::shared_ptr<Entity> enemy) {
    // Check if enemy is in view
    const auto& transform = enemy->getComponent<CTransform>();
    if (std::abs(transform.pos.x - m_worldView.getCenter().x) > m_worldView.getSize().x / 2) {
        return false;
    }

    // is player ative?
    if (!m_player->isActive()) return false;

    // obstacles between enemy and player
    return !hasObstacleBetween(enemy, m_player->getComponent<CTransform>().pos);
}

bool Scene_BulletNinja::shouldAttack(std::shared_ptr<Entity> enemy, float xDistance) {
    return std::abs(xDistance) <= m_config.enemyAttackRange;
}

bool Scene_BulletNinja::shouldChase(std::shared_ptr<Entity> enemy, float xDistance) {
    return std::abs(xDistance) <= m_config.enemyChaseRange;
}


void Scene_BulletNinja::setEnemyAttackState(std::shared_ptr<Entity> enemy) {
    auto& state = enemy->getComponent<CState>();
    auto& anim = enemy->getComponent<CAnimation>();

    if (!state.test(CState::isAttackSword)) {
        state.set(CState::isAttackSword);
        state.unSet(CState::isRunning);
        anim.setAnimation(Assets::getInstance().getAnimation("SamuraiAttackSword"));
    }
}

void Scene_BulletNinja::setEnemyChaseState(std::shared_ptr<Entity> enemy, float xDistance) {
    auto& transform = enemy->getComponent<CTransform>();
    auto& state = enemy->getComponent<CState>();
    auto& anim = enemy->getComponent<CAnimation>();
    auto& ai = enemy->getComponent<CEnemyAI>();

    // Detect sudden direction change
    bool playerChangedSides = (xDistance > 0 && !ai.wasFacingRight) ||
        (xDistance < 0 && ai.wasFacingRight);

    // Always update facing direction
    bool shouldFaceRight = xDistance > 0;
    transform.scale.x = shouldFaceRight ? std::abs(transform.scale.x) : -std::abs(transform.scale.x);

    // Check path clearance (with direction change consideration)
    bool canChase = true;
    if (!playerChangedSides) {
        canChase = canMoveForward(enemy);
    }

    if (canChase) {
        state.set(CState::isRunning);
        transform.vel.x = shouldFaceRight ? m_config.enemySpeed : -m_config.enemySpeed;

        if (anim.animation.getName() != "SamuraiRun") {
            anim.setAnimation(Assets::getInstance().getAnimation("SamuraiRun"));
        }
    }
    else {
        
        transform.vel.x = playerChangedSides ?
            (shouldFaceRight ? m_config.enemySpeed / 2 : -m_config.enemySpeed / 2) : 0;

        if (transform.vel.x == 0) {
            setEnemyIdle(enemy);
        }
    }

    // Update AI memory
    ai.lastPlayerX = xDistance;
    ai.wasFacingRight = shouldFaceRight;
}

void Scene_BulletNinja::setEnemyIdle(std::shared_ptr<Entity> enemy) {
    auto& state = enemy->getComponent<CState>();
    auto& anim = enemy->getComponent<CAnimation>();

    state.unSet(CState::isRunning);
    state.unSet(CState::isAttackSword);

    if (anim.animation.getName() != "SamuraiIdle") {
        anim.setAnimation(Assets::getInstance().getAnimation("SamuraiIdle"));
    }
}

bool Scene_BulletNinja::canMoveForward(std::shared_ptr<Entity> enemy) {
    auto& tfm = enemy->getComponent<CTransform>();
    bool movingRight = tfm.scale.x > 0;

    // Extended check area in movement direction
    sf::FloatRect sideCheck = calculateBoundingBox(enemy, BBType::Hitbox);
    float checkDistance = 25.f; 

    if (movingRight) {
        sideCheck.left += sideCheck.width - 5.f; 
        sideCheck.width = checkDistance + 5.f;
    }
    else {
        sideCheck.left -= checkDistance;
        sideCheck.width = checkDistance + 5.f;
    }

    // check boxes 
    for (auto& box : _entityManager.getEntities("box")) {
        if (box->isActive() && sideCheck.intersects(calculateBoundingBox(box, BBType::Hitbox))) {
            return false;
        }
    }
    return true;
}