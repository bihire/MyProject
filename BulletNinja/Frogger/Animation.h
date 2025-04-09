#ifndef SFMLCLASS_ANIMATION_H
#define SFMLCLASS_ANIMATION_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>

class Animation {
public:
    std::string                 _name{ "none" };
    std::vector<sf::IntRect>    _frames;
    sf::Time                    _timePerFrame;
    size_t                      _currentFrame{ 0 };
    sf::Time                    _countDown{ sf::Time::Zero };
    bool                        _isRepeating{ true };
    bool                        _hasEnded{ false };
    sf::Sprite                  _sprite;
    std::vector<int>            _hitboxFrames;   // New: Hitbox frames
    std::vector<int>            _attackboxFrames; // New: Attackbox frames
    std::function<void()>       onFrameChange; // Broadcast animation restats

public:
    Animation() = default;
    Animation(const std::string& name, const sf::Texture& t,
        std::vector<sf::IntRect> frames, sf::Time tpf, bool repeats = true);

    void                    update(sf::Time dt);
    bool                    hasEnded() const;
    const std::string&      getName() const;
    sf::Sprite&             getSprite();
    sf::Vector2f            getBB() const;
    int                     getCurFrame() const;
    int                     getFramesSize() const;

    
    void                    setHitboxFrames(int start, int end);
    void                    setAttackboxFrames(int start, int end);


    
    const std::vector<int>& getHitboxFrames() const;
    const std::vector<int>& getAttackboxFrames() const;

    // Check if current frame has hitbox or attackbox
    bool                    hasHitbox() const;
    bool                    hasAttackbox() const;
};

#endif //SFMLCLASS_ANIMATION_H
