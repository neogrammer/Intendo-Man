#include "pch.h"
#include "PlayState.h"

std::wstring  PlayState::type()
{
	// TODO: insert return statement here
	return L"PlayState";

}

std::wstring PlayState::getType()
{
	// TODO: insert return statement here
	return L"PlayState";

}

PlayState::PlayState()
	: GameState{}
{
}

PlayState::PlayState(const PlayState&)
{
}

PlayState::PlayState(PlayState&&)
{
}

PlayState& PlayState::operator=(const PlayState&)
{
	// TODO: insert return statement here
	return *this;
}

PlayState& PlayState::operator=(PlayState&&)
{
	// TODO: insert return statement here
	return *this;
}

PlayState::~PlayState()
{
}
