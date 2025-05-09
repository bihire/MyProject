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

//void Animation::update(sf::Time dt) {
//    _countDown -= dt;
//    if (_countDown < sf::Time::Zero) {
//        _countDown = _timePerFrame;
//        _currentFrame += 1;
//
//        if (_currentFrame >= _frames.size() && !_isRepeating) {
//            _hasEnded = true;
//            return;  // on the last frame of non-repeating animation, leave it
//        } else {
//            
//            _currentFrame = (_currentFrame % _frames.size());
//            if (onFrameChange) {
//                onFrameChange();  // Notify listener
//            }
//
//        }
//        _sprite.setTextureRect(_frames[_currentFrame]);
//        centerOrigin(_sprite);
//    }
//}

void Animation::update(sf::Time dt) {
    if (_hasEnded && !_isRepeating) return;

    _countDown -= dt;
    if (_countDown < sf::Time::Zero) {
        _countDown = _timePerFrame;

        if (_isReversed) {
            if (_currentFrame == 0) {
                _hasEnded = true;
                if (_isRepeating) {
                    _currentFrame = _frames.size() - 1;
                    _hasEnded = false;
                }
            }
            else {
                _currentFrame--;
            }
        }
        else {
            if (_currentFrame >= _frames.size() - 1) {
                _hasEnded = true;
                if (_isRepeating) {
                    _currentFrame = 0;
                    _hasEnded = false;
                }
            }
            else {
                _currentFrame++;
            }
        }

        _sprite.setTextureRect(_frames[_currentFrame]);
        centerOrigin(_sprite);
        if (hasEnded() && onFrameChange) {
            onFrameChange();
        }
    }
}

void Animation::setRepeating(bool repeat) {
	_isRepeating = repeat;
}

bool Animation::hasEnded() const {
    return _currentFrame >= _frames.size() -1 ;
    //if (_isRepeating) return false;
    //if (_isReversed) return (_currentFrame == 0 && _countDown < sf::Time::Zero);
    //return (_currentFrame >= _frames.size() - 1 && _countDown < sf::Time::Zero);
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
    if(start == end) {
		_attackboxFrames.clear();
		return;
	}
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

// reverse methods
void Animation::setReversed(bool reversed) {
    _isReversed = reversed;
}

bool Animation::isReversed() const {
    return _isReversed;
}

void Animation::toggleReversed() {
    _isReversed = !_isReversed;
}

void Animation::playForward() {
    _isReversed = false;
    _hasEnded = false;
    _currentFrame = 0;
    _countDown = _timePerFrame;
}

void Animation::play() {
    _hasEnded = false;
    _countDown = _timePerFrame;
}

bool Animation::isPlaying() const {
    return !_hasEnded;
}

void Animation::playBackward() {
    _isReversed = true;
    _hasEnded = false;
    _currentFrame = _frames.size() - 1;
    _countDown = _timePerFrame;
}


bool Animation::hasHitbox() const { return std::find(_hitboxFrames.begin(), _hitboxFrames.end(), _currentFrame) != _hitboxFrames.end(); };
bool Animation::hasAttackbox() const { return std::find(_attackboxFrames.begin(), _attackboxFrames.end(), _currentFrame) != _attackboxFrames.end(); };