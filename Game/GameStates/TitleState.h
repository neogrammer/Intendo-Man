#pragma once
#include "GameState.h"


class TitleState : public GameState
{
	std::wstring type();
public:
	std::wstring getType() override final;
	TitleState();
	TitleState(const TitleState&);
	TitleState(TitleState&&);
	TitleState& operator=(const TitleState&);
	TitleState& operator=(TitleState&&);
	~TitleState() override final;
};