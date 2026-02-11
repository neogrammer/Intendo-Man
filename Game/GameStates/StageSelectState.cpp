#include "pch.h"
#include "StageSelectState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
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
	}

	void StageSelectState::update(float dt_)
	{
	}

	std::vector<engine::Text>& StageSelectState::render(engine::Renderer2D& renderer_)
	{
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
