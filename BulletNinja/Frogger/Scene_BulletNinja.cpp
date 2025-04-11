
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

namespace {
    std::random_device rd;
    std::mt19937 rng(rd());
}


Scene_BulletNinja::Scene_BulletNinja(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine), m_worldView(gameEngine->window().getDefaultView())
{   
    loadLevel(levelPath);
    registerActions();

    m_text.setFont(Assets::getInstance().getFont("Arcade"));

    sf::Vector2f spawnPos{   m_worldView.getSize().x / 2.f , m_worldView.getSize().y / 2.f };
    std::cout << "=== m_worldBounds.width: " << m_worldBounds.width << "  m_worldView.y  " << m_worldView.getSize().y << "\n";


    // _worldView is the camera. It's view is the same size as the render winwidow
    // _worldBounds is the boundry of our game world.
    // this is a vertical scroller, the camera starts at the bottlm of the game world bounds and scrolls up
    // the player is spawned at the bottom of the world and the camera/game scroll up to the top of the world
    m_worldView.setCenter(spawnPos);

    // Spawn static objects
    sSpawnStaticObjects();
    

    //auto pos = m_worldView.getSize();

    // To refactor
    auto playerPos = sf::Vector2f{ spawnPos.x, spawnPos.y + 40.f };

    //m_ground = playerPos;  // To do: move m_ground value to JSON file

    auto boxPos = sf::Vector2f{ spawnPos.x + 800.f, spawnPos.y };
    auto ePos = sf::Vector2f{ boxPos.x + 200.f, playerPos.y };
    spawnPlayer(playerPos);
    

    spawnBox(boxPos);


    //auto ePos2 = sf::Vector2f{ boxPos.x + 250.f, playerPos.y };
    spawnEnemy(ePos);
    //spawnEnemy(ePos2);

   

    m_timer = sf::seconds(60.0f);
    m_maxHeight = spawnPos.y;
    m_score = 0;
    m_lives = 3;
    m_reachGoal = 0;

    

    MusicPlayer::getInstance().play("gameTheme");
    MusicPlayer::getInstance().setVolume(50);
}

void Scene_BulletNinja::init(const std::string& levelPath) {

   
}

void Scene_BulletNinja::spawnPlayer(sf::Vector2f pos) {
    m_player = _entityManager.addEntity("player");
    m_player->addComponent<CTransform>(pos).scale = sf::Vector2f(4.f, 4.f);
    m_player->addComponent<CState>();
    m_player->addComponent<CBoundingBox>(sf::Vector2f(52.f, 102.f));
    m_player->addComponent<CInput>();
    m_player->addComponent<CJump>();
    m_player->addComponent<CScore>(m_lives);
    m_player->addComponent<CScore>(3, 200);
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));

    auto& anim = m_player->getComponent<CAnimation>();
    anim.onAnimationEnd = [this]() {
        auto& state = m_player->getComponent<CState>();
        auto& anim = m_player->getComponent<CAnimation>();

        //std::cout << "Key at callback: " << anim.getCurrentKey() << "\n";

        if (state.test(CState::isAttackSword) || state.test(CState::isAttackSpear)) {
            state.unSet(CState::isAttacking);
            state.unSet(CState::isAttackSword);
            state.unSet(CState::isAttackSpear);
            anim.setAnimation(Assets::getInstance().getAnimation("PlayerIdle"));
        }
        };
    

    
}

void Scene_BulletNinja::spawnEnemy(sf::Vector2f pos) {
    auto enemy = _entityManager.addEntity("enemy");
    enemy->addComponent<CTransform>(pos).scale = sf::Vector2f(4.f, 4.f);
    enemy->addComponent<CState>();
    enemy->addComponent<CBoundingBox>(sf::Vector2f(52.f, 102.f));
    enemy->addComponent<CInput>(); 
    enemy->addComponent<CScore>(1, 100);
    auto& anim = enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));

    
    anim.onAnimationEnd = [this, enemy]() {
        if (!enemy->isActive()) return;

        auto& state = enemy->getComponent<CState>();
        auto& anim = enemy->getComponent<CAnimation>();

        if (state.test(CState::isAttacking)) {
            
            state.unSet(CState::isAttacking);
            state.unSet(CState::isAttackSword);

            
            anim.setAnimation(Assets::getInstance().getAnimation("SamuraiIdle"));
        }
        };

    
    enemy->getComponent<CTransform>().vel = sf::Vector2f(0, 0);
}

void Scene_BulletNinja::spawnBox(sf::Vector2f pos) {
    auto m_box = _entityManager.addEntity("box");
    const sf::Texture& texture = Assets::getInstance().getTexture("box");
    m_box->addComponent<CTransform>(pos).scale = sf::Vector2f(.5f, .5f);
    //m_box->addComponent<CState>().state = "box";
    m_box->addComponent<CBoundingBox>(sf::Vector2f(40.f, 40.f));
    m_box->addComponent<CSprite>(texture).sprite;
    m_box->addComponent<CScore>(1);
}

