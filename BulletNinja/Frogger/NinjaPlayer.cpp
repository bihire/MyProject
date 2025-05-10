
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

void Scene_BulletNinja::spawnPlayer(sf::Vector2f pos) {


    m_player = _entityManager.addEntity("player");
    m_player->addComponent<CTransform>(pos).scale = sf::Vector2f(4.f, 4.f);
    m_player->addComponent<CState>();
    m_player->addComponent<CBoundingBox>(sf::Vector2f(52.f, 102.f));
    m_player->addComponent<CInput>();
    m_player->addComponent<CJump>();
    m_player->addComponent<CScore>(m_lives);
    m_player->addComponent<CScore>(1, 30);
    m_player->addComponent<CHealthBarIndicator>();
    auto& levelComplete =m_player->addComponent<CLevelComplete>();
    m_player->addComponent<CGameComplete>();
    m_player->addComponent<CHitEffect>(sf::seconds(0.5f));
    auto& wasted = m_player->addComponent<CWasted>();
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));


    

    auto& anim = m_player->getComponent<CAnimation>();
    anim.onAnimationEnd = [this]() {
        auto& state = m_player->getComponent<CState>();
        auto& anim = m_player->getComponent<CAnimation>();


        if (state.test(CState::isAttackSword) || state.test(CState::isAttackSpear)) {
            state.unSet(CState::isAttacking);
            state.unSet(CState::isAttackSword);
            state.unSet(CState::isAttackSpear);
            anim.setAnimation(Assets::getInstance().getAnimation("PlayerIdle"));
        }
        };
    wasted.onEnd( [this]() {
        _game->changeScene("MENU", nullptr, false);
	});

    levelComplete.onEnd([this]() {
        for (auto& e : _entityManager.getEntities("trophyKey")) {
            e->respawn();
            e->getComponent<CKeyState>().reset();

        }
	});




}

void Scene_BulletNinja::respawnPlayer() {

   
       if(!m_player) return;
       auto& score = m_player->getComponent<CScore>();
    auto& tfm = m_player->getComponent<CTransform>();
    auto& isJumping = m_player->getComponent<CState>();
    auto doorLeft = closestDoorLeft();
       
    tfm.pos = sf::Vector2f{ doorLeft.x - (m_doorGap / 2), (m_worldView.getSize().y / 2.f) - 200.f };
    isJumping.unSet(CState::isGrounded);
    tfm.scale.x = -std::abs(tfm.scale.x);
    
    m_player->respawn();
    
}




void Scene_BulletNinja::playerMovement(sf::Time dt) {
    auto& pt = m_player->getComponent<CTransform>();
    auto& input = m_player->getComponent<CInput>();
    auto& state = m_player->getComponent<CState>();
    auto& anim = m_player->getComponent<CAnimation>();

    // Reset horizontal velocity
    pt.vel.x = 0.f;

    // Handle movement input
    bool movingLeft = input.LEFT;
    bool movingRight = input.RIGHT;
    bool wantsToMove = movingLeft || movingRight;

    // Apply movement forces
    if (movingLeft) {
        pt.vel.x -= 1.f;
        state.set(CState::isFacingLeft);
        state.set(CState::isRunning);
    }
    else if (movingRight) {
        pt.vel.x += 1.f;
        state.unSet(CState::isFacingLeft);
        state.set(CState::isRunning);
    }
    else {
        state.unSet(CState::isRunning);
    }

    // Animation state management
    std::string desiredAnim = anim.animation.getName();



    if (state.test(CState::isDead)) {
        desiredAnim = "PlayerDead";
    }
    else if (state.test(CState::isAttackSword)) {
        desiredAnim = "PlayerAttackSword";
    }
    else if (state.test(CState::isAttackSpear)) {
        desiredAnim = "PlayerAttackSpear";
    }
    else if (state.test(CState::isRunning)) {
        desiredAnim = "PlayerRun";
    }
    else if (!state.test(CState::isGrounded)) {
        desiredAnim = (pt.vel.y > 0) ? "PlayerIdle" : "PlayerIdle";
    }

    else {
        desiredAnim = "PlayerIdle";
    }

    // Only change animation if needed
    if (desiredAnim != anim.animation.getName()) {
        anim.setAnimation(Assets::getInstance().getAnimation(desiredAnim));

    }

    // Handle jumping
    if (input.UP && state.test(CState::isGrounded)) {
        input.UP = false;
        pt.vel.y = -m_jumpStrength;
        state.set(CState::isJumping);
        state.unSet(CState::isGrounded);
    }

    // Apply physics
    if (!state.test(CState::isGrounded)) {
        pt.vel.y += m_gravity;
    }
    pt.vel.x *= m_speed;

    // Update facing direction
    pt.scale.x = state.test(CState::isFacingLeft)
        ? -std::abs(pt.scale.x)
        : std::abs(pt.scale.x);

    // Move all entities
    for (auto e : _entityManager.getEntities()) {
        auto& tx = e->getComponent<CTransform>();
        tx.prevPos = tx.pos;
        tx.pos += tx.vel;
    }

    updateCamera();
}

void Scene_BulletNinja::playerAttacks() {
    if (!m_player->isActive() || m_player->getComponent<CState>().test(CState::isDead)) {
        return;
    }

    auto& input = m_player->getComponent<CInput>();
    auto& state = m_player->getComponent<CState>();
    auto& anim = m_player->getComponent<CAnimation>();


    if (state.test(CState::isAttacking) && anim.animation.hasEnded()) {
        //std::cout << "CLEARING STALE ATTACK STATE\n";
        state.unSet(CState::isAttacking);
        state.unSet(CState::isAttackSword);
        state.unSet(CState::isAttackSpear);
        anim.animation = Assets::getInstance().getAnimation("PlayerIdle");
    }


    if (state.test(CState::isAttacking)) {
        return;
    }

    // Handle  attack
    if (input.SWORD) {
        state.set(CState::isAttacking);
        state.set(CState::isAttackSword);


        Animation newAnim = Assets::getInstance().getAnimation("PlayerAttackSword");
        newAnim.onFrameChange = anim.animation.onFrameChange;
        anim.animation = newAnim;
        anim.animation.setRepeating(false);


        input.SWORD = false;

    }

    else if (input.SPEAR) {
        state.set(CState::isAttacking);
        state.set(CState::isAttackSpear);

        Animation newAnim = Assets::getInstance().getAnimation("PlayerAttackSpear");
        newAnim.onFrameChange = anim.animation.onFrameChange; // Preserve callback
        anim.animation = newAnim;
        anim.animation.setRepeating(false);


        input.SPEAR = false;
        //SoundPlayer::getInstance().play("spear_throw", m_player->getComponent<CTransform>().pos);
    }

    updateCamera();
}