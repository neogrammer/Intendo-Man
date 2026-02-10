#include "pch.h"
#include "TitleState.h"

std::wstring TitleState::type()
{
	// TODO: insert return statement here
	return L"TitleState";

}

std::wstring TitleState::getType()
{
	// TODO: insert return statement here
	return L"TitleState";

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