void Scene_BulletNinja::playerAttacks() {
    if (!m_player->isActive() || m_player->getComponent<CState>().test(CState::isDead)) {
        return;
    }

    auto& input = m_player->getComponent<CInput>();
    auto& state = m_player->getComponent<CState>();
    auto& anim = m_player->getComponent<CAnimation>();

    
    if (state.test(CState::isAttacking) && anim.animation.hasEnded()) {
        std::cout << "CLEARING STALE ATTACK STATE\n";
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

bool Scene_BulletNinja::isCollidingWithWalls(std::shared_ptr<Entity> entity, sf::Vector2f& nextPos) {
    
    sf::FloatRect entityBB = calculateBoundingBox(entity, Hitbox);
    auto& scale = entity->getComponent<CTransform>().scale;
    
    entityBB.left = scale.x == -std::abs(scale.x) ? nextPos.x - 15.f : nextPos.x - 40.f;
    

    

    
    for (auto& wall : _entityManager.getEntities("box")) {
        sf::FloatRect wallBB = calculateBoundingBox(wall, Hitbox);

        if (entityBB.intersects(wallBB)) {

            auto& transform = entity->getComponent<CTransform>();
            auto& state = entity->getComponent<CState>();
            auto& anim = entity->addComponent<CAnimation>();

            transform.pos = transform.prevPos;
            //state = "idle";
            if (entity->getTag() == "player") anim.animation = Assets::getInstance().getAnimation("PlayerIdle");
            if (entity->getTag() == "enemy") anim.animation = Assets::getInstance().getAnimation("SamuraiIdle");

            


            return true;
        }
    }

    return false;
}


bool Scene_BulletNinja::hasObstacleBetween(std::shared_ptr<Entity> enemy, const sf::Vector2f& playerPos) {
    // Get positions
    sf::Vector2f enemyPos = enemy->getComponent<CTransform>().pos;

    // Determine direction (left or right)
    bool playerIsRight = playerPos.x > enemyPos.x;

    // Get all boxes and doors once
    auto obstacles = _entityManager.getEntities();
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(), [](auto& e) {
            std::string tag = e->getTag();
            return !(tag == "box" || tag == "door");
            }),
        obstacles.end()
    );

    // Check each obstacle in the path
    for (auto& obstacle : obstacles) {
        auto& obstaclePos = obstacle->getComponent<CTransform>().pos;
        auto& obstacleBox = obstacle->getComponent<CBoundingBox>();

        // Check if obstacle horizontally
        bool inXRange = playerIsRight
            ? (obstaclePos.x > enemyPos.x && obstaclePos.x < playerPos.x)
            : (obstaclePos.x < enemyPos.x && obstaclePos.x > playerPos.x);

        
        bool onSameLevel = std::abs(obstaclePos.y - enemyPos.y) < 50.f;

        if (inXRange && onSameLevel) {
            return true;
        }
    }

    return false;
}

void Scene_BulletNinja::enemyMovement(sf::Time dt) {
    if (!m_player) return;

    const float chaseDistance = 500.f;
    const float attackRange = 60.f;

    auto& playerPos = m_player->getComponent<CTransform>().pos;

    for (auto& enemy : _entityManager.getEntities("enemy")) {
        if (!enemy->isActive()) continue;

        auto& transform = enemy->getComponent<CTransform>();
        auto& state = enemy->getComponent<CState>();
        auto& anim = enemy->getComponent<CAnimation>();


        transform.vel.x = 0;

        float xDistance = playerPos.x - transform.pos.x;

        // Skip if out of view
        if (std::abs(transform.pos.x - m_worldView.getCenter().x) > m_worldView.getSize().x / 2) {
            state.unSet(CState::isRunning);
            if (anim.animation.getName() != "SamuraiIdle") {
                anim.setAnimation(Assets::getInstance().getAnimation("SamuraiIdle"));
            }
            continue;
        }

        // Check for obstacles
        if (hasObstacleBetween(enemy, playerPos)) {
            state.unSet(CState::isRunning);
            if (anim.animation.getName() != "SamuraiIdle") {
                anim.setAnimation(Assets::getInstance().getAnimation("SamuraiIdle"));
            }
            continue;
        }

        // attck state
        if (std::abs(xDistance) <= attackRange) {
            if (!state.test(CState::isAttacking)) {
                state.set(CState::isAttacking);
                anim.setAnimation(Assets::getInstance().getAnimation("SamuraiAttackSword"));
            }
            continue;
        }

        // chase player
        if (std::abs(xDistance) <= chaseDistance) {
            state.set(CState::isRunning);
            transform.vel.x = (xDistance > 0) ? m_config.enemySpeed : -m_config.enemySpeed;
            transform.scale.x = (xDistance > 0) ? std::abs(transform.scale.x) : -std::abs(transform.scale.x);

            if (anim.animation.getName() != "SamuraiRun") {
                anim.setAnimation(Assets::getInstance().getAnimation("SamuraiRun"));
            }
        }
        else {
            state.unSet(CState::isRunning);
            if (anim.animation.getName() != "SamuraiIdle") {
                anim.setAnimation(Assets::getInstance().getAnimation("SamuraiIdle"));
            }
            continue;
        }
    }
}

