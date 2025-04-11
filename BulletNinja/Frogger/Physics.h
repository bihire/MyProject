#pragma once


#include "Entity.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>


namespace Physics
{
	sf::Vector2f	getOverlap(const sf::FloatRect& a, const sf::FloatRect& b);
	sf::Vector2f	getPreviousOverlap(const sf::FloatRect& a, const sf::Vector2f& aOffset,
		const sf::FloatRect& b, const sf::Vector2f& bOffset);

	//bool			isOnGround(const std::shared_ptr<Entity>& entity, const sf::Vector2f& groundPos, float groundHeight);

	void			applyGravity(std::shared_ptr<Entity> entity, float gravity, float maxFallSpeed);
	bool			isCollidingWithGround(const std::shared_ptr<Entity>& entity, const sf::Vector2f& candidatePos, const sf::Vector2f& groundPos, float groundHeight);
};

