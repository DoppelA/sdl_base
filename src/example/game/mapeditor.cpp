#include "example_game.hpp"
#include "mapeditor.hpp"

#include <hsnr64/tiles.hpp>

namespace JanSordid::SDL_Example
{
	using namespace HSNR64;

	// Care: This is far from finished and still buggy
	void MapEditorState::Init()
	{
		Base::Init();

		Owned<SDL_Surface> surface = IMG_Load( BasePathGraphic "hsnr64.png" );
		if( surface != nullptr )
		{
			//SDL_SetColorKey( surface,  true, SDL_MapRGB( surface->format, 0, 0, 0 ) );
			const u32 paletteIndex = SDL_MapRGB( SDL_GetPixelFormatDetails( surface->format ), SDL_GetSurfacePalette( surface ), 0, 0, 0 );
			SDL_SetSurfaceColorKey( surface, true, paletteIndex );

			_tileSet.texture     = SDL_CreateTextureFromSurface( renderer(), surface );
			_tileSet.textureSize = { surface->w, surface->h };
			_tileSet.tileSize    = { 16, 16 };
			_tileSet.tileCount   = _tileSet.textureSize / _tileSet.tileSize; // kind of unnecessary
		}

		_tileMaps[0].tileSet   = &_tileSet;
		_tileMaps[0].tileDist  = toF( _tileMaps[0].tileSet->tileSize );
		_tileMaps[0].sizeScale = { 2, 2 };
		_tileMaps[0].center    = toF( _tileMaps[0].tileSet->tileSize ) * _tileMaps[0].sizeScale * _tileMaps[0].halfSize;
		_tileMaps[0].size      = { 128, 128 };
		_tileMaps[0].stride    = _tileMaps[0].size.x; // Needed if tileMap.tiles is one dimensional
	//	tileMaps[0].scaleMode = SDL_ScaleModeBest;

		// TODO: Move this to a "create" method?
		_tileMaps[0].tiles.resize( _tileMaps[0].size.x * _tileMaps[0].size.y );

		_tileMaps[1].tileSet   = &_tileSet;
		_tileMaps[1].tileDist  = toF( _tileMaps[1].tileSet->tileSize );
	//	tileMaps[1].tileDist  = { 15, 15 };
		_tileMaps[1].sizeScale = { 2, 2 };
		_tileMaps[1].center    = toF( _tileMaps[1].tileSet->tileSize ) * _tileMaps[1].sizeScale * _tileMaps[1].halfSize;
		_tileMaps[1].size      = { 128, 128 };
		_tileMaps[1].stride    = _tileMaps[1].size.x; // Needed if tileMap.tiles is one dimensional
	//	tileMaps[1].scaleMode = SDL_ScaleModeBest;

		_tileMaps[1].tiles.resize( _tileMaps[1].size.x * _tileMaps[1].size.y );

		// Fill map layer 0 with randomness, color layer
		for( int y = 0; y < _tileMaps[0].size.y; ++y )
		{
			for( int x = 0; x < _tileMaps[0].size.x; ++x )
			{
				const int index  = x + y * _tileMaps[0].stride;
				_tileMaps[0].tiles[index] = Tile{ TileColor{ (u8) (16 + (rand() % 6)), 3 }, FlipRot::None, 1000 };
			}
		}

		// Fill map layer 1 with randomness
		for( int y = 0; y < _tileMaps[1].size.y; ++y )
		{
			for( int x = 0; x < _tileMaps[1].size.x; ++x )
			{
				const int index  = x + y * _tileMaps[1].stride;
				const int tIndex = 832 + (rand() & 255);
				_tileMaps[1].tiles[index] = Tile{ TileColor{ (u8) (27 + (rand() % 6)), 2 }, (FlipRot)(rand() % 9), (u16)tIndex };
			}
		}

		Point windowSize;
		SDL_GetWindowSize( window(), &windowSize.x, &windowSize.y );

		_paletteOffset = { (f32)windowSize.x - 512, -768 };
	}

	void MapEditorState::Destroy()
	{
		Base::Destroy();
	}

	template <typename E>
	void MapEditorState::HandleSpecificEvent( const E & ev )
	{
		// Not implemented by design
		// Assert here to catch unhandled events
		//assert( false );
	}

