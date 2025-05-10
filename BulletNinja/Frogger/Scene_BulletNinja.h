//
// Created by David Burchill on 2023-09-27.
//

#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"
#include <queue>
#include <cmath>

enum BBType
{
    Hitbox,
    Attackbox
};

struct SpawnTilePoint {

    float x;
    float y;
    float width;
    float height;
    auto operator<(const SpawnTilePoint& other) const {
        return x > other.x;
    }
};

struct SpawnEnemyNinjaPoint {

    float x;
    float BBx;
    float BBy;
    float scale;
    size_t lives;
    int   health;
    int   level;
    auto operator<(const SpawnEnemyNinjaPoint& other) const {
        return x > other.x;
    }
};

struct SpawnWoodenBoxPoint {
	
	float x;
	float width;
	float height;
    size_t level;
	auto operator<(const SpawnWoodenBoxPoint& other) const {
		return x > other.x;
	}
};

struct spawnFlowerEnemyPoint {

    float x;
    float width;
    float height;
    auto operator<(const spawnFlowerEnemyPoint& other) const {
        return x > other.x;
    }
};


struct SpawnDoorPoint {
    std::string name;
    float x;
    int type;  // 0 = entrance, 1 = exit
    int requiredScore = 0;  // Only used for exit doors
    size_t id = 0; // ID for the door

    auto operator<=>(const SpawnDoorPoint& other) const {
        return x <=> other.x;
    }
};


struct SpawnPoint {
    std::string name;
    float       x;

    auto operator<=>(const SpawnPoint& other) const {
        return x <=> other.x;
    }
};

struct LevelConfig {
    float       cameraReactionSpeed{ 100.f };
    float       playerSpeed{ 100.f };
    float       enemySpeed{ 100.f };
    float       enemyAttackRange{ 60.f };
    float       enemyChaseRange{ 150.f };
    
};

struct GroundCoord {
    sf::Vector2f pos{ 0, 582.f };
    float width{ 5000.f };
    float height{ 2.f };
};

struct SceneDimensions {
    float m_screenWidth;
    float m_screenHeight;
    float m_gameplayWidth{ 600.f };
    float m_gameplayHeight;

    float viewportWidth;
    float viewportX;
};

class Scene_BulletNinja : public Scene {
private:
    sPtrEntt        m_player{ nullptr };
    sf::View        m_worldView;
    SceneDimensions m_sceneDimensions;
    sf::FloatRect   m_worldBounds;
    LevelConfig     m_config;
    GroundCoord     m_ground;
    std::priority_queue<SpawnPoint>             _spawnStaticPoints;
    std::priority_queue<SpawnDoorPoint>         _spawnDoorPoints;
    std::priority_queue<SpawnWoodenBoxPoint>    _spawnWoodenBoxPoints;
    std::priority_queue<spawnFlowerEnemyPoint>  _spawnFlowerEnemyPoints;
    std::priority_queue<SpawnTilePoint>         _spawnTilePoints;
    std::priority_queue<SpawnEnemyNinjaPoint>   _spawnEnemyNinjaPoints;
    float           m_doorGap{ 700.f };

    float           m_speed = 15.f;
    float           m_jumpStrength = 18.f;
    float           m_gravity = 0.9f;
    float           m_maxFallSpeed = 15.f;

    bool			m_drawTextures{ true };
    bool			m_drawAABB{ false };
    bool			m_drawGrid{ false };
    std::shared_ptr<Entity> m_nearestExitDoor;

    sf::Text        m_text;
    sf::Time        m_timer;
    float           m_maxHeight;
    int             m_score;
    int             m_lives{ 3 };
    int             m_reachGoal;

    //systems
    void            sMovement(sf::Time dt);
    void            sCollisions();
    void            sUpdate(sf::Time dt);
    void            sAnimation(sf::Time dt);
    void            updateDeadEnties();

    void	        onEnd() override;


    // helper functions
    void            playerMovement(sf::Time dt);
    void            flowerEnemyMovement();

    void            playerCheckState();
    bool            hasObstacleBetween(std::shared_ptr<Entity> enemy, const sf::Vector2f& targetPos);
    void            resolveCollision(std::shared_ptr<Entity> player, std::shared_ptr<Entity> door, sf::Vector2f overlap);

