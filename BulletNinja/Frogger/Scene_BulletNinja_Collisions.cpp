
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
#include "PlayerProgress.h"



void Scene_BulletNinja::resolveCollision(std::shared_ptr<Entity> entityA,
    std::shared_ptr<Entity> entityB,
    sf::Vector2f overlap)
{
    // Determine which entity should be moved (prioritize player, then enemies, then others)
    bool aShouldMove = entityA->hasComponent<CState>();
    bool bShouldMove = entityB->hasComponent<CState>();

    auto& entityToMove = (aShouldMove && !bShouldMove) ? entityA :
        (!aShouldMove && bShouldMove) ? entityB :
        (entityA->hasComponent<CInput>()) ? entityA : entityB;

    auto& otherEntity = (&entityToMove == &entityA) ? entityB : entityA;

    // Get transform components
    auto& moveTransform = entityToMove->getComponent<CTransform>();
    auto& otherTransform = otherEntity->getComponent<CTransform>();

    // Calculate previous positions
    sf::FloatRect movePrevBB = calculateBoundingBox(entityToMove, BBType::Hitbox);
    sf::FloatRect otherPrevBB = calculateBoundingBox(otherEntity, BBType::Hitbox);
    sf::Vector2f prevOverlap = Physics::getPreviousOverlap(
        movePrevBB, moveTransform.pos - moveTransform.prevPos,
        otherPrevBB, otherTransform.pos - otherTransform.prevPos
    );

    // Vertical collision
    if (overlap.y < overlap.x) {
        if (moveTransform.prevPos.y < otherTransform.prevPos.y) { // Moving entity was above
            moveTransform.pos.y -= overlap.y;
            moveTransform.vel.y = 0;

            // Ground detection
            if (prevOverlap.y <= 0 &&
                (otherEntity->getTag() == "ground" || otherEntity->getTag() == "tile" || otherEntity->getTag() == "box")) {
                if (entityToMove->hasComponent<CState>()) {
                    entityToMove->getComponent<CState>().set(CState::isGrounded);
                }
                if (entityToMove->hasComponent<CInput>()) {
                    entityToMove->getComponent<CInput>().canJump = true;
                }
            }
        }
        else { // Moving entity was below
            moveTransform.pos.y += overlap.y;
            moveTransform.vel.y = 0;
        }
    }
    // Horizontal collision
    else {
        if (moveTransform.prevPos.x < otherTransform.prevPos.x) {
            moveTransform.pos.x -= overlap.x;
        }
        else {
            moveTransform.pos.x += overlap.x;
        }
        moveTransform.vel.x = 0;
    }
}

