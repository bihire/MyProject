
//
// Created by David Burchill on 2023-09-27.
//

#include <fstream>
#include <iostream>

#include "Scene_BulletNinja.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include <random>
#include "MenuCard.h"
#include "PlayerProgress.h"

void Scene_BulletNinja::leftStripMenu(sf::Vector2f size) {
	sf::RectangleShape leftStrip(size);
	leftStrip.setPosition(0, 0);
	leftStrip.setFillColor(sf::Color(255, 255, 255, 0));
	scoreCard(sf::Vector2f(0,0), size);
	_game->window().draw(leftStrip);
}

void Scene_BulletNinja::rightStripMenu(sf::Vector2f size) {
	sf::Vector2u windowSize = _game->window().getSize();
	float stripWidth = (windowSize.x - m_sceneDimensions.m_gameplayWidth) / 2.0f;

	sf::RectangleShape rightStrip(size);
	rightStrip.setPosition(windowSize.x - stripWidth, 0);
	rightStrip.setFillColor(sf::Color(255, 255, 255, 0));

	//InstructionsCard(sf::Vector2f(windowSize.x - stripWidth, 0), size);
	_game->window().draw(rightStrip);
}

void Scene_BulletNinja::scoreCard(sf::Vector2f parentPos, sf::Vector2f parentSize) {

	auto cardColor =!PlayerProgress::getInstance().isKeysCollected()? sf::Color(255, 255, 255) : sf::Color(153, 229, 80);
	float margin {20.f };
	MenuCard statsCard(margin, margin, 15.f);
	statsCard.setWidth(parentSize.x - (margin*2.f));
	statsCard.setPosition(parentPos.x + margin, parentPos.y + margin);
	statsCard.setBackgroundColor(cardColor);

	auto& score = m_player->getComponent<CScore>();


	//content
	sf::Text scoreText;
	scoreText.setFont(Assets::getInstance().getFont("Arcade")); 
	scoreText.setCharacterSize(32);
	scoreText.setFillColor(sf::Color::Black);
	scoreText.setString("score  " + std::to_string(PlayerProgress::getInstance().getTotalScore()));
	statsCard.addItem(scoreText);

	
	scoreText.setString("Current HP  " + std::to_string(score._hp));
	statsCard.addItem(scoreText);


	scoreText.setString("Lives " + std::to_string(score._lives));
	statsCard.addItem(scoreText);

	if (m_nearestExitDoor) {
		auto& doorTransform = m_nearestExitDoor->getComponent<CTransform>();
		auto& doorState = m_nearestExitDoor->getComponent<CDoorState>();

		
		scoreText.setString("Required " + std::to_string(doorState.minScoreToOpen) + " points");
		statsCard.addItem(scoreText);
	}

	// draw
	statsCard.draw(_game->window());
}

void Scene_BulletNinja::InstructionsCard(sf::Vector2f parentPos, sf::Vector2f parentSize) {
	float margin {20.f };
	MenuCard instructionsCard(margin, margin, 15.f);
	instructionsCard.setWidth(parentSize.x - (margin*2.f));
	instructionsCard.setPosition(parentPos.x + margin, parentPos.y + margin);
	instructionsCard.setBackgroundColor(sf::Color(255, 255, 255));

	//content
	//Game title
	sf::Text instructionsText;
	instructionsText.setFont(Assets::getInstance().getFont("Arcade"));
	instructionsText.setCharacterSize(42);
	instructionsText.setFillColor(sf::Color::Black);
	instructionsText.setString("BULLET NINJA");
	instructionsCard.addItem(instructionsText);
	
	// Game level
	instructionsText.setFont(Assets::getInstance().getFont("main"));
	instructionsText.setCharacterSize(24);
	instructionsText.setString("#0");
	instructionsCard.addItem(instructionsText);

	// Game instructions

	instructionsText.setString("Secure your weapon");
	instructionsCard.addItem(instructionsText);
	// Game instructions

	instructionsText.setString("Avoid getting Killed on the way.");
	instructionsCard.addItem(instructionsText);

	// draw
	instructionsCard.draw(_game->window());

	//========================================


	// new card
	MenuCard levelInfoCard(margin, margin, 15.f);
	levelInfoCard.setWidth(parentSize.x - (margin * 2.f));
	levelInfoCard.setPosition(instructionsCard.getPosition().x, (instructionsCard.getPosition().y + instructionsCard.getSize().y + margin));
	levelInfoCard.setBackgroundColor(sf::Color(255, 255, 255));

	//content
	//Game title
	sf::Text levelInfoCardText;
	levelInfoCardText.setFont(Assets::getInstance().getFont("main"));
	levelInfoCardText.setCharacterSize(42);
	levelInfoCardText.setFillColor(sf::Color::Black);
	levelInfoCardText.setString("00 : 05");
	levelInfoCard.addItem(levelInfoCardText);

	//draw
	levelInfoCard.draw(_game->window());
	
}