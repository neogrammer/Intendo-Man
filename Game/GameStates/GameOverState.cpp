#include "pch.h"
#include "GameOverState.h"

#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/Renderer2D.h"
#include "../../Engine/Text.h"

namespace game
{

	std::wstring GameOverState::type()
	{
		// TODO: insert return statement here
		return L"GameOverState";
	}


	void GameOverState::enter()
	{
		uiStrings.clear();

	}

	void GameOverState::exit()
	{
		uiStrings.clear();

	}

	void GameOverState::processInput(const engine::ActionMap& actMap_)
	{
	}

	void GameOverState::update(float dt_)
	{
	}

	std::vector<engine::Text>& GameOverState::render(engine::Renderer2D& renderer_)
	{
		return uiStrings;
	}

	GameOverState::GameOverState()
		: GameState{}
	{
	}

	GameOverState::GameOverState(const GameOverState&)
	{
	}

	GameOverState::GameOverState(GameOverState&&)
	{
	}

	GameOverState& GameOverState::operator=(const GameOverState&)
	{
		// TODO: insert return statement here
		return *this;
	}

	GameOverState& GameOverState::operator=(GameOverState&&)
	{
		// TODO: insert return statement here
		return *this;
	}

	GameOverState::~GameOverState()
	{
	}
}