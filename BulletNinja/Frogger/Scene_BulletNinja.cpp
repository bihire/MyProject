
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
    

    //auto pos = m_worldView.getSize();

    // To refactor
    auto playerPos = sf::Vector2f{ spawnPos.x, spawnPos.y + 40.f };
    auto boxPos = sf::Vector2f{ spawnPos.x + 200.f, spawnPos.y };
    auto ePos = sf::Vector2f{ boxPos.x + 200.f, playerPos.y };
    spawnPlayer(playerPos);
    

    spawnBox(boxPos);


    spawnEnemy(ePos);

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
    transform.vel = sf::Vector2f(m_config.enemySpeed, m_config.enemySpeed);
    
}

void Scene_BulletNinja::spawnBox(sf::Vector2f pos) {
    auto m_box = _entityManager.addEntity("box");
    const sf::Texture& texture = Assets::getInstance().getTexture("box");
    m_box->addComponent<CTransform>(pos).scale = sf::Vector2f(.5f, .5f);
    //m_player->addComponent<CState>().state = "idle";
    m_box->addComponent<CBoundingBox>(sf::Vector2f(40.f, 40.f));
    m_box->addComponent<CInput>();
    m_box->addComponent<CSprite>(texture).sprite;
}

void Scene_BulletNinja::playerAttacks() {
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    auto& attack = m_player->getComponent<CInput>().attack;
    auto& pos = m_player->getComponent<CTransform>().pos;

    float speed = 5.f;

    

    if (attack & CInput::SWORD) {
        if (m_player->getComponent<CState>().state != "PlayerAttackSword") {
            m_player->getComponent<CState>().state = "PlayerAttackSword";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerAttackSword"));

            /*auto trans = m_player->getComponent<CTransform>().scale;
            m_player->getComponent<CTransform>().scale.x = -std::abs(trans.x);*/
        }
        //pos.x -= speed;
    }

    if (attack & CInput::SPEAR) {
        if (m_player->getComponent<CState>().state != "PlayerAttackSpear") {
            m_player->getComponent<CState>().state = "PlayerAttackSpear";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerAttackSpear"));

            //auto trans = m_player->getComponent<CTransform>().scale;
            //m_player->getComponent<CTransform>().scale.x = std::abs(trans.x);
        }
        //pos.x += speed;
    }


    if (attack != 0) {
        SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
        attack = 0;
    }

    updateCamera();
}

void Scene_BulletNinja::playerMovement() {
    // no movement if player is dead
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    auto& dir = m_player->getComponent<CInput>().dir;
    auto& pos = m_player->getComponent<CTransform>().pos;
    
    float speed = 5.f;

    if (dir & CInput::UP) {
        
    }
    if (dir & CInput::DOWN) {
        
    }

    if (dir & CInput::LEFT) {
        if (m_player->getComponent<CState>().state != "PlyerRunLeft") {
            m_player->getComponent<CState>().state = "PlayerRunLeft";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"));

            auto trans = m_player->getComponent<CTransform>().scale;
            m_player->getComponent<CTransform>().scale.x = -std::abs(trans.x);
        }
        pos.x -= speed;
    }

    if (dir & CInput::RIGHT) {
        if (m_player->getComponent<CState>().state != "PlayerRunRight") {
            m_player->getComponent<CState>().state = "PlayerRunRight";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"));

            auto trans = m_player->getComponent<CTransform>().scale;
            m_player->getComponent<CTransform>().scale.x = std::abs(trans.x);
        }
        pos.x += speed;
    }
        

    if (dir != 0) {
        SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
        dir = 0;
    }

    updateCamera();
}

