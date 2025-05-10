
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

void Scene_BulletNinja::spawnKey() {
    
        auto key = _entityManager.addEntity("trophyKey");
        key->addComponent<CTransform>();
        key->addComponent<CAnimation>(Assets::getInstance().getAnimation("trophyKey"));
        key->addComponent<CBoundingBox>(sf::Vector2f{ 64.f, 64.f });
        key->addComponent<CKeyState>(10.f);
        key->addComponent<CScore>();
    
}

void Scene_BulletNinja::updateKeyLocation(sf::Time dt) {
    auto& keys = _entityManager.getEntities("trophyKey");
    auto& boxesC = _entityManager.getEntities("box");

    // Early returns remain the same
    if (keys.empty() || boxesC.empty() ||
        keys[0]->getComponent<CKeyState>().isPermanent ||
        keys[0]->getComponent<CKeyState>().isCollected) {
        return;
    }

    if (!keys[0]->getComponent<CKeyState>().isRespawn) {
        keys[0]->getComponent<CKeyState>().update(dt);
        return;
    }

    // Filter  by level
    std::vector<std::shared_ptr<Entity>> validBoxes;
    for (auto& e : boxesC) {
        if (e->isActive() && e->hasComponent<CLevel>() &&
            e->getComponent<CLevel>().level == PlayerProgress::getInstance().getCurrentLevel()) {
            validBoxes.push_back(e);
            std::cout << "Valid box found: " << e->getTag() << std::endl;
            std::cout << "key state: " << keys[0]->isActive() << std::endl;
        }
    }

    if (validBoxes.empty()) return;

    keys[0]->getComponent<CKeyState>().isRespawn = false;

    if (!keys[0]->getComponent<CKeyState>().isActive) {
        keys[0]->getComponent<CKeyState>().isActive = true;
    }

    
    auto& box = validBoxes[rand() % validBoxes.size()];
    auto boxBB = calculateBoundingBox(box, BBType::Hitbox);

    auto& keyTransform = keys[0]->getComponent<CTransform>();
    auto keyBB = calculateBoundingBox(keys[0], BBType::Hitbox);
    keyTransform.pos = boxBB.getPosition();
    keyTransform.pos.y += (boxBB.height - keyBB.height) * 2.5f;
    keyTransform.pos.x += (boxBB.width - keyBB.width) * 2.5f;
    keys[0]->getComponent<CKeyState>().keyID = box->getId();

    keys[0]->getComponent<CKeyState>().update(dt);
}

void Scene_BulletNinja::renderTrophyKey() {
    auto& keys = _entityManager.getEntities("trophyKey");
    for (auto& key: keys) {
        if(!key->isActive()) continue;
        if (key->getComponent<CKeyState>().isActive &&
            !key->getComponent<CKeyState>().isCollected) {
            auto& anim = key->getComponent<CAnimation>().animation;
            anim.getSprite().setPosition(key->getComponent<CTransform>().pos);
            _game->window().draw(anim.getSprite());
        }
    }
}

void Scene_BulletNinja::renderKeyIndicator() {
    
    auto& keys = _entityManager.getEntities("trophyKey");
    if(keys.empty()) return;
    for(auto& key: keys) {
        if (!key->isActive()) continue;
        for(auto& box: _entityManager.getEntities("box")) {
			if(!box->isActive()) continue;
            if(box->getId() == key->getComponent<CKeyState>().keyID) {
                
				auto anim = Assets::getInstance().getAnimation("cross");
                anim.getSprite().setPosition(key->getComponent<CTransform>().pos);
				_game->window().draw(anim._sprite);
			}
		}
	}

    
}

void Scene_BulletNinja::setPermanentKeyWhereDeadBox(std::shared_ptr<Entity> entity) {
    if (entity->getTag() != "box") return;

    auto& keys = _entityManager.getEntities("trophyKey");
    if (keys.empty()) return;

    for (auto& key : keys) {
        if (!key->isActive()) continue;


        if (key->getComponent<CKeyState>().keyID == entity->getId()) {
            key->getComponent<CKeyState>().isPermanent = true;
        }
    }
}