#include "pch.h"
#include "MenuState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include "../../Engine/Text.h"

namespace game
{

	std::wstring  MenuState::type()
	{
		return L"MenuState";
	}

	void MenuState::enter()
	{
		uiStrings.clear();

	}

	void MenuState::exit()
	{
		uiStrings.clear();

	}



	void MenuState::processInput(const  engine::ActionMap& actMap_)
	{
	}

	void MenuState::update(float dt_)
	{
	}

	std::vector<engine::Text>& MenuState::render(engine::Renderer2D& renderer_)
	{
		return uiStrings;
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
}
