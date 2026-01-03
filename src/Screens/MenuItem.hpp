#pragma once

#include <string>
#include "MenuIcon.hpp"

class MenuItem
{
    public:
        MenuItem(
            const std::string& name
            , const std::string &nextScreenId
            , MenuIcon icon = MenuIcon::NONE
            , bool previous = false
        )
            : name_(name)
            , nextScreenId_(nextScreenId)
            , icon_(icon)
            , previous_(previous)
        {
            if(nextScreenId_ == "")
                nextScreenId_ = name_;
        }

        inline const std::string &GetNextScreenId() const { return nextScreenId_; }
        inline const std::string &GetName() const { return name_; }
        inline const MenuIcon GetIcon() const { return icon_; }
        inline bool IsPrevious() const { return previous_; }

    private:
        std::string name_;
        std::string nextScreenId_;
        MenuIcon icon_;
        bool previous_;
};