	template <>
	void MapEditorState::HandleSpecificEvent( const SDL_MouseMotionEvent & ev )
	{
		if( ev.state == 0 )
			return;

		if( _buttonHeld == 1 )
		{
			// Palette
			FPoint pos     = FPoint{ ev.x, ev.y } - _paletteOffset;
			Point  index2d = toI( pos ) / toI( toF( _tileSet.tileSize ) * _paletteScale );
			u16    index1d = index2d.x + index2d.y * _tileSet.tileCount.x;
			print( "pos:   {} {}\n", pos.x, pos.y );
			print( "index: {} {}\n", index2d.x, index2d.y );
			print( "index: {}\n", index1d );
			_selectedIndex = index1d;
		}
		else if ( _buttonHeld == 2 )
		{
			// Map
			FPoint pos     = FPoint{ ev.x, ev.y } - _viewOffset;
			Point  index2d = toI( pos ) / toI( toF( _tileSet.tileSize ) * _viewScale );
			u16    index1d = index2d.x + index2d.y * _tileMaps[1].size.x;
			print( "pos:   {} {}\n", pos.x, pos.y );
			print( "index: {} {}\n", index2d.x, index2d.y );
			print( "index: {}\n", index1d );

			const bool isTransparent = _selectedAlpha == 0;
			Tile &     modTile       = _tileMaps[1].tiles[index1d];

			if( isTransparent )
				modTile.index( 0 );
			else if( 0 <= _selectedIndex )
				modTile.index( _selectedIndex );

			if( 0 <= _selectedColor && _selectedColor <= 63 )
				modTile.color = _selectedColor;
			if( 1 <= _selectedAlpha && _selectedAlpha <= 4 )
				modTile.alpha = _selectedAlpha - 1;

			FlipRot flipRot = FlipRot::None;
			if( 0 <= _selectedRotation && _selectedRotation <= 3 )
			{
				flipRot = flipRot
				        | ((_selectedRotation & (1 << 0)) ? FlipRot::Rotate90 : FlipRot::None)
				        | ((_selectedRotation & (1 << 1)) ? FlipRot::Rotate45 : FlipRot::None);
			}
			if( 0 <= _selectedFlip && _selectedFlip <= 3 )
			{
				flipRot = flipRot
				        | ((_selectedFlip & (1 << 0)) ? FlipRot::HorizontalFlip : FlipRot::None)
				        | ((_selectedFlip & (1 << 1)) ? FlipRot::VerticalFlip   : FlipRot::None);
			}
			modTile.flipRot = flipRot;
		}
	}

