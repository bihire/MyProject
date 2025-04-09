
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

    auto boxPos = sf::Vector2f{ spawnPos.x + 200.f, spawnPos.y };
    auto ePos = sf::Vector2f{ boxPos.x + 200.f, playerPos.y };
    spawnPlayer(playerPos);
    

    spawnBox(boxPos);


    //auto ePos2 = sf::Vector2f{ boxPos.x + 250.f, playerPos.y };
    //spawnEnemy(ePos);
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
    m_player->addComponent<CState>().state = "idle";
    m_player->addComponent<CBoundingBox>(sf::Vector2f(52.f, 52.f));
    m_player->addComponent<CInput>();
    m_player->addComponent<CJump>();
    m_player->addComponent<CScore>(m_lives);
    m_player->addComponent<CScore>(3, 200);
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));
    

    
}

void Scene_BulletNinja::spawnEnemy(sf::Vector2f pos) {
    auto enemy = _entityManager.addEntity("enemy");
    enemy->addComponent<CTransform>();
    enemy->addComponent<CState>().state = "idle";
    enemy->addComponent<CBoundingBox>(sf::Vector2f(52.f, 52.f));
    enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));


    auto& transform = enemy->getComponent<CTransform>();
    transform.pos = pos;
    transform.scale = sf::Vector2f(4.f, 4.f);
    transform.vel = sf::Vector2f(m_config.enemySpeed, 0);
    
}

void Scene_BulletNinja::spawnBox(sf::Vector2f pos) {
    auto m_box = _entityManager.addEntity("box");
    const sf::Texture& texture = Assets::getInstance().getTexture("box");
    m_box->addComponent<CTransform>(pos).scale = sf::Vector2f(.5f, .5f);
    m_box->addComponent<CState>().state = "box";
    m_box->addComponent<CBoundingBox>(sf::Vector2f(40.f, 40.f));
    m_box->addComponent<CSprite>(texture).sprite;
    m_box->addComponent<CScore>(1);
}

void Scene_BulletNinja::playerAttacks() {
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    auto& attack = m_player->getComponent<CInput>().attack;
    auto& pos = m_player->getComponent<CTransform>().pos;
    auto& state = m_player->getComponent<CState>().state;

    float speed = 5.f;


    if (attack & CInput::SWORD) {
        if (state != "attack_sword") {
            state = "attack_sword";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerAttackSword"));

        }

    }

    if (attack & CInput::SPEAR) {
        if (state != "attack_spear") {
            state = "attack_spear";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerAttackSpear"));
        }

    }


    if (attack != 0) {
        SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
        attack = 0;
        auto& animation = m_player->getComponent<CAnimation>().animation;

        
    }

    updateCamera();
}

//void Scene_BulletNinja::playerMovement()
//{
//    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
//        return;
//
//    auto& dir = m_player->getComponent<CInput>().dir;
//    auto& transform = m_player->getComponent<CTransform>();
//    auto& state = m_player->getComponent<CState>().state;
//    transform.prevPos = transform.pos;
//
//    
//
//    sf::Vector2f nextPos = transform.pos;
//
//    auto m_ground = m_worldView.getCenter().y + 40.f; // To do: get ground player y|| clean this later
//
//    bool isJumping = (state == "jump_left" || state == "jump_right");
//    bool onGround = Physics::isOnGround(m_player, {0, m_ground }, 0); // adjust height
//
//    if (dir & CInput::LEFT && !isJumping) {
//        if (state != "run_left") {
//            state = "run_left";
//            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"));
//            if (transform.scale.x != -std::abs(transform.scale.x))
//                nextPos.x -= 55.f;
//            transform.scale.x = -std::abs(transform.scale.x);
//        }
//        nextPos.x -= m_speed;
//    }
//
//    if (dir & CInput::RIGHT && !isJumping) {
//        if (state != "run_right") {
//            state = "run_right";
//            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"));
//            if (transform.scale.x != std::abs(transform.scale.x))
//                nextPos.x += 55.f;
//            transform.scale.x = std::abs(transform.scale.x);
//        }
//        nextPos.x += m_speed;
//    }
//
//    if ((dir & CInput::UP) && onGround) {
//        if (dir & CInput::LEFT) {
//            state = "jump_left";
//            transform.vel.x = -m_speed;
//        }
//        else if (dir & CInput::RIGHT) {
//            state = "jump_right";
//            transform.vel.x = m_speed;
//        }
//        else {
//            state = "jump_up";
//        }
//        transform.vel.y = m_jumpStrength;
//        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));
//    }
//
//    Physics::applyGravity(m_player, m_gravity, m_maxFallSpeed);
//
//    nextPos += transform.vel;
//
//    if (!isCollidingWithWalls(m_player, nextPos) && !isCollidingWithGround(m_player, nextPos))
//        transform.pos = nextPos;
//    else if (onGround && transform.vel.y >= 0) {
//        transform.vel.y = 0;
//        if (state.find("jump") != std::string::npos)
//            state = "idle";
//    }
//
//    if (dir != 0)
//        dir = 0;
//
//    updateCamera();
//}