void Scene_BulletNinja::updateEnemyMovement(std::shared_ptr<Entity> enemy, sf::Vector2f& nextPos,
    std::string& anim, const std::string& curAnim,
    CTransform& enemyTransform, const CTransform& playerTransform) {
    if (!enemy) return; 

    if (!isCollidingWithWalls(enemy, nextPos)) {
        anim = anim.empty() ? curAnim : anim;
        if (curAnim != anim) {
            enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(anim));
        }
        enemyTransform.pos.x = nextPos.x;
        enemyTransform.pos.y = playerTransform.pos.y;
    }
    else {
        if (curAnim != "SamuraiIdle") {
            enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));
            return;
        }
        enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));
    }
}







void Scene_BulletNinja::updateCamera() {
    auto& playerPos = m_player->getComponent<CTransform>().pos;
    sf::Vector2f viewSize = m_worldView.getSize();

    float halfViewWidth = viewSize.x / 2.f;
    float leftBound = halfViewWidth;
    float rightBound = m_worldBounds.width - halfViewWidth;

    sf::Vector2f newCenter = m_worldView.getCenter();

    
    if (playerPos.x < leftBound) {
        newCenter.x = leftBound;
    }
    else if (playerPos.x > rightBound) {
        newCenter.x = rightBound;
    }
    else {
        newCenter.x = playerPos.x; 
    }

    m_worldView.setCenter(newCenter);
}

void Scene_BulletNinja::sMovement(sf::Time dt) {
    playerMovement(dt);
    //adjustEntityMovement(m_player); // in case of box collision

    enemyMovement(dt);
    //for (auto& enemy : _entityManager.getEntities("enemy")) { 
    //    adjustEntityMovement(enemy); // in case of box collision
    //}

    playerAttacks();

    // move all objects
    for (auto e : _entityManager.getEntities()) {
        if (e->hasComponent<CInput>())
            continue; // player is moved in playerMovement
        if (e->hasComponent<CTransform>()) {
            auto& tfm = e->getComponent<CTransform>();

            tfm.prevPos = tfm.pos;

            tfm.pos += tfm.vel * dt.asSeconds();
            tfm.angle += tfm.angVel * dt.asSeconds();
        }
    }
}



sf::FloatRect Scene_BulletNinja::getViewBounds() {
    return sf::FloatRect();
}

void Scene_BulletNinja::playerCheckState() {
    auto& tx = m_player->getComponent<CTransform>();
    auto& state = m_player->getComponent<CState>();

    // face the right way
    if (std::abs(tx.vel.x) > 0.1f)
        tx.scale.x = (tx.vel.x > 0) ? 1 : -1;

    if (!state.test(CState::isGrounded)) {
        m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("PlayerIdle");
    }
    else {
        // if grounded
        if (std::abs(tx.vel.x) > 0.1f) {
            if (!state.test(CState::isRunning)) // wasn't running
            {
                // change to running animation
                m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"));
                state.set(CState::isRunning);
            }
        }
        else {
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));
            state.unSet(CState::isRunning);
        }
    }
}

void Scene_BulletNinja::attackCollisions() {
    const std::unordered_map<std::string, std::vector<std::string>> validTargets = {
        {"player", {"enemy", "box", "zombie"}},  // Player can attack these
        {"enemy", {"player"}}                          // Enemies can only attack player
    };

    for (auto& attacker : _entityManager.getEntities()) {
        // Skip if attacker can't attack
        if (!attacker->hasComponent<CAnimation>() ||
            !attacker->hasComponent<CBoundingBox>() ||
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
                    if (target->hasComponent<CScore>()) {
                        auto& targetLife = target->getComponent<CScore>();
                        if (attacker->getTag() == "player") {
                            std::cout << "Player hit " << targetTag << " (Key: " << attackerKey << ")\n";
                        }

                        if (targetLife.canTakeHit(attackerKey)) {
                            std::cout << "Hit registered\n";
                            targetLife.registerHit(attackerKey);
                            targetLife._hp -= 10;
                            attackerScore._score += 10;

                            // Handle death/respawn
                            if (targetLife._hp <= 0) {
                                targetLife._lives -= 1;
                                if (targetLife._lives <= 0) {
                                    target->destroy();
                                }
                                else {
                                    targetLife._hp = targetLife._DefautHp;
                                }
                            }

                            std::cout << attackerTag << " hit " << targetTag
                                << " (Key: " << attackerKey << ")\n";
                        }
                    }
                }
            }
        }
    }
}

