#include "Scene_Menu.h"
#include "Scene_BulletNinja.h"
#include <memory>
#include "PlayerProgress.h"

void Scene_Menu::onEnd()
{
	_game->window().close();
}


Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	
	//listener update menu
	PlayerProgress::getInstance().addStatusListener([this]() {
		std::cout << "Status changed\n";
		updateMenuList();
	});

	init();
	
}



void Scene_Menu::init()
{

	drawBackground();
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
	registerAction(sf::Keyboard::D, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::Space, "PLAY");
	registerAction(sf::Keyboard::Backspace, "PAUSE");

	m_title = "Bullet Ninja";

	initializeMenu();
	// update visible menus
	updateMenuList();



	m_levelPaths.push_back("../assets/level1.txt");
	/*m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level1.txt");*/

	m_menuText.setFont(Assets::getInstance().getFont("main"));

	
	m_menuText.setCharacterSize(CHAR_SIZE);

	
	for (size_t i = 0; i < m_visibleMenus.size(); ++i)
	{
		auto e = _entityManager.addEntity("menu-item");
		float itemHeight = CHAR_SIZE + 20;
		e->addComponent<CMenuItem>(m_visibleMenus[i].title, itemHeight, m_menuTargetWidth);
	}

	initExtendedInfo();


}

void Scene_Menu::updateMenuList() {

	std::cout << "Updating menu list\n";
	m_visibleMenus.clear();
	if (PlayerProgress::getInstance().getLevelStatus() == QuestState::InProgress) {
		m_visibleMenus.insert(m_visibleMenus.end(),
			m_menus[MenuType::InDonguin].begin(),
			m_menus[MenuType::InDonguin].end());
}
	else {
		m_visibleMenus.insert(m_visibleMenus.end(),
			m_menus[MenuType::OutDonguin].begin(),
			m_menus[MenuType::OutDonguin].end());
	}

		m_visibleMenus.insert(m_visibleMenus.end(),
		m_menus[MenuType::All].begin(),
		m_menus[MenuType::All].end());

}

void Scene_Menu::initExtendedInfo()
{
	auto panel = m_visibleMenus[m_menuIndex];
	createInfoPanel(panel.icon, panel.texts);
}

void Scene_Menu::createInfoPanel(const std::string& imagePath,
	const std::vector<std::string>& texts) {
	auto& assets = Assets::getInstance();
	auto& font = assets.getFont("main");



	auto e = _entityManager.addEntity("extended-info");
	auto& info = e->addComponent<ExtendedInfo>();

	
	info.setSprite(assets.getAnimation(imagePath)._sprite);
	
	if (m_visibleMenus[m_menuIndex].key == "play" || m_visibleMenus[m_menuIndex].key == "continue") {
		size_t currLevel = PlayerProgress::getInstance().getCurrentLevel();

		sf::Text t("This is the current level " + std::to_string(currLevel), font, 42);
		t.setFillColor(sf::Color::Black);
		e->getComponent<ExtendedInfo>().infoTexts.push_back(t);
	}

	for (const auto& text : texts) {
		sf::Text t(text, font, 24);
		t.setFillColor(sf::Color::Black);
		info.infoTexts.push_back(t);
	}

	info.centerInRightHalf(_game->window());
}

void Scene_Menu::updateExtendedInfo() {

	
	
	auto& e = _entityManager.getEntities("extended-info")[0];
	if(m_menuIndex == e->getComponent<ExtendedInfo>().curIndex) return;

	auto panel = m_visibleMenus[m_menuIndex];
	e->getComponent<ExtendedInfo>().setSprite(Assets::getInstance().getAnimation(panel.icon)._sprite);
	e->getComponent<ExtendedInfo>().curIndex = m_menuIndex;
	e->getComponent<ExtendedInfo>().infoTexts.clear();
	
	auto& assets = Assets::getInstance();
	auto& font = assets.getFont("main");

	if (m_visibleMenus[m_menuIndex].key == "play" || m_visibleMenus[m_menuIndex].key == "continue") {
		size_t currLevel = PlayerProgress::getInstance().getCurrentLevel();

		sf::Text t("Level " + std::to_string(currLevel), font, 24);
		t.setFillColor(sf::Color::Black);
		e->getComponent<ExtendedInfo>().infoTexts.push_back(t);
	}
	

	for (const auto& text : panel.texts) {
		sf::Text t(text, font, 24);
		t.setFillColor(sf::Color::Black);
		e->getComponent<ExtendedInfo>().infoTexts.push_back(t);
	}

	if (m_visibleMenus[m_menuIndex].key == "exit") {
		

		sf::Text t("PRESS SPACE OR ESC KEY TO EXIT", font, 42);
		t.setFillColor(sf::Color::Black);
		e->getComponent<ExtendedInfo>().infoTexts.push_back(t);
	}

	if (m_visibleMenus[m_menuIndex].key == "play") {


		sf::Text t("PRESS SPACE KEY TO START", font, 42);
		t.setFillColor(sf::Color::Black);
		e->getComponent<ExtendedInfo>().infoTexts.push_back(t);
	}

	e->getComponent<ExtendedInfo>().centerInRightHalf(_game->window());
	

	
}



void Scene_Menu::update(sf::Time dt)
{
	_entityManager.update();

	


	float dtSeconds = dt.asSeconds();
	size_t itemIndex = 0;

	for (auto e : _entityManager.getEntities("menu-item"))
	{
		if (e->getComponent<CMenuItem>().has)
		{
			auto& item = e->getComponent<CMenuItem>();
			bool isSelected = (itemIndex == m_menuIndex);
			item.update(dtSeconds, isSelected);
			itemIndex++;
		}
	}
	
	updateExtendedInfo();
	


}