void Scene_BulletNinja::playerMovement(sf::Time dt) {

    if (!m_player->hasComponent<CJump>()) return;
    
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;
    auto boxEntiies = _entityManager.getEntities("box");
    auto& dir = m_player->getComponent<CInput>().dir;
    auto& transform = m_player->getComponent<CTransform>();
    auto& state = m_player->getComponent<CState>().state;
    auto& jump = m_player->getComponent<CJump>();
    transform.prevPos = transform.pos;

    sf::Vector2f nextPos = transform.pos + transform.vel * dt.asSeconds();

    bool isJumping = (state == "jump_left" || state == "jump_right");
    sf::FloatRect entityBB = calculateBoundingBox(m_player, Hitbox);

    entityBB.left = nextPos.x;
    entityBB.top = nextPos.y;

    //std::cout << "entity nextpos: " << entityBB.left << ", " << entityBB.top << "\n";
    //std::cout << "current nextpos: " << transform.pos.x << ", " << transform.pos.y << ", " << entityBB.height << "\n";
    //std::cout << "current state: " << state << "\n";
    
    bool onGround = isOnGroundOrPlatforms(entityBB, boxEntiies,  m_ground.pos, m_ground.height);
    
    

    if (dir & CInput::LEFT && !isJumping) {
        
        if (state != "run_left") {
            state = "run_left";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"));
            if (transform.scale.x != -std::abs(transform.scale.x))
                nextPos.x -= 55.f;
            transform.scale.x = -std::abs(transform.scale.x);
        }
        nextPos.x -= m_speed * dt.asSeconds();
    }
    if (dir & CInput::RIGHT && !isJumping) {
        if (state != "run_right") {
            state = "run_right";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"));
            if (transform.scale.x != std::abs(transform.scale.x))
                nextPos.x += 55.f;
            transform.scale.x = std::abs(transform.scale.x);
        }
        nextPos.x += m_speed * dt.asSeconds();
    }

    if ((dir & CInput::UP) && onGround && !jump.isJumping) {
        jump.isJumping = true;
        jump.jumpTime = 0.f;
        if (dir & CInput::LEFT) {
            state = "jump_left";
        }
        else if (dir & CInput::RIGHT) {
            state = "jump_right";
        }
        else {
            state = "jump_up";
        }
        
            
        

        transform.vel.y = -m_jumpStrength;
        //m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerJump"));
    }

    if (onGround && jump.isJumping) {
        jump.jumpTime += dt.asSeconds();
        transform.pos.y = m_ground.pos.y - 1.f;
        transform.vel.x = 0;
        state = "idle";
        jump.isJumping = false;

    }

    if (!onGround && jump.isJumping) {
        jump.jumpTime += dt.asSeconds();
        
        if (state == "jump_left") {

            
            transform.vel.x = -m_speed;
        }
        else if (state == "jump_right") {
            
            transform.vel.x = m_speed;
        }
        

        if (jump.jumpTime < jump.maxJumpTime) {
            
            transform.vel.y -= m_gravity * dt.asSeconds(); // Go gradually upward
            
        }
        
    }


    

    // Prevent Falling Through Ground
    if (onGround && transform.vel.y > 0) {
        std::cout << "current transform.vel.y: " << transform.vel.y << "\n";
        transform.vel.y = 0.f;
        
    }

    

    if (!onGround) {
     
        Physics::applyGravity(m_player, m_gravity, m_maxFallSpeed);
    
    }


    nextPos += transform.vel;

    
    


    

    if (!isCollidingWithWalls(m_player, nextPos)) {
         
        transform.pos = nextPos;
    }

    if (onGround) {
        transform.pos.y = 339.f;
        
        dir = 0;
    }
    

    /*if (dir != 0) {
        SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
        dir = 0;
    }*/

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
            auto& state = entity->getComponent<CState>().state;
            auto& anim = entity->addComponent<CAnimation>();

            transform.pos = transform.prevPos;
            state = "idle";
            if (entity->getTag() == "player") anim.animation = Assets::getInstance().getAnimation("PlayerIdle");
            if (entity->getTag() == "enemy") anim.animation = Assets::getInstance().getAnimation("SamuraiIdle");

            


            return true;
        }
    }

    return false;
}



