#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
class MenuCard
{
private:
    sf::RectangleShape                      m_background;
    std::vector<sf::Text>                   m_items;
    float                                   m_marginX, m_marginY;
    float                                   m_padding;
    float                                   m_width;
    float                                   m_contentHeight;

public:
    MenuCard(float marginX = 20.0f, float marginY = 20.0f, float padding = 15.0f);

    void                                    addItem(const sf::Text& text);
    void                                    setPosition(float x, float y);
    void                                    setWidth(float width);
    void                                    updateLayout();
    void                                    draw(sf::RenderWindow& window) const;

    // Style setters
    void                                    setBackgroundColor(const sf::Color& color);
    void                                    setOutline(float thickness, const sf::Color& color);

    sf::Vector2f                            getSize() const;
    sf::Vector2f                            getPosition() const;


};

