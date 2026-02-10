#pragma once

#include "GameState.h"

class StageSelectState : public GameState
{
	std::wstring type();
public:
	std::wstring getType() override final;
	StageSelectState();
	StageSelectState(const StageSelectState&);
	StageSelectState(StageSelectState&&);
	StageSelectState& operator=(const StageSelectState&);
	StageSelectState& operator=(StageSelectState&&);
	~StageSelectState() override final;
};