void Scene_BulletNinja::enemyMovement() {
    for (auto& enemy : _entityManager.getEntities("enemy")) {

        auto& enemyTransform = enemy->getComponent<CTransform>();

        // don't chase if enemy is out of view
        if (enemyTransform.pos.x > (m_worldView.getCenter().x + (m_worldView.getSize().x / 2)) || enemyTransform.pos.x < (m_worldView.getCenter().x - (m_worldView.getSize().x / 2))) continue;

        auto& state = enemy->getComponent<CState>().state;
        enemyTransform.prevPos = enemyTransform.pos;
        sf::Vector2f nextPos = enemyTransform.pos;
        std::string curAnim = enemy->getComponent<CAnimation>().animation.getName();
        std::string anim = "";

        if (!m_player || !m_player->hasComponent<CTransform>()) {
            continue;
        }

        

        auto& playerTransform = m_player->getComponent<CTransform>();
        sf::Vector2f playerPos = playerTransform.pos;

        float distance = std::abs(playerTransform.pos.x - enemyTransform.pos.x);
        auto& velocity = enemyTransform.vel;

        

        // Increase speed gradually / prevents sudden jumps
        float maxSpeed = m_config.enemySpeed;
        float acceleration = 0.0001f; // Adjust for smoother movement
        velocity.x = std::min(velocity.x + acceleration, maxSpeed);

        // If within attack range, stop moving and attack
        if (distance <= 60.f) {
            if (state != "attack_sword") {
                state = "attack_sword";
                anim = "SamuraiAttackSword";
            }
            updateEnemyMovement(enemy, nextPos, anim, curAnim, enemyTransform, playerTransform);
            continue;
        }

        /*if (isCollidingWithWalls(enemy, nextPos)) {
            updateEnemyMovement(enemy, nextPos, anim, curAnim, enemyTransform, playerTransform);
            continue;
        }*/

        // Update animation state and determine movement direction
        if (enemyTransform.pos.x < playerPos.x) {
            if (state != "run_right") {
                state = "run_right";
                anim = "SamuraiRun";
            }
            enemyTransform.scale.x = std::abs(enemyTransform.scale.x);
            nextPos.x += velocity.x;  // Move right
        }
        else if (enemyTransform.pos.x > playerPos.x) {
            if (state != "run_left") {
                state = "run_left";
                anim = "SamuraiRun";
            }
            enemyTransform.scale.x = -std::abs(enemyTransform.scale.x);
            nextPos.x -= velocity.x;
        }

        updateEnemyMovement(enemy, nextPos, anim, curAnim, enemyTransform, playerTransform);

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




void Scene_BulletNinja::adjustEntityMovement(std::shared_ptr<Entity> entity) {
    
    sf::FloatRect entityBB = calculateBoundingBox(entity, Hitbox);
    

    // Loop thu all boxees
    for (auto& wall : _entityManager.getEntities("box")) {
        sf::FloatRect wallBB = calculateBoundingBox(wall, Hitbox);
        
        if (entityBB.intersects(wallBB)) {
            
            auto& transform = entity->getComponent<CTransform>();
            auto& state = entity->getComponent<CState>().state;
            auto& anim = entity->addComponent<CAnimation>();
            
            transform.pos = transform.prevPos;
            state = "idle";
            if (entity->getTag() == "player") anim.animation = Assets::getInstance().getAnimation("PlayerIdle");
            if (entity->getTag() == "enemy") anim.animation = Assets::getInstance().getAnimation("SamuraiIdle");
            
            transform.vel.x = 0;

            
            return;
        }
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

    enemyMovement();
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

void Scene_BulletNinja::sCollisions() {
    auto entities = _entityManager.getEntities();

    for (auto& attacker : entities) {
        auto& attackerAnim = attacker->getComponent<CAnimation>().animation;
        auto& attackerKey = attacker->getComponent<CAnimation>().key;
        auto& attackerState = attacker->getComponent<CState>().state;
        auto& attackerScore = attacker->getComponent<CScore>();

        

        if ((attackerState == "attack_sword" || attackerState == "attack_spear") && attackerAnim.hasAttackbox()) {
            sf::FloatRect attackBox = calculateBoundingBox(attacker, Attackbox);

            for (auto& target : entities) {
                if (attacker == target || (attacker->getTag() == "enemy" && target->getTag() == "box")) continue;

                sf::FloatRect targetBox = calculateBoundingBox(target, Hitbox);
                auto& targetAnim = target->getComponent<CAnimation>().animation;
                auto& targetLife = target->getComponent<CScore>();

                if ((targetAnim.hasHitbox() || target->getTag() == "box") && attackBox.intersects(targetBox)) {

                    // Check if target already registered this attack
                    if (targetLife.canTakeHit(attackerKey)) {
                        targetLife.registerHit(attackerKey);

                        targetLife._hp -= 10;
                        attackerScore._score += 10;

                        if (targetLife._hp <= 0) {
                            targetLife._lives -= 1;
                            targetLife._hp = targetLife._DefautHp;
                        }
                        if (targetLife._lives <= 0) target->destroy();

                        std::cout << attacker->getTag() << " hit " << target->getTag() << "Key" << attackerKey << "!\n";
                    }
                }
            }
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
    registerAction(sf::Keyboard::Up, "UP");
    
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

    std::cout << "Object spawned at: " << x << "\n";
}

void Scene_BulletNinja::sSpawnStaticObjects()
{
    // spawn enemies when they are half a window above the current camera/view
    auto spawnLine = _game->window().getSize().x;
    std::cout << "_game->window().getSize().x: " << spawnLine << "\n";

    while (!_spawnPoints.empty() && _spawnPoints.top().x > 0 ) {
        SpawnStaticObject(_spawnPoints.top().name, _spawnPoints.top().x);
        _spawnPoints.pop();
    }
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
            //p.x = m_worldBounds.width - _game->windowSize().x - p.x;
            p.x = p.x;
            _spawnPoints.push(p);
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

//void Scene_BulletNinja::sDoAction(const Command& action)
//{
//    auto& input = m_player->getComponent<CInput>();
//    //// On Key Press
//    if (action.type() == "START") {
//        if (action.name() == "PAUSE") { setPaused(!_isPaused); }
//        else if (action.name() == "QUIT") { _game->quitLevel(); }
//        else if (action.name() == "BACK") { _game->backLevel(); }
//        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
//        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
//        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
//
//
//        //Set attacks
//        if (action.name() == "ATTACKSWORD") { input.attack |= CInput::SWORD; }
//        if (action.name() == "ATTACKSPEAR") { input.attack |= CInput::SPEAR; }
//
//        // Set movement state to "running"
//        //m_player->getComponent<CState>().state = "running";
//
//        // Set direction using bitwise OR (|=) to allow multiple directions
//        if (action.name() == "LEFT") { input.dir |= CInput::LEFT; }
//        if (action.name() == "RIGHT") { input.dir |= CInput::RIGHT; }
//        if (action.name() == "UP") { input.dir |= CInput::UP; }
//        if (action.name() == "DOWN") { input.dir |= CInput::DOWN; }
//
//        
//
//
//
//    }
//    
//    
//    
//    else if (action.type() == "END") {
//        if (action.name() == "LEFT") { input.dir &= ~CInput::LEFT; }
//        if (action.name() == "RIGHT") { input.dir &= ~CInput::RIGHT; }
//        if (action.name() == "UP") { input.dir &= ~CInput::UP; }
//        if (action.name() == "DOWN") { input.dir &= ~CInput::DOWN; }
//
//        // If no movement keys are pressed, set state to "idle"
//        if (input.dir == 0) {
//            m_player->getComponent<CState>().state = "idle";
//            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));
//            m_player->getComponent<CInput>().dir = 0;
//        }
//
//        /*if (input.attack == 0) {
//            m_player->getComponent<CState>().state = "idle";
//            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));
//            m_player->getComponent<CInput>().attack = 0;
//        }*/
//    }
//}


void Scene_BulletNinja::sDoAction(const Command& action)
{
    auto& input = m_player->getComponent<CInput>();
    auto& state = m_player->getComponent<CState>().state;

    if (action.type() == "START") {
        if (action.name() == "PAUSE") { setPaused(!_isPaused); }
        else if (action.name() == "QUIT") { _game->quitLevel(); }
        else if (action.name() == "BACK") { _game->backLevel(); }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }

        if (action.name() == "ATTACKSWORD") { input.attack |= CInput::SWORD; }
        if (action.name() == "ATTACKSPEAR") { input.attack |= CInput::SPEAR; }

        if (state.find("jump") == std::string::npos) {
            if (action.name() == "LEFT") { input.dir |= CInput::LEFT; }
            if (action.name() == "RIGHT") { input.dir |= CInput::RIGHT; }
        }
        if (action.name() == "UP") { input.dir |= CInput::UP; }
        if (action.name() == "DOWN") { input.dir |= CInput::DOWN; }
    }
    else if (action.type() == "END") {
        if (action.name() == "LEFT") { input.dir &= ~CInput::LEFT; }
        if (action.name() == "RIGHT") { input.dir &= ~CInput::RIGHT; }
        if (action.name() == "UP") { input.dir &= ~CInput::UP; }
        if (action.name() == "DOWN") { input.dir &= ~CInput::DOWN; }

        if (input.dir == 0 && state.find("jump") == std::string::npos) {
            state = "idle";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));
        }
    }
}

    
void Scene_BulletNinja::resetPlayer()
{
    sf::Vector2f position = m_worldView.getSize();
    position.x /= 2.0f;
    position.y -= 20.f;

    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
    m_player->addComponent<CTransform>(position);
    m_player->addComponent<CState>("none");

    m_timer = sf::seconds(60);
    m_maxHeight = position.y;
}

void Scene_BulletNinja::killPlayer()
{
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    auto& lives = _entityManager.getEntities("lives");
    lives.back()->destroy();
    m_lives -= 1;

    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
    m_player->addComponent<CState>("dead");

    SoundPlayer::getInstance().play("death");
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
    auto& state = e->getComponent<CState>().state;

    if (tag == "box") {
        size = sf::Vector2f{ box.size.x * 2, box.size.y * 2 };
        //position = sf::Vector2f{ position.x, position.y - (size.y / 2) };
    }

    if (tag == "player" || tag == "enemy") {
        if (state == "idle") {
            size = sf::Vector2f{ box.size.x - 1.f, box.size.y * transform.scale.y / 2.f };
            position = position - (size * 0.5f);
            if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
        }
        else if (state == "run_left" || state == "run_right") {
            size = sf::Vector2f{ box.size.x - 1.f, box.size.y * transform.scale.y / 2.f };
            position = position - (size * 0.5f);
            if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
        }
        else if (state == "attack_sword") {
            if (type == Hitbox) {
                size = sf::Vector2f{ box.size.x - 1.f, box.size.y * transform.scale.y / 2.f };
                position = position - (size * 0.5f);
                if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
            }
            if (type == Attackbox) {
                size = sf::Vector2f{ box.size.x * 5.f, box.size.y * transform.scale.y / 1.3f };
                position = sf::Vector2f{ position.x, position.y - (size.y / 2) };
            }
        }
        else if (state == "attack_spear") {
            if (type == Hitbox) {
                size = sf::Vector2f{ box.size.x + 25.f, box.size.y * transform.scale.y / 2.f };
                position = sf::Vector2f{ position.x, position.y - (size.y / 2) };
            }
            if (type == Attackbox) {
                size = sf::Vector2f{ box.size.x * 6.4f, box.size.y * transform.scale.y / 1.3f };
                position = sf::Vector2f{ position.x, position.y - (size.y / 2) };
                if (transform.scale.x == -std::abs(transform.scale.x)) position.x -= 30.f;
            }
        }
    }

    // Center the bounding box origin 
    sf::Vector2f centeredPosition = position - (size * 0.5f);

    return sf::FloatRect(centeredPosition, size);
}


void Scene_BulletNinja::drawHitbox(std::shared_ptr<Entity> e) {
    if (!m_drawAABB) return;

    auto& animation = e->getComponent<CAnimation>().animation;

    
    if (!animation.hasHitbox() && e->getComponent<CState>().state != "box") return;

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
    
    if (state.find("attack") != std::string::npos) std::cout << "attack box: " << boundingBox.height << "\n";
    
    
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
}

void Scene_BulletNinja::sUpdate(sf::Time dt) {
    SoundPlayer::getInstance().removeStoppedSounds();
    _entityManager.update();

    if (m_lives <= 0 || m_reachGoal >= 5)
        _game->quitLevel();

    if (_isPaused)
        return;

    /*m_timer -= dt;

    if (m_timer.asSeconds() <= 0)
        killPlayer();*/

    checkPlayerState();

    sAnimation(dt);
    sMovement(dt);
    sCollisions();

    updateScore();
}


void Scene_BulletNinja::sAnimation(sf::Time dt) {
    auto list = _entityManager.getEntities();
    for (auto e : _entityManager.getEntities()) {
        
        // update all animations
        if (e->hasComponent<CAnimation>()) {
            auto& animation = e->getComponent<CAnimation>().animation;

            if (e->getTag() == "turtles" && e->getComponent<CState>().state != "animated")
                continue;

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
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
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




