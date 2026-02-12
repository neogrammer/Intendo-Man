#pragma once
#include "../Objects/GameObject.h"


#include <winrt/Windows.Foundation.Numerics.h>

namespace game
{
	class Tile : public GameObject
	{
		bool solid;
	public:
		Tile();
		explicit Tile(Cfg::Textures texID_,
			winrt::Windows::Foundation::Numerics::float2 worldSize_,
			winrt::Windows::Foundation::Numerics::float2 frameSize_,
			bool solid_ = true,
			winrt::Windows::Foundation::Numerics::float2 texPosition_ = { 0.0f, 0.0f },
			winrt::Windows::Foundation::Numerics::float2 textureOffset_ = { 0.0f, 0.0f },
			winrt::Windows::Foundation::Numerics::float2 worldPosition_ = { 0.f,0.f });

		~Tile();

		Tile(const Tile& o) = default;
		Tile(Tile&& o) = default;
		Tile& operator=(const Tile& o) = default;
		Tile& operator=(Tile&& o) = default;

		bool isSolid();
	};
}