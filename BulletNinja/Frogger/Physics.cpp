#include "Physics.h"
#include <cmath>
#include "Scene_BulletNinja.h"

sf::Vector2f Physics::getOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    sf::Vector2f overlap(0.f, 0.f);
    if (!a->hasComponent<CBoundingBox>() or !b->hasComponent<CBoundingBox>())
        return overlap;

    auto atx = a->getComponent<CTransform>();
    auto abb = a->getComponent<CBoundingBox>();
    auto btx = b->getComponent<CTransform>();
    auto bbb = b->getComponent<CBoundingBox>();


    if (abb.has && bbb.has)
    {
        float dx = std::abs(atx.pos.x - btx.pos.x);
        float dy = std::abs(atx.pos.y - btx.pos.y);
        overlap = sf::Vector2f(abb.halfSize.x + bbb.halfSize.x - dx, abb.halfSize.y + bbb.halfSize.y - dy);
    }
    return overlap;
}

sf::Vector2f Physics::getPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    sf::Vector2f overlap(0.f, 0.f);
    if (!a->hasComponent<CBoundingBox>() or !b->hasComponent<CBoundingBox>())
        return overlap;

    auto atx = a->getComponent<CTransform>();
    auto abb = a->getComponent<CBoundingBox>();
    auto btx = b->getComponent<CTransform>();
    auto bbb = b->getComponent<CBoundingBox>();

    if (abb.has && bbb.has)
    {
        float dx = std::abs(atx.prevPos.x - btx.prevPos.x);
        float dy = std::abs(atx.prevPos.y - btx.prevPos.y);
        overlap = sf::Vector2f(abb.halfSize.x + bbb.halfSize.x - dx, abb.halfSize.y + bbb.halfSize.y - dy);
    }
    return overlap;
}

//bool Physics::isOnGround(const std::shared_ptr<Entity>& entity, const sf::Vector2f& groundPos, float groundHeight)
//{
//    auto& transform = entity->getComponent<CTransform>();
//    auto& bbox = entity->getComponent<CBoundingBox>();
//
//    sf::FloatRect entityBB;
//    entityBB.left = transform.pos.x - bbox.halfSize.x;
//    entityBB.top = transform.pos.y - bbox.halfSize.y;
//    entityBB.width = bbox.halfSize.x * 2.f;
//    entityBB.height = bbox.halfSize.y * 2.f;
//
//    sf::FloatRect groundBB(groundPos.x, groundPos.y, 10000.f, groundHeight); // long flat ground
//
//    return entityBB.intersects(groundBB);
//}




void  Physics::applyGravity(std::shared_ptr<Entity> entity, float gravity, float maxFallSpeed)
{
    if (!entity->hasComponent<CTransform>())
        return;

    auto& transform = entity->getComponent<CTransform>();
    transform.vel.y += gravity;

    if (transform.vel.y > maxFallSpeed)
        transform.vel.y = maxFallSpeed;
}


bool Physics::isCollidingWithGround(const std::shared_ptr<Entity>& entity, const sf::Vector2f& candidatePos, const sf::Vector2f& groundPos, float groundHeight)
{
    if (!entity->hasComponent<CBoundingBox>() || !entity->hasComponent<CTransform>())
        return false;

    auto& bbox = entity->getComponent<CBoundingBox>();
    sf::FloatRect candidateBB;
    candidateBB.left = candidatePos.x - bbox.halfSize.x;
    candidateBB.top = candidatePos.y - bbox.halfSize.y;
    candidateBB.width = bbox.halfSize.x * 2.f;
    candidateBB.height = bbox.halfSize.y * 2.f;

    sf::FloatRect groundBB(groundPos.x, groundPos.y, 10000.f, groundHeight);

    return candidateBB.intersects(groundBB);
}
