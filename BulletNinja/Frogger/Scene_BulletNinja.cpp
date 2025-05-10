
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

namespace {
    std::random_device rd;
    std::mt19937 rng(rd());
}


Scene_BulletNinja::Scene_BulletNinja(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine), m_worldView(gameEngine->window().getDefaultView())
{   
    auto& playerProgress = PlayerProgress::getInstance();
    playerProgress.setLives(3);
    playerProgress.setResetCurrentLevelCallBack([this]() {
        resetLevel();
        
        });
    init(levelPath);
    
}

void Scene_BulletNinja::resetLevel() {
    respawnPlayer();
    auto& score = m_player->getComponent<CScore>();
    score._hp = score._originalHp;
    score._lives = score._originallives;
    resetAllEntities();

    for(auto& e : _entityManager.getEntities("trophyKey")) {
        e->isActive();
			e->getComponent<CKeyState>().reset();
		
	}
}

void Scene_BulletNinja::init(const std::string& levelPath) {
    if (sf::Shader::isAvailable())
    {
        std::cout <<  "shaders are not available..." << std::endl;
    }
    
    sf::Vector2u windowSize = _game->window().getSize();

    // set m config
    m_config.enemyChaseRange = windowSize.x;
    
    //float gameplayWidth = 600.f;
    float gameplayWidth = windowSize.x ;
    float gameplayHeight = windowSize.y;

    // set view window width
    m_sceneDimensions.m_gameplayWidth = gameplayWidth/2;

    //set m_world height
    m_ground.pos.y = (gameplayHeight*2)/3;
    
    float viewportWidth = gameplayWidth / windowSize.x;
    float viewportX = (1.0f - viewportWidth) / 2.0f;

    
    m_worldView.setSize(gameplayWidth, gameplayHeight);
    m_worldView.setCenter(gameplayWidth / 2, gameplayHeight / 2);
    m_worldView.setViewport(sf::FloatRect(viewportX, 0.f, viewportWidth, 1.f));




    // initialization
    loadLevel(levelPath);
    registerActions();

    m_text.setFont(Assets::getInstance().getFont("Arcade"));
    m_text.setFillColor(sf::Color::Black);
    sf::Vector2f spawnPos{ m_worldView.getSize().x / 2.f, m_worldView.getSize().y / 2.f };
    
    m_worldView.setCenter(spawnPos);
    sSpawnStaticObjects();
    spawnFlowerEnemies();
    spawnTiles();
    sSpawnDoorObjects();


    auto playerPos = sf::Vector2f{ m_doorGap /2.f, spawnPos.y - 200.f };
    auto boxPos = sf::Vector2f{ playerPos.x + 800.f, playerPos.y + 200.f };
    auto ePos = sf::Vector2f{ boxPos.x + 200.f, playerPos.y + 40.f };


   
    spawnPlayer(playerPos);
    spawnWoodenBoxes();
    spawnKey();
    spawnNinjaEnemies();

    m_timer = sf::seconds(60.0f);
    m_maxHeight = spawnPos.y;
    m_score = 0;
    m_lives = 3;
    m_reachGoal = 0;

    //MusicPlayer::getInstance().play("gameTheme");
    //MusicPlayer::getInstance().setVolume(50);
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










void Scene_BulletNinja::sMovement(sf::Time dt) {
    playerMovement(dt);

    enemyMovement(dt);

    playerAttacks();
    flowerEnemyMovement();

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
            if (!e->isActive() || tag == "player" || tag == "staticObject" || tag == "flowerEnemy" || tag=="trophyKey")
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
                    resolveCollision(player, e, overlap);
                    pCurrBB = calculateBoundingBox(player, BBType::Hitbox); // Update after resolution
                }
                continue;
            }

            // Normal collision for other entities
            sf::FloatRect eCurrBB = calculateBoundingBox(e, BBType::Hitbox);
            sf::Vector2f overlap = Physics::getOverlap(pCurrBB, eCurrBB);

            if (overlap.x > 0.f && overlap.y > 0.f) {
                resolveCollision(player, e, overlap);
                pCurrBB = calculateBoundingBox(player, BBType::Hitbox);
            }
        }
    }
}







void Scene_BulletNinja::registerActions() {
    registerAction(sf::Keyboard::Space, "PAUSE");
    registerAction(sf::Keyboard::Escape, "BACK");
    registerAction(sf::Keyboard::Q, "QUIT");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::M, "TOGGLE_IN_GAME_MENU");


    registerAction(sf::Keyboard::A, "ATTACKSWORD");
    registerAction(sf::Keyboard::S, "ATTACKSPEAR");

    registerAction(sf::Keyboard::Left, "LEFT");
    registerAction(sf::Keyboard::Right, "RIGHT");
    registerAction(sf::Keyboard::Up, "JUMP");
    registerAction(sf::Keyboard::Down, "DOWN");
}

