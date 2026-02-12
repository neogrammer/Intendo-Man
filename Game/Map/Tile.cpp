#include "pch.h"
#include "Tile.h"


namespace game
{

	using winrt::Windows::Foundation::Numerics::float2;

	Tile::Tile()
	{

	}

	game::Tile::Tile(Cfg::Textures texID_, float2 worldSize_, float2 frameSize_, bool solid_, float2 texPosition_, float2 textureOffset_, float2 worldPosition_)
		: GameObject{ texID_, worldPosition_, worldSize_, frameSize_, texPosition_, textureOffset_ }
		, solid{ solid_ }
	{
	}

	Tile::~Tile()
	{

	}

	bool game::Tile::isSolid()
	{
		return solid;
	}
}