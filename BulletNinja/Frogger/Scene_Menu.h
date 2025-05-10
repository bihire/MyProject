#pragma once

#include "Scene.h"

struct MenuItem {
	std::string title;
	std::string icon;
	std::string key;
	std::vector<std::string> texts;

	MenuItem(std::string t, std::string k, std::string icon, std::vector<std::string> txts)
		: title(t), key(k), icon(icon), texts(txts) {}
};

enum MenuType{ InDonguin, OutDonguin, All};

struct PanelContent {
	std::string title;
	std::string imageName;
	std::vector<std::string> bodyTexts;
};

class Scene_Menu : public Scene
{
private:
	std::vector<MenuItem>	m_visibleMenus;
	std::unordered_map<MenuType, std::vector<MenuItem>>	m_menus;
	
	sf::Text					m_menuText;
	std::vector<std::string>	m_levelPaths;
	int							m_menuIndex{ 0 };
	float						m_menuTargetWidth{ 500.f };
	int							m_menuRightSideIndex{ 0 };
	size_t						CHAR_SIZE{ 54 };
	std::string					m_title;

	std::shared_ptr<Entity> m_currentInfoEntity{ nullptr };



	void init();
	void onEnd() override;
public:

	Scene_Menu(GameEngine* gameEngine);

	void update(sf::Time dt) override;

	void sRender() override;
	void sDoAction(const Command& action) override;

	// Helper functions
	void	initializeMenu();
	void    updateMenuList();
	void	drawBackground();
	void	initExtendedInfo();
	void	updateExtendedInfo();
	void	renderExtendedInfo();
	void	createInfoPanel(const std::string& imagePath, const std::vector<std::string>& texts);


};
