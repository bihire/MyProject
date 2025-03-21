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

//struct CHitbox : public Component {
//    std::unordered_map<std::string, std::unordered_map<int, sf::FloatRect>> hitboxes;
//    sf::FloatRect currentHitbox;
//
//    CHitbox() = default;
//
//    // Set hitbox for a specific frame
//    void setHitboxForFrame(const std::string& animationName, int frame, const sf::FloatRect& box) {
//        hitboxes[animationName][frame] = box;
//    }
//
//    // Set hitbox for a range of frames
//    void setHitboxForFrame(const std::string& animationName, int startFrame, int endFrame, const sf::FloatRect& box) {
//        for (int i = startFrame; i <= endFrame; ++i) {
//            hitboxes[animationName][i] = box;
//        }
//    }
//
//    // Update the hitbox when the animation frame changes
//    void update(const std::string& animationName, int currentFrame) {
//        if (hitboxes.count(animationName) && hitboxes[animationName].count(currentFrame)) {
//            currentHitbox = hitboxes[animationName][currentFrame];
//        }
//        else {
//            currentHitbox = sf::FloatRect();
//        }
//    }
//};
//
//
//
//struct CAttackBox : public Component {
//    std::unordered_map<std::string, std::unordered_map<int, sf::FloatRect>> attackBoxes;
//    sf::FloatRect currentAttackBox;
//
//    CAttackBox() = default;
//
//    void setAttackBoxForFrame(const std::string& animationName, int frame, const sf::FloatRect& box) {
//        attackBoxes[animationName][frame] = box;
//    }
//
//    void setAttackBoxForFrame(const std::string& animationName, int startFrame, int endFrame, const sf::FloatRect& box) {
//        for (int i = startFrame; i <= endFrame; ++i) {
//            attackBoxes[animationName][i] = box;
//        }
//    }
//
//    void update(const std::string& animationName, int currentFrame) {
//        if (attackBoxes.count(animationName) && attackBoxes[animationName].count(currentFrame)) {
//            currentAttackBox = attackBoxes[animationName][currentFrame];
//        }
//        else {
//            currentAttackBox = sf::FloatRect();
//        }
//    }
//};

//struct CAnimation : public Component {
//    Animation animation;
//    std::string currentAnimName;
//    int lastFrame{ -1 };
//
//    CAnimation() = default;
//    CAnimation(const Animation& a) : animation(a), currentAnimName(a.getName()) {}
//
//    void update(sf::Time dt, CHitbox& hitboxComponent, CAttackBox& attackBoxComponent) {
//        animation.update(dt);
//
//        int currentFrame = animation.getCurFrame();
//        if (currentFrame != lastFrame || currentAnimName != animation.getName()) {
//            hitboxComponent.update(animation.getName(), currentFrame);
//            attackBoxComponent.update(animation.getName(), currentFrame);
//            lastFrame = currentFrame;
//            currentAnimName = animation.getName();
//        }
//    }
//};

struct CAnimation : public Component {
    Animation animation;

    CAnimation() = default;
    CAnimation(const Animation& a) : animation(a) {}
};




struct CBoundingBox : public Component
{
    sf::Vector2f size{0.f, 0.f};
    sf::Vector2f halfSize{ 0.f, 0.f };

    CBoundingBox() = default;
    CBoundingBox(const sf::Vector2f& s) : size(s), halfSize(0.5f * s)
    {}
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