    // Collision
    bool            isCollidingWithWalls(std::shared_ptr<Entity> e, sf::Vector2f& nextPos);
    bool			isOnGroundOrPlatforms(const sf::FloatRect& entityBB, const std::vector<std::shared_ptr<Entity>>& platforms, const sf::Vector2f& groundPos, float groundHeight);
    bool            shouldCollide(std::shared_ptr<Entity> a,std::shared_ptr<Entity> b, const std::unordered_map<std::string, std::vector<std::string>>& validTargets);
    void            checkAndAppyGravity();
    bool            checkEntitySupport(std::shared_ptr<Entity> entity);

    void            playerAttacks();
    void            adjustPlayerPosition();
    void            checkPlayerState();
    void	        registerActions();
    void            spawnPlayer(sf::Vector2f pos);
    void            spawnBox(sf::Vector2f pos, sf::Vector2f size, size_t level);
    void            spawnEnemy(sf::Vector2f pos);
    void            spawnFlowerEnemy(sf::Vector2f pos, sf::Vector2f size);
    void            spawnTile(sf::Vector2f pos, sf::Vector2f size);
    // draw static objects
    void            SpawnStaticObject(std::string name, float x);
    void            SpawnDoor(std::string name, float x, int type = 0, int requiredScore = 0, size_t id = 0);
    void            sSpawnDoorObjects();
    void            spawnWoodenBoxes();
    void            spawnFlowerEnemies();
    void            spawnTiles();
    void            SpawnInBetweenDoorsTile() ;
    void            DrawTiles() ;

    void            sSpawnStaticObjects();
    void            updateCamera();
    void            updateDoors(sf::Time);
    void            updateHits(sf::Time dt);
    void            attackCollisions();
    //void            updateEntity(Entity& entity, sf::Time dt);

 

    void            resetPlayer();
    void            resetAllEntities();

    void            updateScore();

    void            init(const std::string& path);
    void            loadLevel(const std::string& path);
    sf::FloatRect   getViewBounds();

    sf::FloatRect   calculateBoundingBox(std::shared_ptr<Entity> e, BBType t);
    void            drawHitbox(std::shared_ptr<Entity> e);
    void            drawAttackBox(std::shared_ptr<Entity> e);
    void            drawGround(GroundCoord);
    void            renderGamePlayBackgroundView();
    void            renderHealthIndicators();
    void            renderWasted();
    void            renderGameComplete();
    void            renderLevelComplete();

    //Helpers
    sf::Vector2f    furthestDoorLeft();
    sf::Vector2f    closestDoorLeft();
    bool            isFurthestExitDoorRight(std::shared_ptr<Entity> entity);
    void            respawnPlayer();

    // Key
    void            spawnKey();
    void            setPermanentKeyWhereDeadBox(std::shared_ptr<Entity> entity);
    void            renderTrophyKey();
    void            updateKeyLocation(sf::Time);
    void            renderKeyIndicator();

    // enemy ninja movement
    void            enemyMovement(sf::Time dt);
    void            updateEnemyMovement(sf::Time dt);
    void            handleEnemyState(std::shared_ptr<Entity> enemy, const sf::Vector2f& playerPos);
    bool            shouldProcessEnemy(std::shared_ptr<Entity> enemy);
    bool            shouldAttack(std::shared_ptr<Entity> enemy, float xDistance);
    bool            shouldChase(std::shared_ptr<Entity> enemy, float xDistance);
    void            setEnemyAttackState(std::shared_ptr<Entity> enemy);
    void            setEnemyChaseState(std::shared_ptr<Entity> enemy, float xDistance);
    void            setEnemyIdle(std::shared_ptr<Entity> enemy);
    bool            canMoveForward(std::shared_ptr<Entity> enemy);

    void            spawnNinjaEnemies();



public:

    Scene_BulletNinja(GameEngine* gameEngine, const std::string& levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;

    //In Game Menu
    void            leftStripMenu(sf::Vector2f size);
    void            rightStripMenu(sf::Vector2f size);
    void            scoreCard(sf::Vector2f pos, sf::Vector2f size);
    void            InstructionsCard(sf::Vector2f pos, sf::Vector2f size);

    // respawn
    void            updateRespawns(sf::Time dt);
    bool            isRespawnAreaClear(std::shared_ptr<Entity> e);
    void            resetLevel();

};



#endif //BREAKOUT_SCENE_BREAKOUT_H