void Scene_Menu::drawBackground()
{
	auto e = _entityManager.addEntity("glassy-bkg");

	auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture("GlassyBackground")).sprite;

	float windowHeight = static_cast<float>(_game->window().getSize().y);

	sf::Vector2u textureSize = sprite.getTexture()->getSize();
	float scaledHeight = textureSize.y;

	float verticalPosition = (windowHeight - scaledHeight) /2.f;

	sprite.setOrigin(0.f, 0.f);
	sprite.setPosition(0, verticalPosition);
}

void Scene_Menu::renderExtendedInfo() {
	auto entities = _entityManager.getEntities("extended-info");
	if (entities.empty()) {
		return;
	}
	m_currentInfoEntity = entities[0];


	auto& info = m_currentInfoEntity->getComponent<ExtendedInfo>();
	_game->window().draw(info.background);
	if (info.image.getTexture()) {
		_game->window().draw(info.image);
	}
	for (auto& text : info.infoTexts) {
		_game->window().draw(text);
	}
	
}


void Scene_Menu::sRender()
{
	//static const sf::Color selectedColor(163, 30, 29);
	static const sf::Color selectedColor(205, 60, 52);
	static const sf::Color normalColor(255, 255, 255);
	// View setup
	sf::View view = _game->window().getView();
	view.setCenter(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f);
	_game->window().setView(view);

	static const sf::Color backgroundColor(45, 45, 48);
	_game->window().clear(backgroundColor);

	
	for (auto e : _entityManager.getEntities("glassy-bkg")) {
		if (e->getComponent<CSprite>().has) {
			_game->window().draw(e->getComponent<CSprite>().sprite);
		}
	}

	// draw background box
	sf::Vector2f windowSize(_game->windowSize());
	float leftHalf = windowSize.x / 4.f;
	const float itemHeight = 96.f; 
	const float totalMenuHeight = m_visibleMenus.size() * itemHeight;


	float startY = (windowSize.y - totalMenuHeight) / 2.f; //start Y position

	// draw background box
	size_t itemIndex = 0;
	for (auto e : _entityManager.getEntities("menu-item"))
	{
		if (e->getComponent<CMenuItem>().has)
		{
			auto& item = e->getComponent<CMenuItem>();

			item.background.setPosition(leftHalf - (m_menuTargetWidth / 2.f),
				startY + (itemIndex * itemHeight));
			_game->window().draw(item.background);
			itemIndex++;
		}
	}

	

	// draw menu text
	for (size_t i = 0; i < m_visibleMenus.size(); ++i)
	{
		m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		m_menuText.setPosition(leftHalf - (m_menuTargetWidth / 2.f) + 16,
			startY + (i * itemHeight));
		m_menuText.setString(m_visibleMenus.at(i).title);
		_game->window().draw(m_menuText);
	}

	renderExtendedInfo();// draw extended info

}

void Scene_Menu::initializeMenu() {


	m_menus[MenuType::OutDonguin].emplace_back(MenuItem("Start Quest", "play",
		"play",
		{
			"",
			""
		}
	));

	m_menus[MenuType::InDonguin].emplace_back(MenuItem("Continue Playing", "continue",
		"pause",
		{
			"Contunue where you left off",
			"Lets go Champion..."
		}
	));

	m_menus[MenuType::InDonguin].emplace_back(MenuItem("Restart Quest", "restart",
		"play",
		{
			"Lets start again",
			"Lets go Champion..."
		}
	));

	//m_menus[MenuType::All].emplace_back(MenuItem("Quest Archive", "archive",
	//	"play",
	//	{
	//		"Lets start again",
	//		"Lets go Champion..."
	//	}
	//));

	m_menus[MenuType::All].emplace_back(MenuItem("The Samurai Story", "story",
		"ninja",
		{
			"A lone ninja walks the shadowed path,",
			"seeking ancient mystic keys.",
			"",
			"The Demon Gates awaits - its seal holds",
			"easy to get in, hard to get out;",
			"All filled with unspeakable darkness.",
			"",
			"But the land fights back. Fallen foes",
			"rise again with crimson eyes.",
			"",
			"Steel flashes under the blood moon.",
			"Every kill fuels their rebirth.",
			"",
			"Only speed and skill will prevail.",
			"The keys must be claimed... before",
			"the night claims the ninja first."
		}
	));

	m_menus[MenuType::All].emplace_back(MenuItem("Game Controls", "controls",
		"controller",
		{
			"GAME CONTROLS:",
			"",
			"|  S  | - Spear slash",
			"|  A  | - Sword slash",
			"|  UP | - Jump",
			"|LEFT | - Run left",
			"|RIGHT| - Run right"
		}
	));

	m_menus[MenuType::All].emplace_back(MenuItem("Exit Game", "exit",
		"controller",
		{
			"Are you sure you want to exit?",
			"All Your progress will be lost.",
			""
			
		}
	));
}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_visibleMenus.size() - 1) % m_visibleMenus.size();
		}
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_visibleMenus.size();
		}
		else if (action.name() == "PLAY")
		{
			auto& playerProgress = PlayerProgress::getInstance();
			if (m_visibleMenus[m_menuIndex].key == "play" || m_visibleMenus[m_menuIndex].key == "continue") {
				playerProgress.setPaused(false);
				_game->changeScene("PLAY", std::make_shared<Scene_BulletNinja>(_game, m_levelPaths[m_menuIndex]));
			} else if(m_visibleMenus[m_menuIndex].key == "restart") {
				m_menuIndex = 0;
				_game->changeScene("PLAY", nullptr, false);
				playerProgress.resetCurrentLevel();
				
				
			}
			else if (m_visibleMenus[m_menuIndex].key == "exit") {
				onEnd();
			}
				
			
			
			
			
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}