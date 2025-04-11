#include "Physics.h"
#include <cmath>
#include "Scene_BulletNinja.h"

sf::Vector2f Physics::getOverlap(const sf::FloatRect& a, const sf::FloatRect& b)
{
    float dx = std::min(a.left + a.width, b.left + b.width) - std::max(a.left, b.left);
    float dy = std::min(a.top + a.height, b.top + b.height) - std::max(a.top, b.top);

    if (dx < 0.f || dy < 0.f)
        return sf::Vector2f(0.f, 0.f);
    return sf::Vector2f(dx, dy);
}

sf::Vector2f Physics::getPreviousOverlap(const sf::FloatRect& a, const sf::Vector2f& aOffset,
    const sf::FloatRect& b, const sf::Vector2f& bOffset)
{
    sf::FloatRect aPrev = a;
    sf::FloatRect bPrev = b;

    aPrev.left -= aOffset.x;
    aPrev.top -= aOffset.y;

    bPrev.left -= bOffset.x;
    bPrev.top -= bOffset.y;

    float dx = std::min(aPrev.left + aPrev.width, bPrev.left + bPrev.width) -
        std::max(aPrev.left, bPrev.left);
    float dy = std::min(aPrev.top + aPrev.height, bPrev.top + bPrev.height) -
        std::max(aPrev.top, bPrev.top);

    if (dx < 0.f || dy < 0.f)
        return sf::Vector2f(0.f, 0.f);
    return sf::Vector2f(dx, dy);
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
