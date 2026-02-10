#include "pch.h"
#include "TitleState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include "../../Engine/Text.h"

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

	}

	void TitleState::update(float dt_)
	{
	}

	std::vector<engine::Text>& TitleState::render(engine::Renderer2D& renderer_)
	{
		return uiStrings;
	}



	TitleState::TitleState()
		: GameState{}
	{
	}

	TitleState::TitleState(const TitleState&)
	{
	}

	TitleState::TitleState(TitleState&&)
	{
	}

	TitleState& TitleState::operator=(const TitleState&)
	{
		// TODO: insert return statement here
		return *this;
	}

	TitleState& TitleState::operator=(TitleState&&)
	{
		// TODO: insert return statement here
		return *this;
	}

	TitleState::~TitleState()
	{
	}
}