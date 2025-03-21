#include "Animation.h"
#include "Utilities.h"

Animation::Animation(const std::string& name,
    const sf::Texture& t,
    std::vector<sf::IntRect> frames,
    sf::Time tpf,
    bool repeats)
    : _name(name)
    , _frames(frames)
    , _timePerFrame(tpf)
    , _isRepeating(repeats)
    , _countDown(tpf)
    , _sprite(t, _frames[0])
{
    centerOrigin(_sprite);

    std::cout << name << " tpf: " << _timePerFrame.asMilliseconds() << "ms\n";
}

void Animation::update(sf::Time dt) {
    _countDown -= dt;
    if (_countDown < sf::Time::Zero) {
        _countDown = _timePerFrame;
        _currentFrame += 1;

        if (_currentFrame >= _frames.size() && !_isRepeating) {
            _hasEnded = true;
            return;  // on the last frame of non-repeating animation, leave it
        }
        else {
            _currentFrame = (_currentFrame % _frames.size());
        }
        _sprite.setTextureRect(_frames[_currentFrame]);
        centerOrigin(_sprite);
    }
}

bool Animation::hasEnded() const {
    return (_currentFrame >= _frames.size());
}

const std::string& Animation::getName() const {
    return _name;
}

sf::Sprite& Animation::getSprite() {
    return _sprite;
}

sf::Vector2f Animation::getBB() const {
    return static_cast<sf::Vector2f>(_frames[_currentFrame].getSize());
}

int Animation::getCurFrame() const {
    return _currentFrame;
}

int Animation::getFramesSize() const {
    return _frames.size();
}

void Animation::setHitboxFrames(int start, int end) {
    _hitboxFrames.clear();
    for (int i = start; i <= end; ++i) {
        _hitboxFrames.push_back(i);
    }
}

void Animation::setAttackboxFrames(int start, int end) {
    _attackboxFrames.clear();
    for (int i = start; i <= end; ++i) {
        _attackboxFrames.push_back(i);
    }
}

const std::vector<int>& Animation::getHitboxFrames() const {
    return _hitboxFrames;
}

const std::vector<int>& Animation::getAttackboxFrames() const {
    return _attackboxFrames;
}


bool Animation::hasHitbox() const { return std::find(_hitboxFrames.begin(), _hitboxFrames.end(), _currentFrame) != _hitboxFrames.end(); };
bool Animation::hasAttackbox() const { return std::find(_attackboxFrames.begin(), _attackboxFrames.end(), _currentFrame) != _attackboxFrames.end(); };