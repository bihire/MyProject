//
// Created by David Burchill on 2023-09-27.
//

#ifndef BREAKOUT_COMPONENTS_H
#define BREAKOUT_COMPONENTS_H


#include <memory>
#include <SFML/Graphics.hpp>
#include "Utilities.h"
#include "Animation.h"
#include <bitset>
#include <unordered_map>
#include <unordered_set>


struct Component
{
    bool		has{ false };
    Component() = default;
};


struct CSprite : public Component {
    sf::Sprite sprite;

    CSprite() = default;

    CSprite(const sf::Texture &t)
            : sprite(t) {
        centerOrigin(sprite);
    }

    CSprite(const sf::Texture &t, sf::IntRect r)
            : sprite(t, r) {
        centerOrigin(sprite);
    }
};


struct CTransform : public Component
{

    sf::Transformable  tfm;
    sf::Vector2f	pos			{ 0.f, 0.f };
    sf::Vector2f	prevPos		{ 0.f, 0.f };
    sf::Vector2f	vel			{ 0.f, 0.f };
    sf::Vector2f	scale		{ 1.f, 1.f };

    float           angVel{ 0 };
    float	        angle{ 0.f };

    CTransform() = default;
    CTransform(const sf::Vector2f& p) : pos(p)  {}
    CTransform(const sf::Vector2f& p, const sf::Vector2f& v)
            : pos(p), prevPos(p),  vel(v){}
    CTransform(const sf::Vector2f& p, const sf::Vector2f& v, const sf::Vector2f& s)
        : pos(p), prevPos(p), vel(v), scale(s) {}

};

struct CJump : public Component {
    bool isJumping = false;
    float jumpTime = 0.f;
    float maxJumpTime = 0.3f;

    CJump() = default;
    CJump(float maxTime) : maxJumpTime(maxTime) {}
};


struct CScore : public Component {
    int _score{ 0 };
    int _lives{ 1 };
    int _hp{ 100 };
    int _DefautHp;

    std::unordered_set<std::shared_ptr<std::atomic<uint64_t>>> hitTracker;         // To do: clean this on animation chang add listeners on Animation

    CScore() = default;
    CScore(int lives) : _lives(lives), _DefautHp(_hp) {};
    CScore(int lives, int hp) : _lives(lives), _hp(hp), _DefautHp(hp) {};

    bool canTakeHit(std::shared_ptr<std::atomic<uint64_t>> attackerAnimKey) {
        return hitTracker.find(attackerAnimKey) == hitTracker.end();
    }

    void registerHit(std::shared_ptr<std::atomic<uint64_t>> attackerAnimKey) {
        hitTracker.insert(attackerAnimKey);
    }

    void clearOldHits() {
        hitTracker.clear();
    }
};

struct CHitTracker : public Component {
    uint64_t lastHitAnimationKey{ 0 };

    CHitTracker() = default;
};

struct CAnimation : public Component {
    Animation animation;
    std::shared_ptr<std::atomic<uint64_t>> key;

    CAnimation() = default;

    CAnimation(const Animation& a)
        : animation(a), key(std::make_shared<std::atomic<uint64_t>>(0)) {
        bindKey();
    }

    void bindKey() {
        animation.onFrameChange = [this]() {
            // Increment the atomic key when the frame changes
            key->fetch_add(1, std::memory_order_relaxed);
            };


    }

    CAnimation(const CAnimation& other)
        : animation(other.animation), key(other.key) {
        bindKey();
    }

    CAnimation& operator=(const CAnimation& other) {
        if (this != &other) {
            animation = other.animation;
            key = other.key;
            bindKey();
        }
        return *this;
    }
};





struct CBoundingBox : public Component
{
    sf::Vector2f size{0.f, 0.f};
    sf::Vector2f halfSize{ 0.f, 0.f };

    CBoundingBox() = default;
    CBoundingBox(const sf::Vector2f& s) : size(s), halfSize(0.5f * s)
    {}
};

struct CPhysics : public Component {
    bool isJumping = false;
    sf::Vector2f velocity = { 0.f, 0.f };
};

struct CState : public Component {
    std::string state{"none"};

    CState() = default;
    CState(const std::string& s) : state(s){}

};


struct CInput : public Component
{
    enum dirs {
        UP      = 1 << 0,
        DOWN    = 1 << 1,
        LEFT    = 1 << 2,
        RIGHT   = 1 << 3
        
    };

    enum attacks {
        SWORD = 1 << 0,
        SPEAR = 1 << 1
    };

    unsigned char dir{0};
    unsigned char attack{0};

    CInput() = default;
};


#endif //BREAKOUT_COMPONENTS_H