void Scene_BulletNinja::sCollisions() {
    auto players = _entityManager.getEntities("player");

    for (auto& player : players) {
        auto& pTransform = player->getComponent<CTransform>();
        auto& pInput = player->getComponent<CInput>();
        auto& pState = player->getComponent<CState>();

        sf::FloatRect pCurrBB = calculateBoundingBox(player, BBType::Hitbox);
        sf::Vector2f pOffset = pTransform.pos - pTransform.prevPos;

        for (auto& e : _entityManager.getEntities()) {
            if (!e->hasComponent<CBoundingBox>() || e == player)
                continue;

            const std::string& tag = e->getTag();
            if (tag == "player" || tag == "staticObject")
                continue;

            
            if (tag == "door") {
                auto& doorState = e->getComponent<CDoorState>();
                auto& doorBox = e->getComponent<CBoundingBox>();

                // door is fully open 
                if (doorBox.halfSize.x == 0 && doorBox.halfSize.y == 0) {
                    continue;
                }

                
                sf::FloatRect eCurrBB = calculateBoundingBox(e, BBType::Hitbox);
                sf::Vector2f overlap = Physics::getOverlap(pCurrBB, eCurrBB);

                if (overlap.x > 0.f && overlap.y > 0.f) {
                    resolveDoorCollision(player, e, overlap);
                    pCurrBB = calculateBoundingBox(player, BBType::Hitbox); // Update after resolution
                }
                continue;
            }

            // Normal collision for other entities
            sf::FloatRect eCurrBB = calculateBoundingBox(e, BBType::Hitbox);
            sf::Vector2f overlap = Physics::getOverlap(pCurrBB, eCurrBB);

            if (overlap.x > 0.f && overlap.y > 0.f) {
                resolveDoorCollision(player, e, overlap);
                pCurrBB = calculateBoundingBox(player, BBType::Hitbox);
            }
        }
    }
}

void Scene_BulletNinja::resolveDoorCollision(std::shared_ptr<Entity> player, std::shared_ptr<Entity> door, sf::Vector2f overlap) {
    auto& pTransform = player->getComponent<CTransform>();
    auto& pInput = player->getComponent<CInput>();
    auto& pState = player->getComponent<CState>();
    auto& eTransform = door->getComponent<CTransform>();

    // Get previous positions
    sf::FloatRect pPrevBB = calculateBoundingBox(player, BBType::Hitbox);
    sf::FloatRect ePrevBB = calculateBoundingBox(door, BBType::Hitbox);
    sf::Vector2f prevOverlap = Physics::getPreviousOverlap(pPrevBB,
        pTransform.pos - pTransform.prevPos,
        ePrevBB,
        eTransform.pos - eTransform.prevPos);

    // check collision direction
     // Vertical collision
    if (overlap.y < overlap.x) {
        if (pTransform.prevPos.y < eTransform.prevPos.y) {
            
            pTransform.pos.y -= overlap.y;
            pTransform.vel.y = 0;

            
            if (prevOverlap.y <= 0) {  
                pState.set(CState::isGrounded);
                pInput.canJump = true;
            }
        }
        else {
            // Player is below door
            pTransform.pos.y += overlap.y;
            pTransform.vel.y = 0;
        }
    }
    else { // Horizontal collision
        if (pTransform.prevPos.x < eTransform.prevPos.x) {
            pTransform.pos.x -= overlap.x;
        }
        else {
            pTransform.pos.x += overlap.x;
        }
    }
}



void Scene_BulletNinja::registerActions() {
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "BACK");
    registerAction(sf::Keyboard::Q, "QUIT");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");

    //registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::Left, "LEFT");

    registerAction(sf::Keyboard::A, "ATTACKSWORD");
    registerAction(sf::Keyboard::S, "ATTACKSPEAR");

    registerAction(sf::Keyboard::Right, "RIGHT");
    //registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "JUMP");
    
    registerAction(sf::Keyboard::Down, "DOWN");
}

void Scene_BulletNinja::SpawnStaticObject(std::string name, float x) {
    auto _object = _entityManager.addEntity("staticObject");
    _object->addComponent<CTransform>();
    _object->addComponent<CAnimation>(Assets::getInstance().getAnimation(name));



    auto& transform = _object->getComponent<CTransform>();

    sf::Vector2f spawnPos{ m_worldView.getSize().x / 2.f , m_worldView.getSize().y / 2.f };
    transform.pos = sf::Vector2f(x, spawnPos.y - 57.f);
    transform.scale = sf::Vector2f(1.5f, 1.5f);

 
}

