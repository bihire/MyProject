#include "MenuCard.h"

MenuCard::MenuCard(float marginX, float marginY, float padding)
    : m_marginX(marginX), m_marginY(marginY), m_padding(padding), m_width(200.f) {
    // Default things
    m_background.setFillColor(sf::Color(30, 30, 30, 200));
    m_background.setOutlineThickness(1.f);
    m_background.setOutlineColor(sf::Color(100, 100, 100));
    m_background.setPosition(m_marginX, m_marginY);
}

void MenuCard::addItem(const sf::Text& text) {
    m_items.push_back(text);
    updateLayout();
}

void MenuCard::setPosition(float x, float y)
{
	m_background.setPosition(x, y);
	m_marginX = x;
	m_marginY = y;
	updateLayout();
}

void MenuCard::setWidth(float width)
{
	m_width = width;
	updateLayout();
}

void MenuCard::updateLayout() {
    // Calculate total content height
    m_contentHeight = m_padding; // Start with top padding

    for (auto& item : m_items) {
        m_contentHeight += item.getGlobalBounds().height + 10.f; // spacing
    }

    m_contentHeight += m_padding * 2; // Add bottom padding

    // Update background size
    m_background.setSize(sf::Vector2f(m_width, m_contentHeight));

    // Position items
    float currentY = m_marginY + m_padding;
    for (auto& item : m_items) {
        item.setPosition(m_marginX + m_padding, currentY);
        currentY += item.getGlobalBounds().height + 10.f;
    }
}

void MenuCard::draw(sf::RenderWindow& window) const {
    window.draw(m_background);
    for (const auto& item : m_items) {
        window.draw(item);
    }
}

void MenuCard::setBackgroundColor(const sf::Color& color)
{
	m_background.setFillColor(color);
}

void MenuCard::setOutline(float thickness, const sf::Color& color)
{
	m_background.setOutlineThickness(thickness);
	m_background.setOutlineColor(color);
}

sf::Vector2f MenuCard::getSize() const
{
	return m_background.getSize();
    
}

sf::Vector2f MenuCard::getPosition() const
{
	return m_background.getPosition();
    
}


