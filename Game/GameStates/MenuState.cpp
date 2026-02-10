#include "pch.h"
#include "MenuState.h"

std::wstring  MenuState::type()
{
	return L"MenuState";
}

std::wstring MenuState::getType()
{
	// TODO: insert return statement here
	return L"MenuState";

}

MenuState::MenuState()
	: GameState{}
{
}

MenuState::MenuState(const MenuState&)
{
}

MenuState::MenuState(MenuState&&)
{
}

MenuState& MenuState::operator=(const MenuState&)
{
	// TODO: insert return statement here
	return *this;
}

MenuState& MenuState::operator=(MenuState&&)
{
	// TODO: insert return statement here
	return *this;
}

MenuState::~MenuState()
{
}
