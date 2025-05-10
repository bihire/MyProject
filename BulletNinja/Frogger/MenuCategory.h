#pragma once
#include "Scene_Menu.h"
#include "Scene_BulletNinja.h"
#include <memory>

class MenuCategory
{
private:
    std::string m_name;
    std::vector<MenuItem> m_items;
    size_t m_selectedIndex{ 0 };

public:
    MenuCategory(std::string name) : m_name(name) {}

    void addItem(MenuItem item) {
        m_items.push_back(item);
    }

    const std::vector<MenuItem>& getItems() const {
        return m_items;
    }

    std::vector<MenuItem> getAvailableItems() const {
       /* std::vector<MenuItem> available;
        for (const auto& item : m_items) {
            if (item.isAvailable()) {
                available.push_back(item);
            }
        }
        return available;*/
    }
};