	template <>
	void MapEditorState::HandleSpecificEvent( const SDL_MouseButtonEvent & ev )
	{
		Point windowSize;
		SDL_GetWindowSize( window(), &windowSize.x, &windowSize.y );

		if( ev.button == SDL_BUTTON_LEFT && !ev.down )
		{
			_buttonHeld = 0;
		}
		else if( ev.button == SDL_BUTTON_LEFT && ev.down )
		{
			int xDivider = (windowSize.x - 512);

			if( ev.x >= xDivider )
			{
				const FPoint paletteTileSize = toF( _tileSet.tileSize ) * _paletteScale;

				// Palette side
				if( ev.y > 128 )
				{
					// Palette
					_buttonHeld = 1;

					FPoint pos     = FPoint { ev.x, ev.y } - _paletteOffset;
					Point  index2d = toI( pos ) / toI( paletteTileSize );
					u16    index1d = index2d.x + index2d.y * _tileSet.tileCount.x;
					print( "pos:   {} {}\n", pos.x, pos.y );
					print( "index: {} {}\n", index2d.x, index2d.y );
					print( "index: {}\n", index1d );
					_selectedIndex = index1d;
				}
				else
				{
					// Modifiers
					_buttonHeld = 0; // can not hold

					const f32    spacing   = 2.f;
					const FPoint modOffset = { (f32)windowSize.x - 512, 0 };

				//	const Point sel      = { x, y };
				//	const FRect dst      = toF( sel ) * paletteTileSize * spacing + toF( modOffset ) + toWH( paletteTileSize );

					FPoint pos       = FPoint { ev.x, ev.y } - modOffset;
					Point  index2d   = toI( pos ) / toI( paletteTileSize * spacing );
					u16    index1d   = index2d.x + index2d.y * 8;
					_selectedFlip     = (index2d.x & 3);
					_selectedRotation = ((index2d.x & 4) != 0) + index2d.y * 2;
					print( "pos:   {} {}\n", pos.x, pos.y );
					print( "index: {} {}\n", index2d.x, index2d.y );
					print( "index: {}\n", index1d );
					//selectedIndex = index1d;
				}
			}
			else
			{
				// Map
				_buttonHeld = 2;

				FPoint pos     = FPoint{ ev.x, ev.y } - _viewOffset;
				Point  index2d = toI( pos ) / toI( toF( _tileSet.tileSize ) * _viewScale );
				u16    index1d = index2d.x + index2d.y * _tileMaps[1].size.x;
				print( "pos:   {} {}\n", pos.x, pos.y );
				print( "index: {} {}\n", index2d.x, index2d.y );
				print( "index: {}\n", index1d );

				const bool isTransparent = _selectedAlpha == 0;
				Tile &     modTile       = _tileMaps[1].tiles[index1d];

				if( isTransparent )
					modTile.index( 0 );
				else if( 0 <= _selectedIndex )
					modTile.index( _selectedIndex );

				if( 0 <= _selectedColor && _selectedColor <= 63 )
					modTile.color = _selectedColor;
				if( 1 <= _selectedAlpha && _selectedAlpha <= 4 )
					modTile.alpha = _selectedAlpha - 1;

				FlipRot flipRot = FlipRot::None;
				if( 0 <= _selectedRotation && _selectedRotation <= 3 )
				{
					flipRot = flipRot
					        | ((_selectedRotation & (1 << 0)) ? FlipRot::Rotate90 : FlipRot::None)
					        | ((_selectedRotation & (1 << 1)) ? FlipRot::Rotate45 : FlipRot::None);
				}
				if( 0 <= _selectedFlip && _selectedFlip <= 3 )
				{
					flipRot = flipRot
					        | ((_selectedFlip & (1 << 0)) ? FlipRot::HorizontalFlip : FlipRot::None)
					        | ((_selectedFlip & (1 << 1)) ? FlipRot::VerticalFlip   : FlipRot::None);
				}
				modTile.flipRot = flipRot;
			}
		}
	}

	template <>
	void MapEditorState::HandleSpecificEvent( const SDL_MouseWheelEvent & ev )
	{
		Point windowSize;
		SDL_GetWindowSize( window(), &windowSize.x, &windowSize.y );

		const Keymod keyMod    = SDL_GetModState();
		const bool   isModded  = keyMod & SDL_KMOD_SHIFT;
		const bool   isFlipped = ev.direction == SDL_MouseWheelDirection::SDL_MOUSEWHEEL_FLIPPED;
		const int    flipFactor= isFlipped ? -1 : 1;
		const f32    xDivider  = (windowSize.x - 512);
		const FPoint scroll    = isModded ? FPoint { ev.y * flipFactor, -ev.x } : FPoint { -ev.x, ev.y * flipFactor };

		if( ev.mouse_x >= xDivider )
		{
			// Palette
			_paletteOffset  += scroll * 16.0f;
			_paletteOffset.x = std::clamp<f32>( _paletteOffset.x, windowSize.x - _tileSet.textureSize.x * _paletteScale.x - 16, xDivider + 16 );
			_paletteOffset.y = std::clamp<f32>( _paletteOffset.y, windowSize.y - _tileSet.textureSize.y * _paletteScale.y - 16, 128 + 16 );
		}
		else
		{
			// Map
			const Point min = windowSize - toI( toF( _tileMaps[1].size * _tileSet.tileSize ) * _viewScale );
			_viewOffset     += scroll * 16.0f;
			_viewOffset.x    = std::clamp<f32>( _viewOffset.x, min.x - 512, 0 );
			_viewOffset.y    = std::clamp<f32>( _viewOffset.y, min.y      , 0 );
		}
	}

	bool MapEditorState::HandleEvent( const Event & ev )
	{
		switch( ev.type )
		{
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				MapEditorState::HandleSpecificEvent( ev.key );
				break;

			case SDL_EVENT_MOUSE_MOTION:
				MapEditorState::HandleSpecificEvent( ev.motion );
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				MapEditorState::HandleSpecificEvent( ev.button );
				break;

			case SDL_EVENT_MOUSE_WHEEL:
				MapEditorState::HandleSpecificEvent( ev.wheel );
				break;

			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
				MapEditorState::HandleSpecificEvent( ev.gaxis );
				break;

			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
				MapEditorState::HandleSpecificEvent( ev.gbutton );
				break;
		}

		return true; // Not really correct
	}

