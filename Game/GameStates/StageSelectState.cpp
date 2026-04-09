#include "pch.h"
#include "StageSelectState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/SpriteBatchScope.h"
#include "../../Engine/Text.h"

namespace game
{

	std::wstring  StageSelectState::type()
	{
		// TODO: insert return statement here
		return L"StageSelectState";
	}

	void StageSelectState::enter()
	{
		uiStrings.clear();

	}

	void StageSelectState::exit()
	{
		uiStrings.clear();

	}



	void StageSelectState::processInput(const  engine::ActionMap& actMap_)
	{
		(actMap_);

	}

	void StageSelectState::update(float dt_)
	{
		(dt_);

	}

	std::vector<engine::Text>& StageSelectState::render(engine::SpriteBatchScope const& batch_)
	{
		(batch_);

		return uiStrings;
	}

	StageSelectState::StageSelectState()
		: GameState{}
	{
	}


	StageSelectState::~StageSelectState()
	{
	}
}
