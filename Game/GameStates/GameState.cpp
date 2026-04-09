#include "pch.h"
#include "GameState.h"
#include <iostream>

#include "../Game/Resources/Cfg.h"
#include "../../Engine/ActionMap.h"
#include "../../Engine/Camera2D.h"
#include "../../Engine/SpriteBatchScope.h"
#include "../../Engine/Text.h"
#include <utility>

namespace game
{
	std::wstring GameState::type()
	{
		return L"GameState";
	}

	bool GameState::isType(const std::wstring& type_)
	{
		Cfg::debugPrint(L"getType() " + getType() + L" : testType " + type_);

		return (type_ == getType());
	}

	std::wstring GameState::getType()
	{
		return type();
	}

	GameState::GameState()
		: GameStateBase{}
		, camera{nullptr}
		, uiStrings{}
		, actMap{nullptr}
		, cameraOffset{ 0,0 }
	{

		camera = std::make_shared<engine::Camera2D>();
		camera->Reset();
		cameraOffset = { 0,0 };
	}



	GameState::~GameState()
	{
	}

	std::shared_ptr<engine::Camera2D> GameState::getCamera()
	{
		if (!camera)
			camera = std::make_shared<engine::Camera2D>();
		return camera;
	}
	float2 GameState::getCamOffset()
	{
		return cameraOffset;
	}
}