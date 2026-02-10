#include "pch.h"
#include "StageSelectState.h"

std::wstring  StageSelectState::type()
{
	// TODO: insert return statement here
	return L"StageSelectState";

}

std::wstring StageSelectState::getType()
{
	// TODO: insert return statement here
	return L"StageSelectState";

}

StageSelectState::StageSelectState()
	: GameState{}
{
}

StageSelectState::StageSelectState(const StageSelectState&)
{
}

StageSelectState::StageSelectState(StageSelectState&&)
{
}

StageSelectState& StageSelectState::operator=(const StageSelectState&)
{
	// TODO: insert return statement here
	return *this;
}

StageSelectState& StageSelectState::operator=(StageSelectState&&)
{
	// TODO: insert return statement here
	return *this;
}

StageSelectState::~StageSelectState()
{
}
