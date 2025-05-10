
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

sf::FloatRect Scene_BulletNinja::calculateBoundingBox(std::shared_ptr<Entity> e, BBType type) {
    auto box = e->getComponent<CBoundingBox>();
    auto transform = e->getComponent<CTransform>();
    auto size = box.size;
    auto position = transform.pos;

    auto& tag = e->getTag();
    auto& state = e->getComponent<CState>();

    if (tag == "box" || tag == "tile" || tag == "trophyKey") {
        position = sf::Vector2f{ position.x, position.y - box.size.y *1.5f };
    }

    
    else if (tag == "flowerEnemy") {
        if (state.test(CState::isAttacking)) {
            if (type == Hitbox) {
                
                position = sf::Vector2f{ position.x, (position.y - box.size.y/1.f) };
                if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
            }
            else if (type == Attackbox) {
                size = sf::Vector2f{ box.size.x * 2.f, box.size.y + box.size.x/2 };
                position = sf::Vector2f{ position.x, position.y - box.size.y * 1.9f };
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
                position = sf::Vector2f{ position.x, position.y - box.size.y };
            }
        }
    }
    else if (tag == "door") {
        size = sf::Vector2f{ box.size.x * 2, box.size.y * 2.41f };
        position = sf::Vector2f{ position.x, position.y - 718.f };
    }

    else if (tag == "ground") {
        size = sf::Vector2f{ m_ground.width * 3, m_ground.height };
        position = m_ground.pos;
    }
    else if (tag == "player" || tag == "enemy") {
        if (state.test(CState::isAttackSpear)) {
            if (type == Hitbox) {
                size = sf::Vector2f{ box.size.x - 1.f, box.size.y };
                position = sf::Vector2f{ position.x - (size.x * 0.5f), position.y  };
                if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
            }
            else if (type == Attackbox) {
                size = sf::Vector2f{ box.size.x * 6.4f, box.size.y + box.size.x };
                position = sf::Vector2f{ position.x, position.y - box.size.y - 10.f };
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
                position = sf::Vector2f{ position.x, position.y - box.size.y - 10.f };
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
            size = sf::Vector2f{ box.size.x, box.size.y };
            position = sf::Vector2f{ position.x - (size.x * 0.5f), position.y };
            if (transform.scale.x == -std::abs(transform.scale.x)) position.x += size.x;
        }
    }

    sf::Vector2f centeredPosition = sf::Vector2f{ position.x - (size.x * 0.5f), position.y + size.y };
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

bool Scene_BulletNinja::hasObstacleBetween(std::shared_ptr<Entity> enemy, const sf::Vector2f& playerPos) {
    sf::Vector2f enemyPos = enemy->getComponent<CTransform>().pos;

    // get direction (left or right)
    bool playerIsRight = playerPos.x > enemyPos.x;

    // all boxes and doors
    auto obstacles = _entityManager.getEntities();
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(), [](auto& e) {
            std::string tag = e->getTag();
            return !(tag == "door" || tag == "box");
            }),
        obstacles.end()
    );

    // loop thru all obstacles
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

void Scene_BulletNinja::renderHealthIndicators() {
    for (auto& e : _entityManager.getEntities()) {
        if (!e->isActive()) continue;
        if (!e->hasComponent<CHealthBarIndicator>() || !e->hasComponent<CScore>()) {
            continue;
        }

        auto& healthBar = e->getComponent<CHealthBarIndicator>();
        auto& health = e->getComponent<CScore>();
        auto& transform = e->getComponent<CTransform>();

        float yOffset = -40.f;
        if (e->hasComponent<CBoundingBox>()) {
            auto& box = e->getComponent<CBoundingBox>();
            yOffset = -box.size.y - healthBar.fullSize.y - 5.f;
        }

        healthBar.position = {
            transform.pos.x -25.f - healthBar.fullSize.x / 2.f,
            transform.pos.y                    
        };
        healthBar.updatePosition();

        healthBar.updateHealth(health._hp, health._DefautHp);

        _game->window().draw(healthBar.background);
        _game->window().draw(healthBar.healthFill);
    }
}


void Scene_BulletNinja::updateRespawns(sf::Time dt) {
    for (auto& e : _entityManager.getEntities()) {
        
        if (!e->isActive() && e->hasComponent<CRespawn>()) {
            auto& respawn = e->getComponent<CRespawn>();
            respawn.update(dt);

            if (respawn.isReadyToRespawn() && isRespawnAreaClear(e)) {
                // Respawn logic
                auto& transform = e->getComponent<CTransform>();
                auto& score = e->getComponent<CScore>();
                transform.pos = transform.originalPos;
                score._lives = score._originallives;
                score._hp = score._originalHp;
                e->respawn();
                respawn.elapsedTime = sf::Time::Zero; // Reset timer
            }
        }

        // spaswn flower tree
        if (e->getTag() == "flowerEnemy" && e->hasComponent<CRespawn>() && e->getComponent<CState>().test(CState::isDead)) {
			auto& respawn = e->getComponent<CRespawn>();
			respawn.update(dt);

			if (respawn.isReadyToRespawn()) {
				// Respawn logic
				auto& transform = e->getComponent<CTransform>();
                auto& flowerEnemyAnim = e->getComponent<CAnimation>();
                e->getComponent<CState>().unSet(CState::isDead);
                transform.vel = transform.originalvel;
                flowerEnemyAnim.setAnimation(Assets::getInstance().getAnimation("flowerattack"));
				//e->respawn();
				respawn.elapsedTime = sf::Time::Zero; // Reset timer
			}
		}
    }
}

bool Scene_BulletNinja::isRespawnAreaClear(std::shared_ptr<Entity> e) {
    sf::FloatRect respawnArea = calculateBoundingBox(e, BBType::Hitbox);
    const std::set<std::string> blockingTags = {
        "enemy", "box", "zombie", "flowerEnemy", "player", "trophyKey"
    };

    for (auto& other : _entityManager.getEntities()) {
        if (other->isActive() && blockingTags.count(other->getTag()) > 0) {
            if (respawnArea.intersects(calculateBoundingBox(other, BBType::Hitbox))) {
                return false; // Collision detected
            }
        }
    }
    return true; // Safe to respawn
}

void Scene_BulletNinja::resetAllEntities() {
    for (auto& e : _entityManager.getEntities()) {
        if(e->getTag() == "player") continue;
        auto& tfm = e->getComponent<CTransform>();
        tfm.pos = tfm.originalPos;
        e->respawn();
        if (e->hasComponent<CState>()) {
            auto& state = e->getComponent<CState>();
            state.unSet(CState::isDead);
            state.unSet(CState::isAttacking);
            state.unSet(CState::isAttackSword);
            state.unSet(CState::isAttackSpear);
            state.unSet(CState::isGrounded);
        }
    }
}



void Scene_BulletNinja::renderWasted() {
    auto& wasted = m_player->getComponent<CWasted>();
    if (!wasted.isWasted) return;

    // Get view center once
    const auto& view = _game->window().getView();
    const sf::Vector2f center = view.getCenter();

    // Draw semi-transparent background overlay
    sf::RectangleShape overlay;
    overlay.setSize(view.getSize());
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    _game->window().draw(overlay);

    // Draw container box
    sf::RectangleShape container;
    container.setSize({ 700.f, 400.f });
    container.setFillColor({ 45, 45, 48, 200 });
    container.setOrigin(350.f, 200.f);
    container.setPosition(center);
    _game->window().draw(container);

    // Draw "WASTED"
    // Animation came from chatgpt
    sf::Text wastedText;
    wastedText.setFont(Assets::getInstance().getFont("main")); 
    wastedText.setString("WASTED");
    wastedText.setCharacterSize(120);
    wastedText.setFillColor(sf::Color::White);
    wastedText.setOutlineColor(sf::Color::Red);
    wastedText.setOutlineThickness(3.f);

    // Center text and animate
    wastedText.setOrigin(
        wastedText.getLocalBounds().width / 2,
        wastedText.getLocalBounds().height / 2
    );
    float textScale = std::min(wasted.elapsed.asSeconds() * 3.f, 1.f);
    wastedText.setScale(textScale, textScale);
    wastedText.setPosition(center.x, center.y - 80);
    _game->window().draw(wastedText);

    // progress bar background
    sf::RectangleShape progressBg;
    progressBg.setSize({ 600.f, 30.f });
    progressBg.setFillColor({ 80, 80, 80 });
    progressBg.setOrigin(300.f, 0);
    progressBg.setPosition(center.x, center.y + 100);
    _game->window().draw(progressBg);

    
    float progress = wasted.elapsed / wasted.duration;
    sf::RectangleShape progressBar;
    progressBar.setSize({ 600.f * progress, 30.f });
    progressBar.setFillColor(sf::Color(220, 50, 50));
    progressBar.setOrigin(300.f, 0);
    progressBar.setPosition(center.x, center.y + 100);
    _game->window().draw(progressBar);

    // hint text
    sf::Text hintText;
    hintText.setFont(Assets::getInstance().getFont("main"));
    hintText.setString("Wait for revival...");
    hintText.setCharacterSize(30);
    hintText.setFillColor(sf::Color(200, 200, 200));
    hintText.setOrigin(
        hintText.getLocalBounds().width / 2,
        hintText.getLocalBounds().height / 2
    );
    hintText.setPosition(center.x, center.y + 150);
    _game->window().draw(hintText);
}

void Scene_BulletNinja::renderLevelComplete() {
    auto& complete = m_player->getComponent<CLevelComplete>();
    if (!complete.isActive) return;

    const auto& view = _game->window().getView();
    const sf::Vector2f center = view.getCenter();

    sf::RectangleShape overlay;
    overlay.setSize(view.getSize());
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    _game->window().draw(overlay);

    sf::RectangleShape container;
    container.setSize({ 700.f, 400.f });
    container.setFillColor(sf::Color(50, 80, 50, 200));
    container.setOrigin(350.f, 200.f);
    container.setPosition(center);
    _game->window().draw(container);

    sf::Text completeText;
    completeText.setFont(Assets::getInstance().getFont("main"));
    completeText.setString("LEVEL "+ std::to_string(PlayerProgress::getInstance().getCurrentLevel() -1) + " COMPLETE!");
    completeText.setCharacterSize(40);
    completeText.setFillColor(sf::Color::Green);
    completeText.setOutlineColor(sf::Color::White);
    completeText.setOutlineThickness(2.f);
    completeText.setOrigin(completeText.getLocalBounds().width / 2, completeText.getLocalBounds().height / 2);
    completeText.setPosition(center.x, center.y - 80);
    _game->window().draw(completeText);

    float progressWidth = 600.f * complete.progress;
    sf::RectangleShape progressBar;
    progressBar.setSize({ progressWidth, 20.f });
    progressBar.setFillColor(sf::Color(100, 255, 100));
    progressBar.setOrigin(300.f, 0);
    progressBar.setPosition(center.x, center.y + 100);
    _game->window().draw(progressBar);
}

void Scene_BulletNinja::renderGameComplete() {
    auto& complete = m_player->getComponent<CGameComplete>();
    if (!complete.isActive) return;

    const auto& view = _game->window().getView();
    const sf::Vector2f center = view.getCenter();

    sf::RectangleShape overlay;
    overlay.setSize(view.getSize());
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    _game->window().draw(overlay);

    sf::RectangleShape container;
    container.setSize({ 800.f, 500.f });
    container.setFillColor(sf::Color(70, 50, 20, 220));
    container.setOrigin(400.f, 250.f);
    container.setPosition(center);
    _game->window().draw(container);

    sf::Text completeText;
    completeText.setFont(Assets::getInstance().getFont("main"));
    completeText.setString("GAME COMPLETED!");
    completeText.setCharacterSize(100);
    completeText.setFillColor(sf::Color(255, 215, 0));
    completeText.setOutlineColor(sf::Color(150, 100, 0));
    completeText.setOutlineThickness(4.f);
    completeText.setOrigin(completeText.getLocalBounds().width / 2, completeText.getLocalBounds().height / 2);
    completeText.setPosition(center.x, center.y - 100);
    _game->window().draw(completeText);

    sf::Text subText;
    subText.setFont(Assets::getInstance().getFont("main"));
    subText.setString("You have mastered the way of the Shadow Blade");
    subText.setCharacterSize(30);
    subText.setFillColor(sf::Color::White);
    subText.setOrigin(subText.getLocalBounds().width / 2, subText.getLocalBounds().height / 2);
    subText.setPosition(center.x, center.y);
    _game->window().draw(subText);

    float progressWidth = 600.f * complete.progress;
    sf::RectangleShape progressBar;
    progressBar.setSize({ progressWidth, 25.f });
    progressBar.setFillColor(sf::Color(255, 215, 0));
    progressBar.setOrigin(300.f, 0);
    progressBar.setPosition(center.x, center.y + 150);
    _game->window().draw(progressBar);
}

