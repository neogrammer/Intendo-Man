#include "pch.h"
#include "MenuState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/SpriteBatchScope.h"

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
		(actMap_);
	}

	void MenuState::update(float dt_)
	{
		(dt_);
	}

	std::vector<engine::Text>& MenuState::render(engine::SpriteBatchScope const& batch_) 
	{
		(batch_);

		return uiStrings;
	}

	MenuState::MenuState()
		: GameState{}
	{
	}

	MenuState::~MenuState()
	{
	}
}
