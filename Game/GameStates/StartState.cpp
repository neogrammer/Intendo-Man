#include "pch.h"
#include "StartState.h"

std::wstring  StartState::type()
{
	// TODO: insert return statement here
	return L"StartState";

}

std::wstring StartState::getType()
{
	// TODO: insert return statement here
	return L"StartState";

}

StartState::StartState()
	: GameState{}
{
}

StartState::StartState(const StartState&)
{
}

StartState::StartState(StartState&&)
{
}

StartState& StartState::operator=(const StartState&)
{
	// TODO: insert return statement here
	return *this;
}

StartState& StartState::operator=(StartState&&)
{
	// TODO: insert return statement here
	return *this;
}

StartState::~StartState()
{
}
