#include "pch.h"
#include "TitleState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Sprite.h"
#include "../../Engine/Text.h"
#include "../../Engine/SpriteBatchScope.h"

namespace game
{

	std::wstring TitleState::type()
	{
		// TODO: insert return statement here
		return L"TitleState";
	}

	void TitleState::enter()
	{
		uiStrings.clear();
	}

	void TitleState::exit()
	{
		uiStrings.clear();

	}

	void TitleState::processInput(const  engine::ActionMap& actMap_)
	{
		(actMap_);

	}

	void TitleState::update(float dt_)
	{
		(dt_);
	}

	std::vector<engine::Text>& TitleState::render(engine::SpriteBatchScope const& batch_)
	{
		(batch_);

		return uiStrings;
	}



	TitleState::TitleState()
		: GameState{}
	{
	}

	TitleState::~TitleState()
	{
	}
}