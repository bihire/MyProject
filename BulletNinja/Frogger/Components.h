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
    sf::Vector2f	originalPos	{ 0.f, 0.f };
    sf::Vector2f	originalvel	{ 0.f, 0.f };
    sf::Vector2f	prevPos		{ 0.f, 0.f };
    sf::Vector2f	vel			{ 0.f, 0.f };
    sf::Vector2f	scale		{ 1.f, 1.f };

    float           angVel{ 0 };
    float	        angle{ 0.f };

    CTransform() = default;
    CTransform(const sf::Vector2f& p) : pos(p), originalPos(p) {}
    CTransform(const sf::Vector2f& p, const sf::Vector2f& v)
            : pos(p), prevPos(p),  vel(v), originalPos(p), originalvel(v){}
    CTransform(const sf::Vector2f& p, const sf::Vector2f& v, const sf::Vector2f& s)
        : pos(p), prevPos(p), vel(v), scale(s), originalPos(p), originalvel(v){}



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
    int _hp{ 10 };
    int _DefautHp;
    int _originallives{ 1 };
    int _originalHp{ 10 };

    std::unordered_set<std::shared_ptr<std::atomic<uint64_t>>> hitTracker;         // To do: clean this on animation chang add listeners on Animation

    CScore() = default;
    CScore(int lives) : _lives(lives), _DefautHp(_hp), _originallives(lives){};
    CScore(int lives, int hp) : _lives(lives), _hp(hp), _DefautHp(hp), _originalHp(hp) {};

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
    void setAnimation() {
        auto oldFrameChange = animation.onFrameChange;
        
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


struct ExtendedInfo : public Component {
    sf::Sprite image;
    std::vector<sf::Text> infoTexts;
    sf::RectangleShape background;
    size_t curIndex{ 0 };

    void centerInRightHalf(const sf::RenderWindow& window) {
        sf::Vector2f windowSize(window.getSize());
        float rightHalfStart = windowSize.x / 2.f;

        // Position background
        background.setSize(sf::Vector2f(windowSize.x / 2.f, windowSize.y));
        background.setPosition(rightHalfStart, 0);
        background.setFillColor(sf::Color(255, 255, 255));

        // center image horizontally in right half
        if (image.getTexture()) {
            image.setPosition(
                rightHalfStart + (windowSize.x / 2.f ) / 2.f,
                100.f // Top padding
            );
        }

        // text below image
        float currentY = image.getGlobalBounds().top + image.getGlobalBounds().height + 100.f;
        for (auto& text : infoTexts) {
            text.setPosition(rightHalfStart + 200.f, currentY); //pdding
            currentY += text.getGlobalBounds().height + 20.f;
        }
    }

    void setSprite(const sf::Sprite& sprite) {
        image = sprite;
    }

    
};

struct CLevel : public Component {
    size_t level{ 0 };

    CLevel() = default;

    CLevel(size_t l)
        : level(l) {}
};

struct CEnemyAI : public Component {
    enum Behavior {
        Chase = 1,
        JumpOverObstacles = 1 << 1,
        AttackWhenClose = 1 << 2
    };
    unsigned int behaviors{ Chase | AttackWhenClose };
    float jumpCooldown{ 0.f };
    float visionRange{ 500.f };
    float lastPlayerX = 0.f;
    bool wasFacingRight = true;

    CEnemyAI() = default;
    CEnemyAI(unsigned int b) : behaviors(b) {}
};

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
        isAttacking = 1 << 10,
        isHit = 1 << 11,
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
    size_t doorID;

    State state;
    Type type;
    int minScoreToOpen; 
    bool leftSideUsed;

    CDoorState(Type doorType = Exit, int minScore = 0, size_t door = 0)
        : state(Closed), type(doorType), minScoreToOpen(minScore), leftSideUsed(false), doorID(door) {}
};



struct CMenuItem : public Component
{
    bool selected = false;
    float animationProgress = 0.0f;
    sf::RectangleShape background;
    std::string menuString;
    float menuTargetWidth = 300.f;

    CMenuItem() : menuString("")
    {
        background.setFillColor(sf::Color(45, 45, 48, 180));
        background.setSize(sf::Vector2f(0, 0));
    }

    CMenuItem(const std::string& text, float height, float _menuTargetWidth) : menuTargetWidth(_menuTargetWidth), menuString(text)
    {
        background.setFillColor(sf::Color(45, 45, 48, 180));
        background.setSize(sf::Vector2f(0, height));
        background.setOutlineThickness(1.f);
        background.setOutlineColor(sf::Color(80, 80, 85));
    }

    void update(float dtSeconds, bool isSelected)
    {
        // Only update if selection state changed
        if (isSelected != selected)
        {
            selected = isSelected;
        }

        float targetWidth = selected ? menuTargetWidth : 0.f;
        float animationSpeed = 5.0f * dtSeconds;

        if (selected)
        {
            animationProgress = std::min(1.0f, animationProgress + animationSpeed);
        }
        else
        {
            animationProgress = std::max(0.0f, animationProgress - animationSpeed);
        }

        float easedProgress = selected ?
            easeOutCubic(animationProgress) :
            easeInCubic(animationProgress);

        float width = targetWidth * (0.33f + 0.67f * easedProgress);
        background.setSize(sf::Vector2f(width, background.getSize().y));
    }

private:
    static float easeOutCubic(float t) { return 1.f - pow(1.f - t, 3.f); }
    static float easeInCubic(float t) { return t * t * t; }
};

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