void Scene_BulletNinja::SpawnDoorObject(std::string name, float x, int type, int requiredScore) {
    auto door = _entityManager.addEntity("door");


    door->addComponent<CAnimation>(Assets::getInstance().getAnimation(name));
    sf::Vector2f spawnPos{ m_worldView.getSize().x / 2.f, m_worldView.getSize().y / 2.f };
    door->addComponent<CTransform>(sf::Vector2f(x, spawnPos.y - 80.f));


    auto& doorState = door->addComponent<CDoorState>();
    doorState.type = (type == 0) ? CDoorState::Entrance : CDoorState::Exit;
    doorState.minScoreToOpen = requiredScore;
    doorState.state = CDoorState::Closed;


    door->addComponent<CBoundingBox>(sf::Vector2f{ 20.f, 200.f });


    std::cout << "Spawned " << (type == 0 ? "Entrance" : "Exit")
        << " door at: " << x
        << " (Requires: " << requiredScore << " points)\n";
}

void Scene_BulletNinja::sSpawnStaticObjects()
{
    // spawn enemies when they are half a window above the current camera/view
    auto spawnLine = _game->window().getSize().x;
    std::cout << "_game->window().getSize().x: " << spawnLine << "\n";

    while (!_spawnStaticPoints.empty() && _spawnStaticPoints.top().x > 0 ) {
        SpawnStaticObject(_spawnStaticPoints.top().name, _spawnStaticPoints.top().x);
        _spawnStaticPoints.pop();
    }

    while (!_spawnDoorPoints.empty() && _spawnDoorPoints.top().x > 0) {
        SpawnDoorObject(_spawnDoorPoints.top().name, _spawnDoorPoints.top().x, _spawnDoorPoints.top().type, _spawnDoorPoints.top().requiredScore);
        _spawnDoorPoints.pop();
    }

    auto _object = _entityManager.addEntity("ground");
    _object->addComponent<CBoundingBox>();
    _object->addComponent<CTransform>(m_ground.pos);


}

void Scene_BulletNinja::loadLevel(const std::string& path) {
    std::ifstream config(path);
    if (config.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        config.close();
        exit(1);
    }

    std::string token{ "" };
    config >> token;
    while (!config.eof()) {
        if (token == "Bkg") {
            std::string name;
            sf::Vector2f pos;
            config >> name >> pos.x >> pos.y;
            auto e = _entityManager.addEntity("bkg");

            // for background, no textureRect its just the whole texture
            // and no center origin, position by top left corner
            // stationary so no CTransfrom required.
            /*auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
            sprite.setOrigin(0.f, 0.f);
            sprite.setPosition(pos);*/
            
        }
        else if (token == "World") {
            config >> m_worldBounds.width >> m_worldBounds.height;
            
        }
        else if (token == "Speeds") {
            config >> m_config.cameraReactionSpeed  >> m_config.playerSpeed >> m_config.enemySpeed;
        }
        else if (token == "Tree") {
            SpawnPoint p;
            config >> p.name >> p.x;
            p.x = p.x;
            _spawnStaticPoints.push(p);
        }
        else if (token == "Door") {
            SpawnDoorPoint p;
            config >> p.name >> p.x >> p.type;

            // If it's an exit door, read required score
            if (p.type == 1) {
                config >> p.requiredScore;
            }

            _spawnDoorPoints.push(p);
        }
        else if (token[0] == '#') {
            std::cout << token;
        }

        config >> token;
    }

    config.close();
}

void Scene_BulletNinja::onEnd()
{
    _game->changeScene("MENU", nullptr, false);
}

void Scene_BulletNinja::update(sf::Time dt)
{
   sUpdate(dt);
}




void Scene_BulletNinja::sDoAction(const Command& action)
{
    auto& input = m_player->getComponent<CInput>();
    auto& state = m_player->getComponent<CState>();

    if (action.type() == "START") {
        if (action.name() == "PAUSE") { setPaused(!_isPaused); }
        else if (action.name() == "QUIT") { _game->quitLevel(); }
        else if (action.name() == "BACK") { _game->backLevel(); }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }

        // Player control
        else if (action.name() == "LEFT") { m_player->getComponent<CInput>().LEFT = true; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().RIGHT = true; }

        else if (action.name() == "ATTACKSWORD") { m_player->getComponent<CInput>().SWORD = true; }
        else if (action.name() == "ATTACKSPEAR") { m_player->getComponent<CInput>().SPEAR = true; }
        //if (action.name() == "ATTACKSWORD") { input.attack |= CInput::SWORD; }
        //if (action.name() == "ATTACKSPEAR") { input.attack |= CInput::SPEAR; }

        else if (action.name() == "JUMP") {
            if (m_player->getComponent<CInput>().canJump && m_player->getComponent<CState>().test(CState::isGrounded)) {
                m_player->getComponent<CInput>().UP = true;
                m_player->getComponent<CInput>().canJump = false;
            }
        }


    }
   
    else if (action.type() == "END") {
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().LEFT = false; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().RIGHT = false; }
        else if (action.name() == "JUMP") { m_player->getComponent<CInput>().UP = false; }
        else if (action.name() == "ATTACKSWORD") { m_player->getComponent<CInput>().SWORD = false; }
        else if (action.name() == "ATTACKSPEAR") { m_player->getComponent<CInput>().SPEAR = false; }
    }
}

    
void Scene_BulletNinja::resetPlayer()
{
    sf::Vector2f position = m_worldView.getSize();
    position.x /= 2.0f;
    position.y -= 20.f;

    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
    m_player->addComponent<CTransform>(position);
    //m_player->addComponent<CState>("none");

    m_timer = sf::seconds(60);
    m_maxHeight = position.y;
}



