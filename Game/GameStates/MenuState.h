#pragma once

#include "GameState.h"

class MenuState : public GameState
{
	std::wstring type();
public:
	std::wstring getType() override final;
	MenuState();
	MenuState(const MenuState&);
	MenuState(MenuState&&);
	MenuState& operator=(const MenuState&);
	MenuState& operator=(MenuState&&);
	~MenuState() override final;
};