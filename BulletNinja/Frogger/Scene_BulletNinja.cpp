
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

    

    auto pos = m_worldView.getSize();

    // spawn frog in middle of first row
    pos.x = pos.x / 2.f;
    pos.y -= pos.y / 2.f;
    spawnPlayer(pos);

    m_timer = sf::seconds(60.0f);
    m_maxHeight = pos.y;
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
    m_player->addComponent<CTransform>(pos);
    m_player->addComponent<CBoundingBox>(sf::Vector2f(15.f, 15.f));
    m_player->addComponent<CInput>();
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("SamuraiIdle"));
}

void Scene_BulletNinja::playerMovement() {
    // no movement if player is dead
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    auto& dir = m_player->getComponent<CInput>().dir;
    auto& pos = m_player->getComponent<CTransform>().pos;

    if (dir & CInput::UP) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
        pos.y -= 40.f;
    }
    if (dir & CInput::DOWN) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
        pos.y += 40.f;
    }

    if (dir & CInput::LEFT) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("left"));
        pos.x -= 40.f;
    }

    if (dir & CInput::RIGHT) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("right"));
        pos.x += 40.f;
    }

    if (dir != 0) {
        SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
        dir = 0;
    }
}
void Scene_BulletNinja::sMovement(sf::Time dt) {
    playerMovement();

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

void Scene_BulletNinja::spawnLane10()
{
    sf::Vector2f position(350.0f, 600.0f - 460.0f);
    sf::Vector2f velocity(50.0f, 0.0f);

    for (int i = 0; i < 3; ++i)
    {
        auto tree = _entityManager.addEntity("tree");
        tree->addComponent<CAnimation>(Assets::getInstance().getAnimation("tree1"));
        tree->addComponent<CBoundingBox>(sf::Vector2f(70.0f, 15.0f));
        tree->addComponent<CTransform>(position, velocity);
        position.x -= 175.0f;
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

    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::Left, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Right, "RIGHT");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
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
            auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
            sprite.setOrigin(0.f, 0.f);
            sprite.setPosition(pos);
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
    //// On Key Press
    if (action.type() == "START") {
        if (action.name() == "PAUSE") { setPaused(!_isPaused); }
        else if (action.name() == "QUIT") { _game->quitLevel(); }
        else if (action.name() == "BACK") { _game->backLevel(); }

        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
    
        // Player control
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().dir = CInput::LEFT; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
        else if (action.name() == "UP") { m_player->getComponent<CInput>().dir = CInput::UP; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().dir = CInput::DOWN; }
    }
    
    //// the frog can only go in one direction at a time, no angles
    
    else if (action.type() == "END" && (action.name() == "LEFT" || action.name() == "RIGHT" || action.name() == "UP" ||
        action.name() == "DOWN")) {
        m_player->getComponent<CInput>().dir = 0;
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
        _game->window().draw(anim.getSprite());

        if (m_drawAABB) {
            if (e->hasComponent<CBoundingBox>()) {
                auto box = e->getComponent<CBoundingBox>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
                centerOrigin(rect);
                rect.setPosition(e->getComponent<CTransform>().pos);
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color{ 0, 255, 0 });
                rect.setOutlineThickness(2.f);
                _game->window().draw(rect);
            }
        }
    }

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

    m_timer -= dt;

    if (m_timer.asSeconds() <= 0)
        killPlayer();

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