void Scene_BulletNinja::updateScore()
{
    //if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
    //    return;

    //auto& position = m_player->getComponent<CTransform>().pos;

    //if (position.y < m_maxHeight)
    //{
    //    m_maxHeight = position.y;

    //    if (position.y != 600.0f - 260.0f)
    //        m_score += 10;
    //}
}


sf::FloatRect Scene_BulletNinja::calculateBoundingBox(std::shared_ptr<Entity> e, BBType type) {
    auto box = e->getComponent<CBoundingBox>();
    auto transform = e->getComponent<CTransform>();
    auto size = box.size;
    auto position = transform.pos;

    auto& tag = e->getTag();
    auto& state = e->getComponent<CState>();

    if (tag == "box") {
        size = sf::Vector2f{ box.size.x * 2, box.size.y * 2 };
        position = sf::Vector2f{ position.x, position.y + box.size.y };
    }
    else if (tag == "door") {
		size = sf::Vector2f{ box.size.x * 2, box.size.y * 2 };
		position = sf::Vector2f{ position.x, position.y + box.size.y };
	}
	
    else if (tag == "ground") {
        size = sf::Vector2f{ m_ground.width * 3, m_ground.height };
        position = m_ground.pos;
    }
    else if (tag == "player" || tag == "enemy") {
        if (state.test(CState::isAttackSpear)) {
            if (type == Hitbox) {
                size = sf::Vector2f{ box.size.x - 1.f, box.size.y };
                position = sf::Vector2f{ position.x - (size.x * 0.5f), position.y };
                if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
            }
            else if (type == Attackbox) {
                size = sf::Vector2f{ box.size.x * 6.4f, box.size.y + box.size.x };
                position = sf::Vector2f{ position.x, position.y };
                if (transform.scale.x == -std::abs(transform.scale.x)) position.x -= 30.f;
            }
        }
        else if (state.test(CState::isAttackSword)) {
            if (type == Hitbox) {
                size = sf::Vector2f{ box.size.x - 1.f, box.size.y };
                position = sf::Vector2f{ position.x - (size.x * 0.5f), position.y };
                if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
            }
            else if (type == Attackbox) {
                size = sf::Vector2f{ box.size.x * 5.f, box.size.y + box.size.x };
                position = sf::Vector2f{ position.x, position.y };
            }
        }
        else if (state.test(CState::isRunning)) {
            size = sf::Vector2f{ box.size.x - 1.f, box.size.y };
            position = sf::Vector2f{ position.x - (size.x * 0.5f), position.y };
            if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
        }
        else if (state.test(CState::isGrounded) || state.test(CState::isJumping)) {
            size = sf::Vector2f{ box.size.x - 1.f, box.size.y };
            position = sf::Vector2f{ position.x - (size.x * 0.5f), position.y };
            if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
        }
        else {
            size = sf::Vector2f{ box.size.x - 1.f, box.size.y };
            position = sf::Vector2f{ position.x - (size.x * 0.5f), position.y };
            if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
        }
    }

    sf::Vector2f centeredPosition = sf::Vector2f{ position.x - (size.x * 0.5f), position.y - size.y };
    return sf::FloatRect(centeredPosition, size);
}



void Scene_BulletNinja::drawHitbox(std::shared_ptr<Entity> e) {
    if (!m_drawAABB) return;

    auto& animation = e->getComponent<CAnimation>().animation;

    
    //if (!animation.hasHitbox() && e->getTag() == "box") return;

    sf::FloatRect boundingBox = calculateBoundingBox(e, Hitbox);
    sf::RectangleShape rect;
    rect.setSize(boundingBox.getSize());
    //centerOrigin(rect);
    rect.setPosition(boundingBox.getPosition());

    //std::cout << e->getTag() << "-Box position: " << rect.getGlobalBounds().top + rect.getLocalBounds().height << "\n";  // Delete

    rect.setFillColor(sf::Color(0, 0, 0, 0)); // Transparent
    rect.setOutlineColor(sf::Color::Green); // Green outline for hitbox
    rect.setOutlineThickness(2.f);

    _game->window().draw(rect);
}

