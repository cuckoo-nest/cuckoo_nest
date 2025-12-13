#pragma once

#include <string>
#include "MenuIcon.hpp"

class MenuItem
{
    public:
        MenuItem(
            const std::string& name, 
            int nextScreenId,
            MenuIcon icon = MenuIcon::NONE) : 
            name(name), 
            nextScreenId(nextScreenId),
            icon(icon) {}

        const int GetNextScreenId() const { return nextScreenId; }
        const std::string &GetName() const { return name; }
        const MenuIcon GetIcon() const { return icon; }

    private:
        std::string name;
        int nextScreenId;
        MenuIcon icon;

};