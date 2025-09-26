#pragma once

#include "example_game.hpp"

namespace JanSordid::SDL_Example
{
	class MapEditorState final : public MyGameState
	{
		using Base = MyGameState;

	protected:
		using Tile = HSNR64::Tile64K;

		HSNR64::TileSet _tileSet;
		HSNR64::TileMap _tileMaps[2];

		// Map Viewport
		FPoint  _viewScale  = { 2, 2 };
		FPoint  _viewOffset = { -256, -256 };

		// Palette
		FPoint  _paletteScale    = { 2, 2 };
		FPoint  _paletteOffset   = { 0, 0 };

		int     _selectedLayer   = 0;
		int     _selectedIndex   = 0;
		int     _selectedMod     = 0; // Index of Rotations & Flips
		int     _selectedColor   = 10;
		int     _selectedAlpha   = 4;
		int     _selectedRotation= 0;
		int     _selectedFlip    = 0;
		int     _buttonHeld      = 0;
		bool    _isKeepColor     = false;
		bool    _isKeepAlpha     = false;

		// Care: all below might be junk

		Texture * bg[4] = { nullptr };
		Point bgSize[4];
		const FPoint bgStart[4] = {
			{ 0,    -330 },
			{ -350, -330 },
			{ -450, -900 },
			{ -800, -1500 },
		};
		const FPoint bgFactor[4] = {
			{ 0.2f, 0.3f },
			{ 0.4f, 0.45f },
			{ 0.8f, 0.8f },
			{ 1.2f, 1.2f },
		};
		bool bgIsVisible[4] = {
			true,
			true,
			true,
			true,
		};
		FPoint mouseOffset      = { 0, 0 };
		FPoint mouseOffsetEased = { 0, 0 };

		bool   isInverted = false;
		bool   isEased    = true;
		bool   isFlux     = true;
		FPoint cam { .x = 0, .y = 0 };

	public:
		/// Ctors & Dtor
		using Base::Base;

		void Init() override;
		void Destroy() override;

		template <typename E>
		void HandleSpecificEvent( const E & ev );
		bool HandleEvent( const Event & event ) override;
		void Update( const u64 framesSinceStart, const u64 msSinceStart, const f32 deltaT ) override;
		void Render( const u64 framesSinceStart, const u64 msSinceStart, const f32 deltaTNeeded ) override;
		ImGuiOnly(
			void RenderUI( const u64 framesSinceStart, const u64 msSinceStart, const f32 deltaTNeeded ) override;
		)

		void SaveMap() const;

		FPoint CalcFluxCam( const u32 totalMSec ) const;
		void RenderLayer( const Point winSize, const FPoint camPos, const int index ) const;
	};
}