void Scene_BulletNinja::attackCollisions() {
    const std::unordered_map<std::string, std::vector<std::string>> validTargets = {
        {"player", {"enemy", "box", "zombie", "flowerEnemy"}},  // Player attack these
        {"enemy", {"player"}},
        {"trophyKey", {"player"}},
        {"flowerEnemy", {"player"}}
    };

    for (auto& attacker : _entityManager.getEntities()) {
        // Skip if attacker can't attack
        if (!attacker->hasComponent<CAnimation>() ||
            !attacker->hasComponent<CBoundingBox>() ||
            !attacker->isActive() ||
            !attacker->hasComponent<CScore>()) {
            continue;
        }

        auto& attackerAnim = attacker->getComponent<CAnimation>().animation;
        auto& attackerKey = attacker->getComponent<CAnimation>().key;
        auto& attackerScore = attacker->getComponent<CScore>();
        

        // Only check during frames with active attackboxes
        if (attackerAnim.hasAttackbox()) {
            sf::FloatRect attackBox = calculateBoundingBox(attacker, BBType::Attackbox);
            auto attackerTag = attacker->getTag();

            // Skip if no valid targets
            if (validTargets.count(attackerTag) == 0) continue;

            for (auto& target : _entityManager.getEntities()) {

                if (target == attacker || !target->isActive()) continue;


                auto targetTag = target->getTag();

                const auto& targets = validTargets.at(attackerTag);
                if (std::find(targets.begin(), targets.end(), targetTag) == targets.end()) {
                    continue;
                }

                // Check if target can be hit
                if (!target->hasComponent<CBoundingBox>() ||
                    (target->hasComponent<CScore>() && target->getComponent<CScore>()._hp <= 0)) {
                    continue;
                }

                sf::FloatRect targetBox = calculateBoundingBox(target, BBType::Hitbox);
                

                if (attackBox.intersects(targetBox)) {
                    //player to key collision
                    if(attackerTag == "trophyKey" && targetTag == "player") {
						std::cout << "Player hit by trophy key\n";
                        // Check if the key box is still alive to prevent key from being collected
                        bool keyBoxAlive = false;
                        for(auto& e : _entityManager.getEntities("box")) {
							if(attacker->getComponent<CKeyState>().keyID == e->getId() && e->isActive()) {
								keyBoxAlive = true;
								break;
							}
						}
                        if (!keyBoxAlive) {
                            attacker->destroy();
                            auto& playerProgress = PlayerProgress::getInstance();
                            playerProgress.setKeysCollected(true);
                        }
                        continue;
                        
					}
                    // player to flower enemy collision
                    if (attackerTag == "player" && targetTag == "flowerEnemy") {
                        
                        auto& flowerEnemyState = target->getComponent<CState>();
                        auto& flowerEnemyAnim = target->getComponent<CAnimation>();
                        auto& tfm = target->getComponent<CTransform>();

                        if(flowerEnemyState.test(CState::isDead)) {
							continue;
						}
                        flowerEnemyState.set(CState::isDead);
                        tfm.vel = sf::Vector2f(0, 0);
                        PlayerProgress::getInstance().setCurrentLevelScore(30);
                        flowerEnemyAnim.setAnimation(Assets::getInstance().getAnimation("flowerdeath"));
                        continue;
                    }

                    // rest of entities
                    if (target->hasComponent<CScore>()) {
                        auto& targetLife = target->getComponent<CScore>();
                        auto& targetState = target->getComponent<CState>();
                        

                        if (targetLife.canTakeHit(attackerKey)) {
                            targetLife.registerHit(attackerKey);
                            targetLife._hp -= 10;
                            if (targetTag == "enemy") PlayerProgress::getInstance().setCurrentLevelScore(10);
                            
                            

                            if (target->hasComponent<CHitEffect>()) {
                                auto& hit = target->getComponent<CHitEffect>();
                                hit.start(targetState);
                            }
                            // Handle death/respawn
                            if (targetLife._hp <= 0) {
                                targetLife._lives -= 1;
                                if (target->getTag() == "player" && targetLife._lives > 0) {
                                    auto& playerProgress = PlayerProgress::getInstance();
                                    playerProgress.setLives(playerProgress.getLives() - 1);
                                    respawnPlayer();
                                }
                                if (targetLife._lives <= 0) {
                                    if (targetTag == "box") {
                                        if (attackerTag == "player") PlayerProgress::getInstance().setCurrentLevelScore(10);
                                    }
                                    target->destroy();
                                    if(target->getTag() == "player") {
										m_player->getComponent<CWasted>().isWasted = true;
									}
                                }
                                else {
                                    targetLife._hp = targetLife._DefautHp;
                                }

                                setPermanentKeyWhereDeadBox(target); // Set key to permanent if box contains key
                            }


                        }
                    }
                }
            }
        }
    }
}

void Scene_BulletNinja::flowerEnemyMovement() {
    auto flowerEnemies = _entityManager.getEntities("flowerEnemy");

    for (auto& enemy : flowerEnemies) {
        auto& tfm = enemy->getComponent<CTransform>();

        tfm.prevPos = tfm.pos;
        tfm.pos += tfm.vel;

        // Check collision with box or doors
        bool shouldReverse = false;
        sf::FloatRect enemyBB = calculateBoundingBox(enemy, BBType::Hitbox);

        for (auto& other : _entityManager.getEntities()) {
            if (!other->hasComponent<CBoundingBox>() || other == enemy)
                continue;

            const std::string& tag = other->getTag();
            if (tag != "box" && tag != "door" || !other->isActive())
                continue;

            // Check collision
            sf::FloatRect otherBB = calculateBoundingBox(other, BBType::Hitbox);
            sf::Vector2f overlap = Physics::getOverlap(enemyBB, otherBB);

            if (overlap.x > 0.f && overlap.y > 0.f) {
                shouldReverse = true;
                break; // Stop checking after first collision
            }
        }

        
        if (shouldReverse) {
            tfm.vel.x *= -1;

            tfm.pos = tfm.prevPos;
        }
    }
}


void Scene_BulletNinja::checkAndAppyGravity() {
    for (auto& e : _entityManager.getEntities()) {
        if (!e->hasComponent<CState>()) continue;

        auto& state = e->getComponent<CState>();
        bool wasGrounded = state.test(CState::isGrounded);

        // Only check support if was grounded last frame
        if (wasGrounded) {
            bool hasSupport = false;
            sf::FloatRect feetCheck = calculateBoundingBox(e, BBType::Hitbox);
            feetCheck.top += 1.0f;

            // Check for supporting entities
            for (auto& other : _entityManager.getEntities()) {
                if (other == e) continue;
                const std::string& tag = other->getTag();
                if ((tag == "box" || tag == "tile" || tag == "ground") &&
                    feetCheck.intersects(calculateBoundingBox(other, BBType::Hitbox))) {
                    hasSupport = true;
                    break;
                }
            }

            // nly unset if no support
            if (!hasSupport) {
                state.unSet(CState::isGrounded);
            }
        }
    }
    
}

