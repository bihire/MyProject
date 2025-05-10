
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



void Scene_BulletNinja::SpawnDoor(std::string name, float x, int type, int requiredScore, size_t id) {
    auto door = _entityManager.addEntity("door");


    auto& anim = door->addComponent<CAnimation>(Assets::getInstance().getAnimation(name));
    
    sf::Vector2f spawnPos{ m_worldView.getSize().x / 2.f, m_worldView.getSize().y / 2.f };

    door->addComponent<CTransform>(sf::Vector2f(x, m_ground.pos.y - 243.f), sf::Vector2f(0, 0), (type == 1) ? sf::Vector2f(-1, 1): sf::Vector2f(1, 1));


    auto& doorState = door->addComponent<CDoorState>();
    doorState.type = (type == 0) ? CDoorState::Entrance : CDoorState::Exit;
    doorState.minScoreToOpen = requiredScore;
    doorState.state = CDoorState::Closed;
    doorState.doorID = id;

    

    //set door animation frame to the last frame
    

    


    door->addComponent<CBoundingBox>(sf::Vector2f{ 20.f, 200.f });


}




void Scene_BulletNinja::SpawnInBetweenDoorsTile() {
    auto doors = _entityManager.getEntities("door");
    auto cover = _entityManager.addEntity("tile");

    if (doors.empty()) return;
    
    for (auto& door : doors) {
        auto& doorPos = door->getComponent<CTransform>().pos;
        cover->addComponent<CTransform>(doorPos);
        

    }



}

void Scene_BulletNinja::DrawTiles() {
    auto doors = _entityManager.getEntities("door");
    if (doors.size() < 3) return; // Need at least 3 doors to skip some

    // Sort doors by their x-position
    std::sort(doors.begin(), doors.end(), [](const auto& a, const auto& b) {
        return a->getComponent<CTransform>().pos.x < b->getComponent<CTransform>().pos.x;
        });



    // for 1rst
    auto& tile = doors[0];
    auto& tfmPos = tile->getComponent<CTransform>().pos;

    auto box = calculateBoundingBox(doors[0], Hitbox);
    auto bb = sf::Vector2f(tfmPos.x + box.width, box.top);

    sf::FloatRect BB(0, 0, bb.x, bb.y);

    sf::RectangleShape rectangle;

    rectangle.setSize(BB.getSize());
    rectangle.setPosition(sf::Vector2f(0, 0));
    rectangle.setFillColor(sf::Color(18, 18, 18, 255));


    _game->window().draw(rectangle);

    // for Last
    auto& tileLast = doors[doors.size() - 1];
    auto& tfmPosLast = tileLast->getComponent<CTransform>().pos;

    auto bbLast = sf::Vector2f(m_worldBounds.width - tfmPosLast.x, box.top);

    sf::FloatRect BBLast(tfmPosLast.x - box.width, 0, m_worldBounds.width - tfmPosLast.x + box.width, bbLast.y);

    sf::RectangleShape rectangleLast;

    rectangleLast.setSize(BBLast.getSize());
    rectangleLast.setPosition(BBLast.getPosition());
    rectangleLast.setFillColor(sf::Color(18, 18, 18, 255));


    _game->window().draw(rectangleLast);


    // Draw tiles between every other door pair (skip 2)
    for (size_t i = 1; i < doors.size() - 1; i += 2) {


        if (i + 1 >= doors.size()) break;

        auto& leftDoor = doors[i];
        auto& rightDoor = doors[i + 1];

        auto& leftPos = leftDoor->getComponent<CTransform>().pos;
        auto& rightPos = rightDoor->getComponent<CTransform>().pos;


        float tileLeft = leftPos.x + leftDoor->getComponent<CBoundingBox>().size.x / 2;
        float tileRight = rightPos.x - rightDoor->getComponent<CBoundingBox>().size.x / 2;
        float tileWidth = tileRight - tileLeft;
        float tileHeight = box.top;


        sf::RectangleShape tile(sf::Vector2f(tileWidth + (box.width * 2), tileHeight));
        tile.setPosition(tileLeft - box.width, 0);
        tile.setFillColor(sf::Color(18, 18, 18, 255));

        _game->window().draw(tile);


    }
}

void Scene_BulletNinja::sSpawnDoorObjects()
{
    while (!_spawnDoorPoints.empty() && _spawnDoorPoints.top().x > 0) {
        SpawnDoor(_spawnDoorPoints.top().name, _spawnDoorPoints.top().x, _spawnDoorPoints.top().type, _spawnDoorPoints.top().requiredScore, _spawnDoorPoints.top().id);
        _spawnDoorPoints.pop();
    }

    _entityManager.update();
    SpawnInBetweenDoorsTile();

}