void Scene_BulletNinja::drawGround(GroundCoord ground) {
    if (!m_drawAABB) return;
    sf::FloatRect groundBB(ground.pos.x, ground.pos.y, ground.width, ground.height);

    sf::RectangleShape rect;
    rect.setSize(groundBB.getSize());
    //centerOrigin(rect);
    rect.setPosition(groundBB.getPosition());

    rect.setFillColor(sf::Color::Green); // Transparent
    rect.setOutlineColor(sf::Color(0, 0, 0, 0)); // Blue outline for attack box
    rect.setOutlineThickness(4.f);

    _game->window().draw(rect);

}

void Scene_BulletNinja::drawAttackBox(std::shared_ptr<Entity> e) {
    if (!m_drawAABB) return;

    auto& animation = e->getComponent<CAnimation>().animation;
    auto& state = e->getComponent<CState>().state;
    if (!animation.hasAttackbox()) return;

    sf::FloatRect boundingBox = calculateBoundingBox(e, Attackbox);
    
    //if (state.find("attack") != std::string::npos) std::cout << "attack box: " << boundingBox.height << "\n";
    
    
    sf::RectangleShape rect;
    rect.setSize(boundingBox.getSize());
    //centerOrigin(rect);
    rect.setPosition(boundingBox.getPosition());

    rect.setFillColor(sf::Color(0, 0, 0, 0)); // Transparent
    rect.setOutlineColor(sf::Color::Blue); // Blue outline for attack box
    rect.setOutlineThickness(4.f);

    _game->window().draw(rect);
}




void Scene_BulletNinja::sRender()
{
    _game->window().setView(m_worldView);

    // draw bkg first
    for (auto e : _entityManager.getEntities("bkg")) {
        if (e->getComponent<CSprite>().has) {
            auto& sprite = e->getComponent<CSprite>().sprite;
            _game->window().draw(sprite);
        }
    }

    
    for (auto& e : _entityManager.getEntities()) {
        if (!e->hasComponent<CAnimation>())
            continue;

        // Draw Sprite
        auto& anim = e->getComponent<CAnimation>().animation;
        auto& tfm = e->getComponent<CTransform>();
        anim.getSprite().setPosition(tfm.pos);
        anim.getSprite().setRotation(tfm.angle);
        anim.getSprite().setScale(tfm.scale);
        _game->window().draw(anim.getSprite());

        //drawAABB(e);
        drawHitbox(e);
        drawAttackBox(e);

    }

    for (auto e : _entityManager.getEntities("box")) {
        if (e->getComponent<CSprite>().has) {
            auto& anim = e->getComponent<CSprite>().sprite;
            auto& tfm = e->getComponent<CTransform>();
            anim.setPosition(tfm.pos);
            anim.setRotation(tfm.angle);
            anim.setScale(tfm.scale);

            //drawAABB(e);
            drawHitbox(e);
            drawAttackBox(e);

            _game->window().draw(anim);
        }
    }

    drawGround(m_ground);

    _game->window().setView(_game->window().getDefaultView());

    auto& score = m_player->getComponent<CScore>();

    m_text.setPosition(5.0f, -5.0f);
    m_text.setString("score  " + std::to_string(score._score));
    _game->window().draw(m_text);

    int time = static_cast<int>(std::ceil(m_timer.asSeconds()));

    m_text.setPosition(5.0f, 22.5f);
    m_text.setString("Current HP  " + std::to_string(score._hp));
    _game->window().draw(m_text);

    m_text.setPosition(5.0f, 52.5f);
    m_text.setString("Lives  " + std::to_string(score._lives));

    _game->window().draw(m_text);

    if(m_nearestExitDoor) {
		auto& doorTransform = m_nearestExitDoor->getComponent<CTransform>();
		auto& doorState = m_nearestExitDoor->getComponent<CDoorState>();

		m_text.setPosition(5.0f, 82.5f);
		m_text.setString("Level requirement: " + std::to_string(doorState.minScoreToOpen) + " points)");
		_game->window().draw(m_text);
	}
}

void Scene_BulletNinja::sUpdate(sf::Time dt) {
    SoundPlayer::getInstance().removeStoppedSounds();
    _entityManager.update();

    if (m_lives <= 0 || m_reachGoal >= 5)
        _game->quitLevel();

    if (_isPaused)
        return;
    auto& anim = m_player->getComponent<CAnimation>();
    auto& state = m_player->getComponent<CState>();



    /*m_timer -= dt;

    if (m_timer.asSeconds() <= 0)
        killPlayer();*/

    checkPlayerState();

    sAnimation(dt);
    sMovement(dt);
    attackCollisions();
    updateDoors();
    sCollisions();

    
    updateScore();
}


