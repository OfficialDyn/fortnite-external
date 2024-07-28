#include "visuals.hpp"
#include <core/game/sdk.hpp>

auto itx::visuals_c::ActorLoop( ) -> void
{
	ImDrawList* iDrawList = ImGui::GetBackgroundDrawList( );

	for ( auto& Actor : ActorList )
	{
		auto Pawn = Actor.Pawn;
		auto Mesh = Actor.Mesh;
		auto pBoneArray = Actor.pBoneArray;

        if ( !Pawn )
            continue;

        if ( !Mesh )
            continue;
		
		Cached::ViewState->UpdateCamera( );

		auto vHeadWorld = Mesh->GetBoneMatrix( pBoneArray, EBoneIndex::Head );
		auto vHeadScreen = Cached::ViewState->WorldToScreen( { vHeadWorld.x, vHeadWorld.y, vHeadWorld.z + 15 } );

		auto vRootWorld = Mesh->GetBoneMatrix( pBoneArray, EBoneIndex::Root );
		auto vRootScreen = Cached::ViewState->WorldToScreen( vRootWorld );

        auto fDistance = Camera::Location.Distance( vRootWorld ) / 100.f;
        if ( fDistance >= itx::settings.iRenderDistance )
            continue;

        if ( Pawn->IsDespawning( ) )
            continue;

        if ( !vHeadScreen.OnScreen( ) && !vRootScreen.OnScreen( ) )
            continue;

        if ( Cached::Pawn )
        {
            if ( Cached::TeamIndex == Actor.PlayerState->GetTeamIndex( ) )
                continue;
        }

		auto fBoxSize = ( abs( vHeadScreen.y - vRootScreen.y ) ) * 0.45;
		auto bBounds = Mesh->GetStaticBounds( vHeadScreen, vRootScreen, fBoxSize );

        bool bVisible = itx::settings.bVisibleCheck ? Mesh->IsVisible( ) : true;
        ImColor iColor = bVisible ? itx::settings.iVisibleColor : itx::settings.iInvisibleColor;

		if ( itx::settings.bUsername )
		{
			auto vSize = ImGui::CalcTextSize( Actor.Username.c_str( ) );
			iDrawList->AddText( ImVec2( ( ( bBounds.left + bBounds.right ) / 2 ) - ( vSize.x / 2 ) - 1, bBounds.top - 14.f ), iColor, Actor.Username.c_str( ) , 0, itx::settings.bTextOutline );
		}

        if ( itx::settings.bDistance )
        {
            auto sDistance = _s( "[" ) + std::to_string( int( fDistance ) ) + _s( "m]" );
            auto vSize = ImGui::CalcTextSize( sDistance.c_str( ) );
            float fPadding = itx::settings.bHeldWeapon ? 15.f : 0.f;
            iDrawList->AddText( ImVec2( ( ( bBounds.left + bBounds.right ) / 2 ) - ( vSize.x / 2 ) - 1, bBounds.bottom + 3.f + fPadding ), iColor, sDistance.c_str( ), 0, itx::settings.bTextOutline );
        }

        if ( itx::settings.bHeldWeapon )
        {
            auto WeaponData = Pawn->GetCurrentWeapon( )->GetWeaponData( );

            std::string sBottomText;

            auto iBuildingState = itx::memory.read<uint8_t>( std::uintptr_t( Pawn ) + Offset::BuildingState );
            if ( iBuildingState == 2 )
                sBottomText = WeaponData->GetDisplayName( );
            else
                sBottomText = _s( "Building Plan" );

            auto vSize = ImGui::CalcTextSize( sBottomText.c_str( ) );
            iDrawList->AddText( ImVec2( ( ( bBounds.left + bBounds.right ) / 2 ) - ( vSize.x / 2 ) - 1, bBounds.bottom + 3.f ), iColor, sBottomText.c_str( ), 0, itx::settings.bTextOutline );
        }

        if ( itx::settings.bSkeletons )
        {
            for ( const auto& ePair : eBonePairs )
            {
                auto vBone1 = Cached::ViewState->WorldToScreen( Mesh->GetBoneMatrix( pBoneArray, ePair.first ) );
                auto vBone2 = Cached::ViewState->WorldToScreen( Mesh->GetBoneMatrix( pBoneArray, ePair.second ) );

                iDrawList->AddLine( ImVec2( vBone1.x, vBone1.y ), ImVec2( vBone2.x, vBone2.y ), iColor, itx::settings.iSkeletonThickness );
            }
        }

        if ( itx::settings.bBox )
        {
            if ( !itx::settings.iBoxType )
            {
                if ( itx::settings.bBoxOutline )
                    iDrawList->AddRect( ImVec2( bBounds.left, bBounds.top ), ImVec2( bBounds.right, bBounds.bottom ), ImColor( 0, 0, 0, 200 ), 0, 0, itx::settings.iBoxThickness * 2.f );

                iDrawList->AddRect( ImVec2( bBounds.left, bBounds.top ), ImVec2( bBounds.right, bBounds.bottom ), iColor, 0, 0, itx::settings.iBoxThickness );
            }
            else if ( itx::settings.iBoxType )
            {
                auto DrawCorneredBox = [ & ]( int x, int y, int w, int h, const ImColor color, int thickness ) -> void
                    {
                        float fLineWidth = ( w / 3 );
                        float fLineHeight = ( h / 9 );
                        iDrawList->AddLine( ImVec2( x, y ), ImVec2( x, y + fLineHeight ), color, thickness );
                        iDrawList->AddLine( ImVec2( x, y ), ImVec2( x + fLineWidth, y ), color, thickness );
                        iDrawList->AddLine( ImVec2( x + w - fLineWidth, y ), ImVec2( x + w, y ), color, thickness );
                        iDrawList->AddLine( ImVec2( x + w, y ), ImVec2( x + w, y + fLineHeight ), color, thickness );
                        iDrawList->AddLine( ImVec2( x, y + h - fLineHeight ), ImVec2( x, y + h ), color, thickness );
                        iDrawList->AddLine( ImVec2( x, y + h ), ImVec2( x + fLineWidth, y + h ), color, thickness );
                        iDrawList->AddLine( ImVec2( x + w - fLineWidth, y + h ), ImVec2( x + w, y + h ), color, thickness );
                        iDrawList->AddLine( ImVec2( x + w, y + h - fLineHeight ), ImVec2( x + w, y + h ), color, thickness );
                    };

                if ( itx::settings.bBoxOutline )
                    DrawCorneredBox( bBounds.left, bBounds.top, bBounds.right - bBounds.left, bBounds.bottom - bBounds.top, ImColor( 0, 0, 0, 200 ), itx::settings.iBoxThickness * 1.5f );
                
                DrawCorneredBox( bBounds.left, bBounds.top, bBounds.right - bBounds.left, bBounds.bottom - bBounds.top, iColor, itx::settings.iBoxThickness );
            }
        }
	}
}