void Scene_BulletNinja::updateDoors(sf::Time dt) {
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

        bool approachingFromLeft = (playerPos.x < doorTransform.pos.x);


        // Constraints for opening the exit door
        bool hasRequiredItems = false;
        if (m_player->hasComponent<CScore>()) {
            auto& playerScore = m_player->getComponent<CScore>();
			if (PlayerProgress::getInstance().getTotalScore() >= doorState.minScoreToOpen && PlayerProgress::getInstance().isKeysCollected()) {
				hasRequiredItems = true;
			}
        }

        // Only allow opening if coming from left side 
        bool shouldOpen = (distance <= activationDistance) &&
            approachingFromLeft &&
            (doorState.state == CDoorState::Closed);
        

        if (shouldOpen && doorState.type == CDoorState::Exit) {
            shouldOpen = hasRequiredItems; 
        }

        if (shouldOpen) {
            doorState.state = CDoorState::Open;
            doorAnim.playForward();
        } else if (distance > activationDistance && doorState.state == CDoorState::Open) {
            doorState.state = CDoorState::Closed;
            doorAnim.playBackward();

            bool status = doorState.type == CDoorState::Entrance ? true : false;
            if(doorState.type == CDoorState::Exit && approachingFromLeft) {
				status = true;
			}  

            if(doorState.type == CDoorState::Entrance && !approachingFromLeft) {
                PlayerProgress::getInstance().startLevel(dt);
			}
            
            

            if (doorState.type == CDoorState::Exit && !approachingFromLeft) {
                PlayerProgress::getInstance().endLevel(dt);
                if (!PlayerProgress::getInstance().isLastLevel()) {
                    if(m_player->hasComponent<CLevelComplete>()) {
						auto& levelComplete = m_player->getComponent<CLevelComplete>();
                        levelComplete.isActive = true;
					}
                    
                } else {
					if(m_player->hasComponent<CGameComplete>()) {
						auto& gameComplete = m_player->getComponent<CGameComplete>();
						gameComplete.isActive = true;
					}
				}
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



sf::Vector2f Scene_BulletNinja::closestDoorLeft()
{
    const sf::Vector2f INVALID_POSITION{ 0,0 };

    if (!m_player) return INVALID_POSITION;

    auto& playerX = m_player->getComponent<CTransform>().pos.x;
    //float playerX = ptfm.pos.x;
    sf::Vector2f closestPos = INVALID_POSITION;
        float closestX = std::numeric_limits<float>::lowest();

    for (const auto& door : _entityManager.getEntities("door"))
    {


        const auto& transform = door->getComponent<CTransform>();
        const float doorX = transform.pos.x;

        if (doorX < playerX && doorX > closestX)
        {
            closestX = doorX;
            closestPos = transform.pos;
        }
    }

    return closestPos;
}

sf::Vector2f Scene_BulletNinja::furthestDoorLeft()
{
    const sf::Vector2f INVALID_POSITION{ std::numeric_limits<float>::max(),
                                         std::numeric_limits<float>::max() };

    if (!m_player) return INVALID_POSITION;

    const float playerX = m_player->getComponent<CTransform>().pos.x;
    sf::Vector2f furthestPos = INVALID_POSITION;
    float furthestX = std::numeric_limits<float>::max();

    for (const auto& door : _entityManager.getEntities("door"))
    {
        if (!door->hasComponent<CDoorState>()) continue;

        const auto& doorState = door->getComponent<CDoorState>();
        if (doorState.type != CDoorState::Exit) continue;

        const auto& transform = door->getComponent<CTransform>();
        const float doorX = transform.pos.x;

        if (doorX < playerX && doorX < furthestX)
        {
            furthestX = doorX;
            furthestPos = transform.pos;
        }
    }

    return furthestPos;
}


bool Scene_BulletNinja::isFurthestExitDoorRight(std::shared_ptr<Entity> entity) {
    if (!entity->hasComponent<CDoorState>() ||
        entity->getComponent<CDoorState>().type != CDoorState::Exit) {
        return false;
    }

    const float entityX = entity->getComponent<CTransform>().pos.x;
    float furthestX = -std::numeric_limits<float>::max();

    for (const auto& door : _entityManager.getEntities("door")) {
        if (!door->hasComponent<CDoorState>() ||
            door->getComponent<CDoorState>().type != CDoorState::Exit) {
            continue;
        }

        const float doorX = door->getComponent<CTransform>().pos.x;
        furthestX = std::max(furthestX, doorX);
    }

    return (entityX == furthestX);
}