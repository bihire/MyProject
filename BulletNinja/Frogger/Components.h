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
    std::function<void()> onAnimationEnd;

    CAnimation() : key(std::make_shared<std::atomic<uint64_t>>(0)) {}

    CAnimation(const Animation& a)
        : animation(a),
        key(std::make_shared<std::atomic<uint64_t>>(0)) {
        bindKey();
    }

    void bindKey() {
        // Store the existing onAnimationEnd
        auto existingCallback = onAnimationEnd;

        animation.onFrameChange = [this, existingCallback]() {
            
            std::cout << "Animation key changed: " << key << std::endl;
            std::cout << "animation name: " << animation.getName() << std::endl;
            
            if (existingCallback) existingCallback();
            if (onAnimationEnd) onAnimationEnd();
            };
    }

    CAnimation(const CAnimation& other)
        : animation(other.animation),
        key(other.key),
        onAnimationEnd(other.onAnimationEnd) {  // Copy the callback!
        bindKey();
    }

    uint64_t getCurrentKey() const {
        return key->load();
    }

    CAnimation& operator=(const CAnimation& other) {
        if (this != &other) {
            // Preserve our callback during assignment
            auto oldCallback = onAnimationEnd;

            animation = other.animation;
            key = other.key;
            onAnimationEnd = other.onAnimationEnd;

            bindKey();

            // Restore if we had different callback
            if (oldCallback && !other.onAnimationEnd) {
                onAnimationEnd = oldCallback;
            }
        }
        return *this;
    }

    
    void setAnimation(const Animation& newAnim) {
        auto oldFrameChange = animation.onFrameChange;
        animation = newAnim;
        key = std::make_unique<std::atomic<uint64_t>>(0);
        animation.onFrameChange = oldFrameChange;  // Preserve the callback
        animation.play();
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

//struct CState : public Component {
//    std::string state{"none"};
//
//    CState() = default;
//    CState(const std::string& s) : state(s){}
//
//};

struct CState : public Component {
    enum State {
        isGrounded = 1,
        isFacingLeft = 1 << 1,
        isRunning = 1 << 2,
        isAttackSword = 1 << 3,
        isAttackSpear = 1 << 4,
        isDead = 1 << 5,
        isJumping = 1 << 6,
        isFalling = 1 << 7,
        wasRunning = 1 << 8,      
        wasGrounded = 1 << 9,
        isAttacking = 1 << 10
    };
    unsigned int state{ 0 };

    CState() = default;
    CState(unsigned int s) : state(s) {}

    bool test(unsigned int x) const { return (state & x); }
    void set(unsigned int x) { state |= x; }
    void unSet(unsigned int x) { state &= ~x; }

    // check state changes
    bool becameGrounded() const { return test(isGrounded) && !test(wasGrounded); }
    bool becameAirborne() const { return !test(isGrounded) && test(wasGrounded); }
    bool startedRunning() const { return test(isRunning) && !test(wasRunning); }
    bool stoppedRunning() const { return !test(isRunning) && test(wasRunning); }

    // Call this at the end of each frame to update previous states
    void updatePreviousStates() {
        if (test(isRunning)) set(wasRunning);
        else unSet(wasRunning);

        if (test(isGrounded)) set(wasGrounded);
        else unSet(wasGrounded);
    }
};

struct CDoorState : public Component {
    enum State { Open, Closed, Opening, Closing };
    enum Type { Entrance, Exit };

    State state;
    Type type;
    int minScoreToOpen; 
    bool leftSideUsed;

    CDoorState(Type doorType = Exit, int minScore = 0)
        : state(Closed), type(doorType), minScoreToOpen(minScore), leftSideUsed(false) {}
};


//struct CInput : public Component
//{
//    enum dirs {
//        UP      = 1 << 0,
//        DOWN    = 1 << 1,
//        LEFT    = 1 << 2,
//        RIGHT   = 1 << 3
//        
//    };
//
//    enum attacks {
//        SWORD = 1 << 0,
//        SPEAR = 1 << 1
//    };
//
//    unsigned char dir{0};
//    unsigned char attack{0};
//
//    CInput() = default;
//};

struct CInput : public Component
{
    bool UP{ false };
    bool LEFT{ false };
    bool RIGHT{ false };
    bool DOWN{ false };

    bool SWORD{ false };
    bool SPEAR{ false };

    bool canJump{ true };

    CInput() = default;
};



#endif //BREAKOUT_COMPONENTS_H
