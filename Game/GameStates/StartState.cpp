#include "pch.h"
#include "StartState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/SpriteBatchScope.h"
#include "../../Engine/Text.h"

namespace game
{

	std::wstring  StartState::type()
	{
		// TODO: insert return statement here
		return L"StartState";
	}

	void StartState::enter()
	{
		uiStrings.clear();

	}

	void StartState::exit()
	{
		uiStrings.clear();

	}



	void StartState::processInput(const engine::ActionMap& actMap_)
	{
		(actMap_);
	}

	void StartState::update(float dt_)
	{
		(dt_);
	}

	std::vector<engine::Text>& StartState::render(engine::SpriteBatchScope const& batch_)
	{
		(batch_);
		return uiStrings;
	}

	StartState::StartState()
		: GameState{}
	{
	}


	StartState::~StartState()
	{
	}
}
