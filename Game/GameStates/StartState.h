#pragma once
#include "GameState.h"


class StartState : public GameState
{
	std::wstring type();
public:
	std::wstring getType() override final;
	StartState();
	StartState(const StartState&);
	StartState(StartState&&);
	StartState& operator=(const StartState&);
	StartState& operator=(StartState&&);
	~StartState() override final;
};