void Scene_BulletNinja::sAnimation(sf::Time dt) {
    auto list = _entityManager.getEntities();
    for (auto e : _entityManager.getEntities()) {
        
        // update all animations
        if (e->hasComponent<CAnimation>()) {
            auto& animation = e->getComponent<CAnimation>().animation;

            

            animation.update(dt);
            // do nothing if animation has ended
        }
    }
}


void Scene_BulletNinja::adjustPlayerPosition() {
    auto center = m_worldView.getCenter();
    sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


    auto left = center.x - viewHalfSize.x;
    auto right = center.x + viewHalfSize.x;
    auto top = center.y - viewHalfSize.y;
    auto bot = center.y + viewHalfSize.y;

    auto& player_pos = m_player->getComponent<CTransform>().pos;
    auto halfSize = sf::Vector2f{ 20, 20 };
    // keep player in bounds
    player_pos.x = std::max(player_pos.x, left + halfSize.x);
    player_pos.x = std::min(player_pos.x, right - halfSize.x);
    player_pos.y = std::max(player_pos.y, top + halfSize.y);
    player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

void Scene_BulletNinja::checkPlayerState()
{
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().test(CState::isDead))
    {
        auto& animation = m_player->getComponent<CAnimation>().animation;

        if (animation.hasEnded())
        {
            resetPlayer();
        }
    }
}

bool Scene_BulletNinja::isOnGroundOrPlatforms(const sf::FloatRect& entityBB,
    const std::vector<std::shared_ptr<Entity>>& platforms,
    const sf::Vector2f& groundPos,
    float groundHeight) {
    sf::FloatRect feet = entityBB;
    //sf::FloatRect e = entityBB;
    
    feet.top += 1.f;
    //feet.height = 2.f;
    //centerOrigin(feet);
    

    sf::FloatRect groundBB(groundPos.x, groundPos.y, 5000.f, groundHeight);

 /*   std::cout << "feet " << feet.left << ", " << feet.top + feet.height << ", " << feet.width << ", " << feet.height << ", " << "\n";
    std::cout << "ground " << groundBB.left << ", " << groundBB.top + groundBB.height << ", " << groundBB.width << ", " << groundBB.height << "\n";*/

    if (feet.intersects(groundBB)) {
        return true;
    }

    for (auto& platform : platforms) {
        sf::FloatRect platformBB = calculateBoundingBox(platform, Hitbox);
        if (feet.intersects(platformBB)) {
            return true;
        }
    }
    return false;
}


void Scene_BulletNinja::updateDoors() {
    if (!m_player) return;

    const float activationDistance = 150.f;
    auto& playerPos = m_player->getComponent<CTransform>().pos;

    float minXDifference = FLT_MAX; 

    auto playerX = m_player->getComponent<CTransform>().pos.x;

    // check on x axis
    for (auto& door : _entityManager.getEntities("door")) {
        if (!door->hasComponent<CDoorState>()) continue;

        auto& doorState = door->getComponent<CDoorState>();
        if (doorState.type != CDoorState::Exit) continue;

        float doorX = door->getComponent<CTransform>().pos.x;
        float xDiff = std::abs(doorX - playerX);

        if (xDiff < minXDifference) {
            minXDifference = xDiff;
            m_nearestExitDoor = door;
        }
    }

    for (auto& door : _entityManager.getEntities("door")) {
        if (!door->hasComponent<CDoorState>() || !door->hasComponent<CAnimation>()) {
            continue;
        }

        auto& doorState = door->getComponent<CDoorState>();
        auto& doorTransform = door->getComponent<CTransform>();
        auto& doorAnim = door->getComponent<CAnimation>().animation;
        auto& doorBox = door->getComponent<CBoundingBox>();
        float distance = std::abs(playerPos.x - doorTransform.pos.x);

        bool shouldOpen = distance <= activationDistance;

        // Only change state if needed
        if ((shouldOpen && doorState.state == CDoorState::Closed) ||
            (!shouldOpen && doorState.state == CDoorState::Open)) {

            doorState.state = shouldOpen ? CDoorState::Open : CDoorState::Closed;
            doorAnim.setReversed(!shouldOpen);

            if (shouldOpen) {
                doorAnim.playForward();
            }
            else {
                doorAnim.playBackward();
            }
        }

        // create animation from frame progress
        if (doorState.state == CDoorState::Opening || doorState.state == CDoorState::Closing) {
            
            float progress = static_cast<float>(doorAnim.getCurFrame()) / doorAnim.getFramesSize();
            if (doorState.state == CDoorState::Closing) {
                progress = 1.f - progress;
            }
            doorBox.halfSize.x = 20.f * (1.f - progress); // Width reduces as door opens
        }
        else if (doorAnim.hasEnded()) {
            // Final state after animation completes
            if (doorState.state == CDoorState::Open) {
                doorBox.halfSize = sf::Vector2f(0.f, 0.f); // set Fully open
            }
            else {
                doorBox.halfSize = sf::Vector2f(20.f, 200.f); // set closed
            }
        }
    }
}

