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
	sf::Vector2f	getOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    sf::Vector2f	getPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);

	//bool			isOnGround(const std::shared_ptr<Entity>& entity, const sf::Vector2f& groundPos, float groundHeight);

	void			applyGravity(std::shared_ptr<Entity> entity, float gravity, float maxFallSpeed);
	bool			isCollidingWithGround(const std::shared_ptr<Entity>& entity, const sf::Vector2f& candidatePos, const sf::Vector2f& groundPos, float groundHeight);
};