class NullFont : public sf::Font {
public:
    NullFont() {
        // Create minimal in-memory font
        // (This is a simplified example)
    }
};

struct CLevelComplete : public Component {
    bool isActive = false;
    sf::Time duration = sf::seconds(5.f);
    sf::Time elapsed = sf::Time::Zero;
    std::function<void()> onCompleteCallback;
    float progress = 0.f;

    void update(const sf::Time& delta) {
        if (!isActive) return;

        elapsed += delta;
        progress = elapsed / duration;

        if (elapsed >= duration) {
            isActive = false;
            if (onCompleteCallback) onCompleteCallback();
        }
    }

    void activate() {
        isActive = true;
        elapsed = sf::Time::Zero;
        progress = 0.f;
    }

    void onEnd(std::function<void()> callback = nullptr) {

        onCompleteCallback = callback;
    }
};

struct CGameComplete : public Component {
    bool isActive = false;
    sf::Time duration = sf::seconds(8.f); // Longer for final completion
    sf::Time elapsed = sf::Time::Zero;
    std::function<void()> onCompleteCallback;
    float progress = 0.f;

    void update(const sf::Time& delta) {
        if (!isActive) return;

        elapsed += delta;
        progress = elapsed / duration;

        if (elapsed >= duration) {
            isActive = false;
            if (onCompleteCallback) onCompleteCallback();
        }
    }

    void activate() {
        isActive = true;
        elapsed = sf::Time::Zero;
        progress = 0.f;
    }
};


struct CWasted : public Component {
    bool isWasted = false;
    sf::Time duration = sf::seconds(10.f); // Total wasted screen duration
    sf::Time elapsed = sf::Time::Zero;
    std::function<void()> onCompleteCallback;

    // Container
    sf::Font m_font;
    sf::RectangleShape container;
    sf::Vector2f containerSize{ 700.f, 400.f };
    sf::Color containerColor{ 45, 45, 48, 200 }; // Semi-transparent

    // Progress Bar
    sf::RectangleShape progressBarBg;
    sf::RectangleShape progressBar;
    sf::Vector2f progressBarSize{ 600.f, 30.f };
    sf::Color progressBarColor{ 255, 255, 255 }; // Red

    // Text
    sf::Text wastedText;
    float textScale = 0.f; // For animation

    
    CWasted() {
        init();
    }

    void init() {
        // Container setup
        container.setSize(containerSize);
        container.setFillColor(containerColor);
        container.setOrigin(containerSize.x / 2, containerSize.y / 2);

        // Progress bar
        progressBarBg.setSize(progressBarSize);
        progressBarBg.setFillColor(sf::Color(80, 80, 80));
        progressBarBg.setOrigin(progressBarSize.x / 2, 0);

        progressBar.setSize(sf::Vector2f(0, progressBarSize.y));
        progressBar.setFillColor(progressBarColor);
        progressBar.setOrigin(progressBarSize.x / 2, 0);

        // Text setup
        //wastedText.setFont(m_font);
        wastedText.setString("WASTED");
        wastedText.setCharacterSize(120);
        wastedText.setFillColor(sf::Color::White);
        wastedText.setOutlineColor(sf::Color::Red);
        wastedText.setOutlineThickness(3.f);
    }



    void update(sf::Time dt) {
        if (!isWasted) return;

        elapsed += dt;
        float progress = elapsed / duration;

        // Update progress bar width
        progressBar.setSize({
            600.f * progress,
            progressBar.getSize().y
            });

        // Animate text
        textScale = std::min(1.0f, progress * 2.f);
        wastedText.setScale(textScale, textScale);

        if (elapsed >= duration) {
            reset();
            if (onCompleteCallback) {
                onCompleteCallback();
            }
        }
    }

    void setPosition(const sf::Vector2f& pos) {
        container.setPosition(pos);
        progressBarBg.setPosition(pos.x, pos.y - containerSize.y / 2 + 50);
        progressBar.setPosition(progressBarBg.getPosition());
        wastedText.setPosition(
            pos.x - wastedText.getLocalBounds().width / 2 * textScale,
            pos.y - wastedText.getLocalBounds().height / 2 * textScale + 100
        );
    }
    void reset() {
        isWasted = false;
        elapsed = sf::Time::Zero;
        /*progressBar.setSize({ 0.f, progressBarSize.y });
        textScale = 0.f;*/
    }

    void onEnd(std::function<void()> callback = nullptr) {
        
        onCompleteCallback = callback;
    }

    

    
};



struct CRespawn : public Component {
    sf::Time respawnDelay{ sf::Time::Zero };
    sf::Time elapsedTime{ sf::Time::Zero };  