void Scene_BulletNinja::SpawnStaticObject(std::string name, float x) {
    auto _object = _entityManager.addEntity("staticObject");
    _object->addComponent<CTransform>();
    _object->getComponent<CState>();
    _object->addComponent<CAnimation>(Assets::getInstance().getAnimation(name));



    auto& transform = _object->getComponent<CTransform>();

    sf::Vector2f spawnPos{ m_worldView.getSize().x / 2.f , m_worldView.getSize().y / 2.f };
    transform.pos = sf::Vector2f(x, m_ground.pos.y - 100.f);
    transform.scale = sf::Vector2f(1.5f, 1.5f);

 
}




void Scene_BulletNinja::sSpawnStaticObjects()
{
    // spawn enemies when they are half a window above the current camera/view
    auto spawnLine = _game->window().getSize().x;

    while (!_spawnStaticPoints.empty() && _spawnStaticPoints.top().x > 0 ) {
        SpawnStaticObject(_spawnStaticPoints.top().name, _spawnStaticPoints.top().x);
        _spawnStaticPoints.pop();
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

            
        }
        
        else if (token == "Speeds") {
            config >> m_config.cameraReactionSpeed  >> m_config.playerSpeed >> m_config.enemySpeed;
        }
        else if (token == "Tree") {
            SpawnPoint p;
            config >> p.name >> p.x;
            
            _spawnStaticPoints.push(p);
        }
        else if (token == "NinjaEnemy") {
            SpawnEnemyNinjaPoint p;
            config >> p.x >> p.BBx >> p.BBy >> p.lives >> p.health >> p.level;
            
            _spawnEnemyNinjaPoints.push(p);
        }
        else if (token == "Door") {
            SpawnDoorPoint p;
            config >> p.name >> p.x >> p.type;

            // Adjust worlld size
            if(p.x + m_doorGap > m_worldBounds.width) {
				m_worldBounds.width = p.x + m_doorGap;
			}
            // If it's an exit door, read required score
            if (p.type == 1) {
                config >> p.requiredScore;
                PlayerProgress::getInstance().incrementLastLevel();
            }

            config >> p.id;

            _spawnDoorPoints.push(p);
        }
		else if (token == "WoodenBox") {
			SpawnWoodenBoxPoint p;
			config >> p.x >> p.width >> p.height >> p.level;
			_spawnWoodenBoxPoints.push(p);
		}
        else if (token == "FlowerEnemy") {
            spawnFlowerEnemyPoint p;
            config >> p.x >> p.width >> p.height;
            _spawnFlowerEnemyPoints.push(p);
        }
        else if (token == "Tile") {
			SpawnTilePoint p;
			config >> p.x >> p.y >> p.width >> p.height;
			_spawnTilePoints.push(p);
		}
		else if (token == "Ground") {
			config >> m_ground.pos.x >> m_ground.pos.y >> m_ground.width >> m_ground.height;
		}
		else if (token == "Player") {
			config >> token;
			spawnPlayer(sf::Vector2f{ m_worldView.getSize().x / 2.f, m_worldView.getSize().y / 2.f });
		}
		else if (token == "Enemy") {
			config >> token;
			spawnEnemy(sf::Vector2f{ m_worldView.getSize().x / 2.f, m_worldView.getSize().y / 2.f });
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
        if (action.name() == "PAUSE") { _game->quitLevel(); }
        else if (action.name() == "QUIT") { _game->quitLevel(); }
        else if (action.name() == "BACK") { _game->backLevel(); }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
        else if (action.name() == "TOGGLE_IN_GAME_MENU") { PlayerProgress::getInstance().setIsInGameMenu(); }

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




 




void Scene_BulletNinja::renderGamePlayBackgroundView()
{
    // Get window dimensions
    sf::Vector2u windowSize = _game->window().getSize();
    
    float stripWidth = (windowSize.x - m_sceneDimensions.m_gameplayWidth) / 2.0f;

    // Draw left and right side , easier than drawing the middle
    if (stripWidth > 0)
    {
        
        _game->window().setView(_game->window().getDefaultView());

        if (PlayerProgress::getInstance().isInGameMenu()) {
            
            leftStripMenu(sf::Vector2f(stripWidth, windowSize.y));
            
            rightStripMenu(sf::Vector2f(stripWidth, windowSize.y));
            //
            
        }
    }

    
    _game->window().setView(m_worldView);


    // Set the gameplay view
    _game->window().setView(m_worldView);
}

void Scene_BulletNinja::sRender()
{
    
   
    
    try {
        // get shadow shader
        auto& shadowShader = Assets::getInstance().getShader("shadow");
        sf::RenderStates shadowStates;
        shadowStates.shader = &shadowShader;
        shadowStates.blendMode = sf::BlendAlpha;

        // Now setUniform will work
        shadowShader.setUniform("texture", sf::Shader::CurrentTexture);
        shadowShader.setUniform("offset", sf::Vector2f(5.f, 10.f));

    // Draw background first
    for (auto e : _entityManager.getEntities("bkg")) {
        if (e->getComponent<CSprite>().has) {
            auto& sprite = e->getComponent<CSprite>().sprite;
            _game->window().draw(sprite);
        }
    }

    renderTrophyKey();


    // Draw entities with animations - WITH SHADOWS
    for (auto& e : _entityManager.getEntities()) {
            if (!e->isActive() || !e->hasComponent<CAnimation>()) continue;

            auto& anim = e->getComponent<CAnimation>().animation;
            auto& tfm = e->getComponent<CTransform>();
            auto& sprite = anim.getSprite();


            

            // Calculate base shadow position / mirror effect
            float groundLevel = m_ground.pos.y;
            float spriteHeight = sprite.getLocalBounds().height;
            float verticalOffset = 10.0f;

            // Mirror position calculation
            float shadowY = groundLevel * 2 - verticalOffset;

            
            float scaleFactor = 1.0f - (tfm.vel.y * 0.004f);
            scaleFactor = std::clamp(scaleFactor, 0.8f, 1.2f);

            // 4. Apply transforms
            shadowStates.transform = sf::Transform::Identity;
            shadowStates.transform.translate(0, shadowY);
            shadowStates.transform.scale(1.0f, -scaleFactor);
            shadowStates.transform.rotate(tfm.angle);
            _game->window().draw(sprite, shadowStates);

            // Draw original sprite
            sprite.setPosition(tfm.pos);
            sprite.setRotation(tfm.angle);
            sprite.setScale(tfm.scale);
            _game->window().draw(sprite);

            drawHitbox(e);
            drawAttackBox(e);
        }

    

    // Draw boxes
    for (auto e : _entityManager.getEntities("box")) {
        if (!e->isActive()) continue;
        if (e->getComponent<CSprite>().has) {
            auto& anim = e->getComponent<CSprite>().sprite;
            auto& tfm = e->getComponent<CTransform>();
            anim.setPosition(tfm.pos);
            anim.setRotation(tfm.angle);
            anim.setScale(tfm.scale);

            drawHitbox(e);
            drawAttackBox(e);
            _game->window().draw(anim);
        }
    }

    renderHealthIndicators();
    drawGround(m_ground);
    DrawTiles();
    
    renderKeyIndicator();
    

    

    
    renderWasted();
    renderLevelComplete();
    renderGameComplete();



    }
    catch (const std::exception& e) {
        std::cerr << "Shader error: " << e.what() << std::endl;
        
        // fal back
    }

    renderGamePlayBackgroundView();
}

void Scene_BulletNinja::sUpdate(sf::Time dt) {
    SoundPlayer::getInstance().removeStoppedSounds();
    _entityManager.update();
    updateDeadEnties(); // Todo- fix this for a long time

    if (m_lives <= 0 || m_reachGoal >= 5)
        _game->quitLevel();

    if (PlayerProgress::getInstance().isPaused())
        return;
    auto& anim = m_player->getComponent<CAnimation>();
    auto& state = m_player->getComponent<CState>();

    checkPlayerState();

    sAnimation(dt);
    sMovement(dt);
    attackCollisions();
    updateDoors(dt);

    checkAndAppyGravity();
    sCollisions();
    updateHits(dt);
    updateKeyLocation(dt);
    
    updateScore();
    updateRespawns(dt);



    auto& wasted = m_player->getComponent<CWasted>();
    auto& levelComplete = m_player->getComponent<CLevelComplete>();
    auto& gameComplete = m_player->getComponent<CGameComplete>();
    if(wasted.isWasted == true){
        wasted.update(dt);
    }
    if (levelComplete.isActive == true) {
        levelComplete.update(dt);
    }
    if (gameComplete.isActive == true) {
        gameComplete.update(dt);
    }
}

void Scene_BulletNinja::updateDeadEnties() {
	for (auto e : _entityManager.getEntities()) {
		if (!e->isActive() && e->hasComponent<CTransform>()) {
			auto& transform = e->getComponent<CTransform>();
			transform.vel = sf::Vector2f(0, 0);
		}
	}

}

void Scene_BulletNinja::updateHits(sf::Time dt) {
    for (auto e : _entityManager.getEntities()) {
        if (!e->hasComponent<CHitEffect>()) continue;

        auto& state = e->getComponent<CState>();

        auto& hitEffect = e->getComponent<CHitEffect>();
        hitEffect.update(dt, e->getComponent<CAnimation>().animation.getSprite(), state);
    }
}


void Scene_BulletNinja::sAnimation(sf::Time dt) {
    auto list = _entityManager.getEntities();
    for (auto e : _entityManager.getEntities()) {
        
        // update all animations
        if (e->hasComponent<CAnimation>()) {
            auto& animation = e->getComponent<CAnimation>().animation;
            // Don't animate door at start
            if(animation.getName() == "door" && e->getComponent<CDoorState>().state == CDoorState::Closed && animation.getCurFrame() == 0) continue;
            

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



