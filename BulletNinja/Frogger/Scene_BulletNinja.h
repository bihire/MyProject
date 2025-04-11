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


struct SpawnDoorPoint {
    std::string name;
    float x;
    int type;  // 0 = entrance, 1 = exit
    int requiredScore = 0;  // Only used for exit doors

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
    float       enemyChaseDistance{ 150.f };
    
};

struct GroundCoord {
    sf::Vector2f pos{ 0, 344.f };
    float width{ 5000.f };
    float height{ 10.f };
};

class Scene_BulletNinja : public Scene {
private:
    sPtrEntt        m_player{ nullptr };
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;
    LevelConfig     m_config;
    GroundCoord     m_ground;
    std::priority_queue<SpawnPoint> _spawnStaticPoints;
    std::priority_queue<SpawnDoorPoint> _spawnDoorPoints;

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

    void	        onEnd() override;


    // helper functions
    void            playerMovement(sf::Time dt);
    void            enemyMovement(sf::Time dt);
    void            updateEnemyMovement(std::shared_ptr<Entity> enemy, sf::Vector2f& nextPos, std::string& anim, const std::string& curAnim, CTransform& enemyTransform, const CTransform& playerTransform);

    void            playerCheckState();
    bool            hasObstacleBetween(std::shared_ptr<Entity> enemy, const sf::Vector2f& targetPos);
    void            resolveDoorCollision(std::shared_ptr<Entity> player, std::shared_ptr<Entity> door, sf::Vector2f overlap);

    // Collision
    bool            isCollidingWithWalls(std::shared_ptr<Entity> e, sf::Vector2f& nextPos);
    bool			isOnGroundOrPlatforms(const sf::FloatRect& entityBB, const std::vector<std::shared_ptr<Entity>>& platforms, const sf::Vector2f& groundPos, float groundHeight);

    void            playerAttacks();
    void            adjustPlayerPosition();
    void            checkPlayerState();
    void	        registerActions();
    void            spawnPlayer(sf::Vector2f pos);
    void            spawnBox(sf::Vector2f pos);
    void            spawnEnemy(sf::Vector2f pos);
    void            SpawnStaticObject(std::string name, float x);
    void            SpawnDoorObject(std::string name, float x, int type = 0, int requiredScore = 0);
    void            sSpawnStaticObjects();
    void            updateCamera();
    void            updateDoors();
    void            updatePlayerAnimation();
    void            attackCollisions();
    //void            updateEntity(Entity& entity, sf::Time dt);

 

    void            resetPlayer();

    void            updateScore();

    void            init(const std::string& path);
    void            loadLevel(const std::string& path);
    sf::FloatRect   getViewBounds();

    sf::FloatRect   calculateBoundingBox(std::shared_ptr<Entity> e, BBType t);
    void            drawHitbox(std::shared_ptr<Entity> e);
    void            drawAttackBox(std::shared_ptr<Entity> e);
    void            drawGround(GroundCoord);

public:

    Scene_BulletNinja(GameEngine* gameEngine, const std::string& levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;

    //void          drawAABB(std::shared_ptr<Entity> e);

};



#endif //BREAKOUT_SCENE_BREAKOUT_H