	void MapEditorState::Update( const u64 frame, const u64 totalMSec, const float deltaT )
	{}

#ifdef IMGUI

	void MapEditorState::RenderUI( const u64 framesSinceStart, const u64 msSinceStart, const float deltaTNeeded )
	{
		static bool auto_update = false;
		static bool drawColorNumber = false;
		//ImGuiIO & io = ImGui::GetIO();
		ImGui::Begin( "MapEditor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing );
		if( framesSinceStart == 0 )
			ImGui::SetWindowFocus( nullptr );

		//if( ImGui::SliderInt( "int", &p.x, 0, 320 ) && auto_update )
			//_blendedText = nullptr;
		//	nullptr;

		ImGui::Checkbox( "Auto-Redraw", &auto_update );     // Edit bools storing our window open/close state

		if( ImGui::Button( "Save" ) )                     // Buttons return true when clicked (most widgets return true when edited/activated)
			SaveMap();

		ImGui::SliderFloat2( "Blah", (float*)(void*)&_viewScale, 0.1, 8.0 );

		if( ImGui::SliderInt( "Layer", &_selectedLayer, 0, 4 ) )
		{ }

		if( ImGui::SliderInt( "Color", &_selectedColor, -1, 63 ) )
		{ }

		//ImGui::Checkbox( "Keep Alpha", &isKeepAlpha );
		if( ImGui::SliderInt( "Alpha", &_selectedAlpha, -1, 4 ) )
		{ }

		if( ImGui::SliderInt( "Flip", &_selectedFlip, -1, 3 ) )
		{ }

		if( ImGui::SliderInt( "Rotation", &_selectedRotation, -1, 3 ) )
		{ }

		ImGui::Checkbox( "Draw Color Number", &drawColorNumber );

		ImGui::PushID( "fg color" );
		ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1 );
		// CARE: ImU32 as color is 0XAABBGGRR - opposite of what might be expected
		ImGui::PushStyleColor( ImGuiCol_Border, 0xAAFFFFFF );
		constexpr fmt::format_string<int>
			withNumber   ( "{:02}" ),
			withoutNumber( "  ##{:02}" );
		const fmt::format_string<int> & fmt = drawColorNumber
			? withNumber
			: withoutNumber;
		for( int i = 0; i < 64; ++i )
		{
			ImU32 pcol = std::bit_cast<ImU32>( HSNR64::Palette( i ) );
			//Color color = hsnr64::Palette[i];
			ImGui::PushStyleColor( ImGuiCol_Button, pcol );
			ImGui::PushStyleColor( ImGuiCol_Text, pcol ^ 0x00808080 );
			if( ImGui::Button( format( fmt::runtime( fmt ), i ).c_str() ) )
				_selectedColor = i;
			ImGui::PopStyleColor( 2 );
			//ImGui::ColorButton( format( "color{}", i ).c_str(), *((ImVec4*)&sor::hsnr64::Palette[i]), ImGuiColorEditFlags_Uint8 );
			//if(i%10 != 0)
			if( true
			    //&& i != 0
			    && i != 10
			    //&& i != 17
			    && i != 25
			    //&& i != 32
			    && i != 40
			    //&& i != 48
			    //&& i != 57
			    && i != 52
				)
				ImGui::SameLine();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopID();

		ImGui::End();
	}

#endif

	void MapEditorState::SaveMap() const
	{
		// TODO: Show filepicker to to allow custom naming
		const char * filename = "map-i-map.json5"; // json5 is a relaxed version of json
		File * file = std::fopen( filename, "w" );
		if( file == nullptr )
			throw fmt::system_error( errno, "cannot open file '{}'", filename );

		print( file,
			"{{"
			"\n\ttileByteSize: {},"
			"\n\tmap: {{"
			"\n\t\tlayers: [ ",
			sizeof( Tile ) );

		for( int l = 0; l < 2; ++l )
		{
			const TileMap & tileMap = _tileMaps[l];

			if( l != 0 )
				print( file, ",\n\t\t" );

			print( file,
				"{{"
				"\n\t\t\tnum: {0},"
				"\n\t\t\tsize: [ {1}, {2} ],"
				"\n\t\t\ttiles:"
				,
				l,
				tileMap.size.x,
				tileMap.size.y,
				nullptr );

			const Point  viewSize     = { 640, 480 };
			const Point  startIndex   = toI( _viewOffset / tileMap.tileDist );

			print( file, " [" );
			for( int y = 0; y < tileMap.size.y; ++y )
			{
				if( y == 0 ) [[unlikely]]
					print( file, "\n\t\t\t\t[" );
				else
					print( file, ",\n\t\t\t\t[" );

				for( int x = 0; x < tileMap.size.x; ++x )
				{
					constexpr u32 mask = (sizeof(Tile) == 3) ? 0x00ffffff : 0xffffffff;
					const int    index    = x + y * tileMap.stride;
					const Tile * currTile = &tileMap.tiles[index];
					if( x == 0 ) [[unlikely]]
					{
						u32 tileData = mask & *reinterpret_cast<const u32 *>( currTile );
						print( file, "{:#x}", tileData );
					}
					else
					{
						print( file, ",{:#x}", mask & *reinterpret_cast<const u32 *>( currTile ) );
					}
				}
				print( file, "]" );
			}
			print( file,
				"\n\t\t\t]"
				"\n\t\t}}" );
		}

		print( file,
			" ]"
			"\n\t}}"
			"\n}}" );

		std::fclose( file );
	}

	void MapEditorState::Render( const u64 framesSinceStart, const u64 msSinceStart, const f32 deltaTNeeded )
	{
		Point windowSize;
		SDL_GetWindowSize( window(), &windowSize.x, &windowSize.y );

		{
			const Rect clipRect = { 0, 0, windowSize.x - 512, windowSize.y };
			SDL_SetRenderClipRect( renderer(), &clipRect );
		}

		const u8     randColor        = (9 + (rand() % 55));
		const FRect  selectorOverhang = FRect{ -8, -8, 32, 32 } * toXYWH( _paletteScale );
		const Tile   selectorTile     = Tile{ TileColor { randColor, 3 }, FlipRot::None,  827 };
		const Tile   sentinelTile     = Tile{ TileColor { 9,         3 }, FlipRot::None, 1000 };
		const Tile * lastTile         = &sentinelTile;

		// Reset Color- and Alpha-Mod according to the sentinel Tile
		SDL_SetTextureColorMod( _tileSet.texture, 255, 255, 255 );
		SDL_SetTextureAlphaMod( _tileSet.texture, SDL_ALPHA_OPAQUE );

		// Draw Map
		for( int l = 0; l < 2; ++l )
		{
			const TileMap & tileMap = _tileMaps[l];

			SDL_SetTextureBlendMode( _tileSet.texture, tileMap.blendMode );
			SDL_SetTextureScaleMode( _tileSet.texture, tileMap.scaleMode );

			const Point  viewSize     = { 640, 480 };
			const Point  startIndex   = toI( _viewOffset / tileMap.tileDist );

			for( int y = 0; y < tileMap.size.y; ++y )
			{
				for( int x = 0; x < tileMap.size.x; ++x )
				{
					const FPoint pt       = { (f32)x, (f32)y };
					const FPoint pos      = pt * tileMap.tileDist * tileMap.sizeScale;
					const FPoint size     = toF( tileMap.tileSet->tileSize ) *  tileMap.sizeScale;
					const FRect  dst      = pos + toWH( size ) + _viewOffset;
	//				const FRect  dst      = toXY( pt, 1 ) * toXYWH( toF( tileMap.tileSet->tileSize ) * viewScale ) + toF( viewOffset );
					const int    index    = x + y * tileMap.stride;
					const Tile * currTile = &tileMap.tiles[index];
					if( currTile->index() != 0 )
					{
						DrawTile( renderer(), _tileSet.texture, *currTile, *lastTile, dst, tileMap.center );
						lastTile = currTile;
					}
				}
			}
		}

		const FPoint paletteTileSize = toF( _tileSet.tileSize ) * _paletteScale;

		// Draw selected colored Tile with all possible Modifiers (Flip, Rotate)
		{
			// unlock clipping
			SDL_SetRenderClipRect( renderer(), nullptr );

			SDL_SetTextureColorMod( _tileSet.texture, 255, 255, 255 );
			SDL_SetTextureAlphaMod( _tileSet.texture, SDL_ALPHA_OPAQUE );

			const float  spacing   = 2.f;
			const Point  modOffset = { windowSize.x - 512 + 16, 16 };

			if( _selectedIndex != 0 )
			{
				const Color color    = HSNR64::Palette( _selectedColor );
				const bool  isBright = color.r > 160 || color.g > 140 || color.b > 180;
				//if(!isBright)
				{
					const Color compColor = { (u8)(color.r ^ 0x80), (u8)(color.g ^ 0x80), (u8)(color.b ^ 0x80), SDL_ALPHA_OPAQUE };
					const FRect dst = toF( modOffset ) + FRect { -16, -16, 512, 128 };
					SDL_SetRenderDrawColor( renderer(), compColor.r, compColor.g, compColor.b, compColor.a );
					//SDL_SetRenderDrawColor( renderer(), 180, 180, 180, SDL_ALPHA_OPAQUE );
					SDL_RenderFillRect( renderer(), &dst );
				}

				for( int y = 0; y <= 1; ++y )
				{
					for( int x = 0; x <= 7; ++x )
					{
						const Point sel      = { x, y };
						const FRect dst      = toF( sel ) * paletteTileSize * spacing + toF( modOffset ) + toWH( paletteTileSize );
						const FlipRot flipRot = FlipRot::None
							| (((x & 1) != 0) ? HSNR64::FlipRot::VerticalFlip   : HSNR64::FlipRot::None)
							| (((x & 2) != 0) ? HSNR64::FlipRot::HorizontalFlip : HSNR64::FlipRot::None)
							| (((x & 4) != 0) ? HSNR64::FlipRot::Rotate90       : HSNR64::FlipRot::None)
							| ((y != 0)       ? HSNR64::FlipRot::Rotate45       : HSNR64::FlipRot::None);
						const Tile  currTile = Tile { TileColor{ (u8)_selectedColor, 3 }, flipRot, (u16)_selectedIndex };
						// Draw Cursor
						DrawTile( renderer(), _tileSet.texture, currTile, sentinelTile, dst, paletteTileSize * 0.5f );
					}
				}
			}

			_tileMaps[1].center = toF( _tileMaps[1].tileSet->tileSize ) * _tileMaps[1].sizeScale * _tileMaps[1].halfSize;

			_selectedMod = _selectedFlip | _selectedRotation << 2;
			const int   stride = 8;
			const Point sel    = {_selectedMod % stride, _selectedMod / stride };
			const FRect dst    = toF( sel * _tileSet.tileSize ) * _paletteScale * spacing + toF( modOffset ) + selectorOverhang;

			// Draw Mod Cursor
			DrawTile( renderer(), _tileSet.texture, selectorTile, sentinelTile, dst, {} );
		}

		// Draw Palette
		{
			SDL_SetTextureColorMod( _tileSet.texture, 255, 255, 255 );
			SDL_SetTextureAlphaMod( _tileSet.texture, SDL_ALPHA_OPAQUE );

			const Rect  clipRect = { windowSize.x - 512, 128, 512, windowSize.y };
			SDL_SetRenderClipRect( renderer(), &clipRect );

			const FRect palSrc = { 0, 0, (f32)_tileSet.textureSize.x, (f32)_tileSet.textureSize.y };
			const FRect palDst = {
				(f32) _paletteOffset.x,
				(f32) _paletteOffset.y,
				(f32) _tileSet.textureSize.x * _paletteScale.x,
				(f32) _tileSet.textureSize.y * _paletteScale.y };

			// Draw the Palette (the whole TileSet)
			SDL_RenderTexture( renderer(), _tileSet.texture, &palSrc, &palDst );

			const int   stride   = _tileSet.tileCount.x;
			const Point sel      = { _selectedIndex % stride, _selectedIndex / stride };
			const FRect overhang = FRect{ -8, -8, 32, 32 } * toXYWH( _paletteScale );
			const FRect dst      = toF( sel ) * paletteTileSize + _paletteOffset + selectorOverhang;

			const Rect clipRect2 = { (windowSize.x - 512) + (int)selectorOverhang.x, 128 + (int)selectorOverhang.y, 512 - (int)selectorOverhang.x, windowSize.y };
			SDL_SetRenderClipRect( renderer(), &clipRect2 );

			// Draw Palette Cursor
			DrawTile( renderer(), _tileSet.texture, selectorTile, sentinelTile, dst, {} );
		}

		// unlock clipping
		SDL_SetRenderClipRect( renderer(), nullptr );
	}
}