void Scene_BulletNinja::enemyMovement() {
    for (auto& enemy : _entityManager.getEntities("enemy")) {
        

        auto& enemyTransform = enemy->getComponent<CTransform>();
        sf::Vector2f enemyPos = enemyTransform.pos;
       // std::cout << "enemy scale: " << enemyTransform.scale << "\n";
       // std::cout << "enemy pos: x: " << enemyTransform.pos.x << " ,y: " << enemyTransform.pos.x << "\n";
        //enemyTransform.scale = sf::Vector2f{ 4.f, 4.f }; // this is the get around but there is more side effects

        // Ensure player exists bfr accessing its components
        if (!m_player || !m_player->hasComponent<CTransform>()) {
            continue;
        }

        auto& playerTransform = m_player->getComponent<CTransform>();
        sf::Vector2f playerPos = playerTransform.pos;

        //std::cout << "Player pos: " << playerPos.x << "\n";
        //std::cout << "enemy pos: " << enemyPos.x << "\n";
        // Compute distance from player
        float distance = std::hypot(playerPos.x - enemyPos.x, playerPos.y - enemyPos.y);

        

        auto& velocity = enemy->getComponent<CTransform>().vel;

        // Increase speed Gradually / prevents sudden jumps
        float maxSpeed = m_config.enemySpeed;
        float acceleration = 0.0005f; // Adjust for smoother movement
        velocity.x = std::min(velocity.x + acceleration, maxSpeed);
        //velocity.y = std::min(velocity.y + acceleration, maxSpeed);

        // If within attack range
        if (distance <= 100.f) {
            if (enemy->getComponent<CState>().state != "SamuraiAttackSword") {
                enemy->getComponent<CState>().state = "SamuraiAttackSword";
                enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiAttackSword"));
                //SoundPlayer::getInstance().play("enemy_attack", enemyPos);
            }
            continue;
        }

        

        // Update animation state
        
        if (enemyPos.x < playerPos.x) {
            if (enemy->getComponent<CState>().state != "SamuraiRunRight") {
                enemy->getComponent<CState>().state = "SamuraiRunRight";
                enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiRun"));
            }
            
            enemyTransform.scale.x = std::abs(enemyTransform.scale.x);
        }else if (enemyPos.x > playerPos.x) {
            if (enemy->getComponent<CState>().state != "SamuraiRunLeft") {
                enemy->getComponent<CState>().state = "SamuraiRunLeft";
                enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiRun"));
            }
            enemyTransform.scale.x = -std::abs(enemyTransform.scale.x);
        }

        // Move toward player
        sf::Vector2f direction = { playerPos.x - enemyPos.x, playerPos.y - enemyPos.y };
        float length = std::hypot(direction.x, direction.y); // from https://en.cppreference.com/w/cpp/numeric/math/hypot to calculate the distnce
        if (length > 0) {
            direction /= length;
            //std::cout << "Normalize: " << direction.x << " velocity: " << velocity.x << "\n";
            enemyTransform.pos += direction * velocity.x;
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
    playerMovement();
    enemyMovement();
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
    adjustPlayerPosition();

    for (auto& e : _entityManager.getEntities())
    {
        const float offset = 20.0f;

        auto& bounds = m_worldView.getSize();

        auto& transform = e->getComponent<CTransform>();
        auto& half = e->getComponent<CBoundingBox>().halfSize;

        auto& position = transform.pos;
        auto& velocity = transform.vel;

        if (velocity.x < 0 && position.x + half.x + offset < 0)
            position.x = bounds.x + half.x + offset;

        if (velocity.x > 0 && position.x - half.x - offset > bounds.x)
            position.x = -half.x - offset;
    }

    auto& playerPosition = m_player->getComponent<CTransform>().pos;

    
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
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    
    registerAction(sf::Keyboard::Down, "DOWN");
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
    //// On Key Press
    if (action.type() == "START") {
        if (action.name() == "PAUSE") { setPaused(!_isPaused); }
        else if (action.name() == "QUIT") { _game->quitLevel(); }
        else if (action.name() == "BACK") { _game->backLevel(); }
        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }


        //Set attacks
        if (action.name() == "ATTACKSWORD") { input.attack |= CInput::SWORD; }
        if (action.name() == "ATTACKSPEAR") { input.attack |= CInput::SPEAR; }

        // Set movement state to "running"
        //m_player->getComponent<CState>().state = "running";

        // Set direction using bitwise OR (|=) to allow multiple directions
        if (action.name() == "LEFT") { input.dir |= CInput::LEFT; }
        if (action.name() == "RIGHT") { input.dir |= CInput::RIGHT; }
        if (action.name() == "UP") { input.dir |= CInput::UP; }
        if (action.name() == "DOWN") { input.dir |= CInput::DOWN; }

        



    }
    
    
    
    else if (action.type() == "END") {
        if (action.name() == "LEFT") { input.dir &= ~CInput::LEFT; }
        if (action.name() == "RIGHT") { input.dir &= ~CInput::RIGHT; }
        if (action.name() == "UP") { input.dir &= ~CInput::UP; }
        if (action.name() == "DOWN") { input.dir &= ~CInput::DOWN; }

        // If no movement keys are pressed, set state to "idle"
        if (input.dir == 0) {
            m_player->getComponent<CState>().state = "idle";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerIdle"));
            m_player->getComponent<CInput>().dir = 0;
        }

        /*if (input.attack == 0) {
            m_player->getComponent<CState>().state = "idle";
            m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));
            m_player->getComponent<CInput>().attack = 0;
        }*/
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
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    auto& position = m_player->getComponent<CTransform>().pos;

    if (position.y < m_maxHeight)
    {
        m_maxHeight = position.y;

        if (position.y != 600.0f - 260.0f)
            m_score += 10;
    }
}

void Scene_BulletNinja::drawAABB(std::shared_ptr<Entity> e) {
    if (m_drawAABB) {
        /*auto box = e->getComponent<CBoundingBox>();
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f{ box.size.x * 2, box.size.y * 2 });
        centerOrigin(rect);
        rect.setPosition(e->getComponent<CTransform>().pos);
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        rect.setOutlineColor(sf::Color{ 0, 255, 0 });
        rect.setOutlineThickness(2.f);
        _game->window().draw(rect);*/
        auto box = e->getComponent<CBoundingBox>();
        auto transform = e->getComponent<CTransform>();

        sf::RectangleShape rect;

        auto& size = box.size;
        auto position = e->getComponent<CTransform>().pos;

        auto& cmp = e->getTag();
        if (cmp == "box") {
            size = sf::Vector2f{ box.size.x * 2, box.size.y * 2 };
        }

        if (cmp == "player" && e->getComponent<CState>().state == "idle") {
            size = sf::Vector2f{ box.size.x - 1.f, box.size.y * transform.scale.y / 2.f };
            position = sf::Vector2f{ position.x - (box.size.x / 2.f) - 3.f, position.y - (box.size.y/2.f) };
        }

        if (cmp == "player" && e->getComponent<CState>().state == "PlayerAttackSword") {
            size = sf::Vector2f{ box.size.x * 6.4f, box.size.y * transform.scale.y / 1.3f };
            position = sf::Vector2f{ (position.x * 2.f) - (box.size.x) + 50.f, position.y - (box.size.y / 2) };
        }

        if (cmp == "player" && e->getComponent<CState>().state == "PlayerAttackSpear") {
            size = sf::Vector2f{ box.size.x * 6.4f, box.size.y * transform.scale.y / 1.3f };
            position = sf::Vector2f{  (position.x * 2.f) - (box.size.x ) + 50.f, position.y - (box.size.y/2) };
        }
        

        rect.setSize(size);
        centerOrigin(rect);

        rect.setPosition(position);
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        rect.setOutlineColor(sf::Color{ 0, 255, 0 });
        rect.setOutlineThickness(2.f);
        _game->window().draw(rect);
        

    }
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

    for (auto e : _entityManager.getEntities("box")) {
        if (e->getComponent<CSprite>().has) {
            auto& anim = e->getComponent<CSprite>().sprite;
            auto& tfm = e->getComponent<CTransform>();
            anim.setPosition(tfm.pos);
            anim.setRotation(tfm.angle);
            anim.setScale(tfm.scale);

            drawAABB(e);
            
            _game->window().draw(anim);
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

        drawAABB(e);

    }

    _game->window().setView(_game->window().getDefaultView());

    m_text.setPosition(5.0f, -5.0f);
    m_text.setString("score  " + std::to_string(m_score));
    _game->window().draw(m_text);

    int time = static_cast<int>(std::ceil(m_timer.asSeconds()));

    m_text.setPosition(5.0f, 22.5f);
    m_text.setString("time  " + std::to_string(time));
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
    //sCollisions();

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