    CRespawn() = default;

    CRespawn(float delay)
        : respawnDelay(sf::seconds(delay)) {}

    bool isReadyToRespawn() const {
        return elapsedTime >= respawnDelay;
    }

    void update(sf::Time dt) {
        //std::cout << "Respawn timer: " << elapsedTime.asSeconds() << std::endl;
        if (!isReadyToRespawn()) {
            elapsedTime += dt;
        }
    }
};

struct CKeyState : public Component {
    bool isActive = false;
    bool isPermanent = false;
    bool isCollected = false;
    bool isRespawn = false;
    size_t keyID{ 0 };
    sf::Time respawnTimer;
    sf::Time elapsedTime;

    CKeyState() = default;
    CKeyState(float respawnSeconds)
        : respawnTimer(sf::seconds(respawnSeconds)),
        elapsedTime(sf::Time::Zero) {}

    void update(sf::Time dt) {
        if (!isRespawn && !isCollected) {
            elapsedTime += dt;
            if (elapsedTime >= respawnTimer) {
                isRespawn = true;
                elapsedTime = sf::Time::Zero;
            }
        }
    }

    void reset() {
        isActive = false;
        isPermanent = false;
        isCollected = false;
        isRespawn = false;
        elapsedTime = sf::Time::Zero;
          
    }


};

struct CHealthBarIndicator : public Component {
    sf::Vector2f fullSize{ 100.f, 20.f }; 
    sf::Vector2f position{ 0.f, 0.f };
    sf::Color fillColor{ 255, 0, 0 };      // Red
    sf::Color bgColor{ 45, 45, 48 };       // Dark gray
    sf::Color outlineColor{ 255, 255, 255 }; // White
    float outlineThickness{ 2.f };

    sf::RectangleShape background;
    sf::RectangleShape healthFill;

    CHealthBarIndicator() { init(); }

    CHealthBarIndicator(const sf::Vector2f& size) : fullSize(size) {
        init();
    }

    void init() {
        background.setSize(fullSize);
        background.setFillColor(bgColor);
        background.setOutlineThickness(outlineThickness);
        background.setOutlineColor(outlineColor);

        healthFill.setSize(fullSize);
        healthFill.setFillColor(fillColor);

        updatePosition();
    }

    void updatePosition() {
        background.setPosition(position);
        healthFill.setPosition(position);
    }

    void updateHealth(float currentHP, float maxHP) {
        float ratio = std::clamp(currentHP / maxHP, 0.f, 1.f);

        healthFill.setSize(sf::Vector2f(fullSize.x * ratio, fullSize.y));

        if (ratio < 0.25f) {
            healthFill.setFillColor(sf::Color(255, 0, 0));  
        }
        else if (ratio < 0.6f) {
            healthFill.setFillColor(sf::Color(255, 165, 0)); 
        }
        else {
            healthFill.setFillColor(sf::Color(0, 255, 0));  
        }
    }
};
struct CHitEffect : public Component {
    sf::Time duration;
    sf::Time elapsed;
    bool isActive;

    // Blinking pattern 
    std::vector<sf::Color> blinkPattern{
        sf::Color::White,
        sf::Color::Black,
        sf::Color::White,
        sf::Color::White 
    };

    CHitEffect(sf::Time duration = sf::seconds(0.5f))
        : duration(duration), elapsed(sf::Time::Zero), isActive(false) {}

    void start() {
        isActive = true;
        elapsed = sf::Time::Zero;
    }

    void start(CState& state) {
        start();
        state.set(CState::isHit);
    }

    void update(sf::Time deltaTime, sf::Sprite& sprite) {
        if (!isActive) return;

        std::cout << "Blinking..." << std::endl;
        elapsed += deltaTime;
        float progress = elapsed / duration;

        if (progress >= 1.0f) {
            isActive = false;
            sprite.setColor(sf::Color::White); // reset
            return;
        }

        // calculate cur phase  (0-1-2-3)
        size_t phase = static_cast<size_t>(progress * (blinkPattern.size() - 1));
        float phaseProgress = (progress * (blinkPattern.size() - 1)) - phase;

        // Interpolate between current and next color
        sf::Color current = blinkPattern[phase];
        sf::Color next = blinkPattern[std::min(phase + 1, blinkPattern.size() - 1)];

        // Preserve original alpha
        sf::Uint8 alpha = sprite.getColor().a;
        sf::Color newColor(
            static_cast<sf::Uint8>(current.r + (next.r - current.r) * phaseProgress),
            static_cast<sf::Uint8>(current.g + (next.g - current.g) * phaseProgress),
            static_cast<sf::Uint8>(current.b + (next.b - current.b) * phaseProgress),
            alpha
        );

        sprite.setColor(newColor);
    }

    void update(sf::Time deltaTime, sf::Sprite& sprite, CState& state) {
        update(deltaTime, sprite);
        if (!isActive) {
            state.unSet(CState::isHit);
        }
    }

    bool isRunning() const { return isActive; }
};



#endif //BREAKOUT_COMPONENTS_H
