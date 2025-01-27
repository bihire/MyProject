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



class Scene_BulletNinja : public Scene {
private:
    sPtrEntt        m_player{ nullptr };
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;

    bool			m_drawTextures{ true };
    bool			m_drawAABB{ false };
    bool			m_drawGrid{ false };

    sf::Text        m_text;
    sf::Time        m_timer;
    float           m_maxHeight;
    int             m_score;
    int             m_lives;
    int             m_reachGoal;

    //systems
    void            sMovement(sf::Time dt);
    void            sCollisions();
    void            sUpdate(sf::Time dt);
    void            sAnimation(sf::Time dt);

    void	        onEnd() override;


    // helper functions
    void            playerMovement();
    void            adjustPlayerPosition();
    void            checkPlayerState();
    void	        registerActions();
    void            spawnPlayer(sf::Vector2f pos);

    void            spawnLane1();
    void            spawnLane2();
    void            spawnLane3();
    void            spawnLane4();
    void            spawnLane5();
    void            spawnLane6();
    void            spawnLane7();
    void            spawnLane8();
    void            spawnLane9();
    void            spawnLane10();
    void            spawnGoal();
    void            spawnLives();

    void            resetPlayer();
    void            killPlayer();

    void            updateScore();

    void            init(const std::string& path);
    void            loadLevel(const std::string& path);
    sf::FloatRect   getViewBounds();

public:

    Scene_BulletNinja(GameEngine* gameEngine, const std::string& levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;

};



#endif //BREAKOUT_SCENE_BREAKOUT_H