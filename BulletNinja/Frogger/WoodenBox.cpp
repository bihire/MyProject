
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

void Scene_BulletNinja::spawnBox(sf::Vector2f pos, sf::Vector2f size, size_t level) {
    auto m_box = _entityManager.addEntity("box");
    m_box->addComponent<CBoundingBox>(size);
    m_box->addComponent<CTransform>(pos);
    m_box->addComponent<CAnimation>(Assets::getInstance().getAnimation("woodenBox"));
    m_box->addComponent<CScore>(1, 20);
    m_box->addComponent<CRespawn>(5);
    m_box->addComponent<CLevel>(level);
}

void Scene_BulletNinja::spawnWoodenBoxes() {
    struct BoxArea {
        float left, right;  // X-axis
        float top, bottom;  // Y-axi
    };
    std::vector<BoxArea> placedBoxes;

    while (!_spawnWoodenBoxPoints.empty()) {
        const auto& box = _spawnWoodenBoxPoints.top();
        const float halfWidth = box.width / 2.0f;
        const float halfHeight = box.height / 2.0f;

        // current box bounds
        const float left = box.x - halfWidth;
        const float right = box.x + halfWidth;
        float top = m_ground.pos.y - box.height;
        const float bottom = top + box.height;

        for (const auto& placed : placedBoxes) {
            const bool xOverlap = right > placed.left && left < placed.right;
            const bool yOverlap = bottom > placed.top && top < placed.bottom;

            if (xOverlap && yOverlap) {
                top = std::min(top, placed.top - box.height);
            }
        }

        spawnBox(
            sf::Vector2f(box.x, top + halfHeight),
            sf::Vector2f(box.width, box.height),
            box.level - 1
        );


        placedBoxes.push_back({ left, right, top, top + box.height });

        _spawnWoodenBoxPoints.pop();
    }
}