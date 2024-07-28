#include "render.hpp"

#include <Windows.h>
#include <iostream>
#include <string>
#include <d3d9.h>
#include <d3d11.h>
#include <dwmapi.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <dependencies/framework/imgui.h>
#include <dependencies/framework/backends/imgui_impl_dx11.h>
#include <dependencies/framework/backends/imgui_impl_win32.h>

#include <dependencies/framework/imgui_internal.h>
#include <dependencies/framework/custom-widgets.h>

#include <core/game/sdk.hpp>
#include <core/game/features/visuals/visuals.hpp>

ID3D11Device* D3DDevice;
ID3D11DeviceContext* D3DDeviceContext;
IDXGISwapChain* D3DSwapChain;
ID3D11RenderTargetView* D3DRenderTarget;
D3DPRESENT_PARAMETERS D3DPresentParams;
HWND hWindowHandle = 0;

auto itx::render_c::Initialize( ) -> bool
{
    //Overlay Removed And Swapped With This.
    {
        WNDCLASSEXA wcex = {
            sizeof( WNDCLASSEXA ),
            0,
            DefWindowProcA,
            0,
            0,
            nullptr,
            LoadIcon( nullptr, IDI_APPLICATION ),
            LoadCursor( nullptr, IDC_ARROW ),
            nullptr,
            nullptr,
            ( "Unknown-Cheats" ),
            LoadIcon( nullptr, IDI_APPLICATION )
        }; RegisterClassExA( &wcex );

        hWindowHandle = CreateWindowExA( NULL, ( "Unknown-Cheats" ), ( " " ), WS_POPUP, 0, 0, itx::screen.fWidth, itx::screen.fHeight, NULL, NULL, wcex.hInstance, NULL );
        if ( !hWindowHandle )
            return false;

        SetWindowLong( hWindowHandle, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED );

        MARGINS margin = { -1 };
        DwmExtendFrameIntoClientArea( hWindowHandle, &margin );

        ShowWindow( hWindowHandle, SW_SHOW );
        SetWindowPos( hWindowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        SetLayeredWindowAttributes( hWindowHandle, RGB( 0, 0, 0 ), 255, LWA_ALPHA );
        UpdateWindow( hWindowHandle );
    }

    DXGI_SWAP_CHAIN_DESC SwapChainDescription;
    ZeroMemory( &SwapChainDescription, sizeof( SwapChainDescription ) );
    SwapChainDescription.BufferCount = 2;
    SwapChainDescription.BufferDesc.Width = 0;
    SwapChainDescription.BufferDesc.Height = 0;
    SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.OutputWindow = hWindowHandle;
    SwapChainDescription.SampleDesc.Count = 1;
    SwapChainDescription.SampleDesc.Quality = 0;
    SwapChainDescription.Windowed = 1;
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL d3d_feature_lvl;
    const D3D_FEATURE_LEVEL d3d_feature_array[ 2 ] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, d3d_feature_array, 2, D3D11_SDK_VERSION,
        &SwapChainDescription, &D3DSwapChain, &D3DDevice, &d3d_feature_lvl, &D3DDeviceContext );

    if ( FAILED( hr ) )
        return false;

    ID3D11Texture2D* pBackBuffer;
    hr = D3DSwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) );
    if ( FAILED( hr ) )
        return false;

    hr = D3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &D3DRenderTarget );
    pBackBuffer->Release( );
    if ( FAILED( hr ) )
        return false;

    IMGUI_CHECKVERSION( );
    ImGui::CreateContext( );
    ImGuiIO& io = ImGui::GetIO( );
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    io.Fonts->AddFontFromFileTTF( _( "C:\\Windows\\Fonts\\calibrib.ttf" ), 11.f );
    itx::fonts.Menu = io.Fonts->AddFontFromFileTTF( _( "C:\\Windows\\Fonts\\calibrib.ttf" ), 18.f );

    if ( !ImGui_ImplWin32_Init( hWindowHandle ) || !ImGui_ImplDX11_Init( D3DDevice, D3DDeviceContext ) )
        return false;

    return true;
}

auto LoadStyles( ) -> void
{
    ImGuiStyle* iStyle = &ImGui::GetStyle( );
    iStyle->WindowRounding = 8.f;
    iStyle->FrameRounding = 6.f;
    iStyle->WindowBorderSize = 0.f;
    iStyle->Colors[ ImGuiCol_WindowBg ] = ImColor( 16, 16, 16 );
    iStyle->Colors[ ImGuiCol_ChildBg ] = ImColor( 20, 20, 20 );
    iStyle->Colors[ ImGuiCol_Button ] = ImColor( 24, 24, 24 );
    iStyle->Colors[ ImGuiCol_ButtonHovered ] = ImColor( 24, 24, 24 );
    iStyle->Colors[ ImGuiCol_ButtonActive ] = ImColor( 24, 24, 24 );
    iStyle->Colors[ ImGuiCol_Border ] = ImColor( 15, 15, 15 );
}

const char* cHitboxes[ ] = { "Head", "Neck", "Chest", "Pelvis" };
const char* cBoxTypes[ ] = { "2D", "Cornered" };

bool b2D = true, bCornered;

bool bShowMenu = true;
int iMenuTab = 0;
auto itx::render_c::RenderMenu( ) -> void
{
    if ( GetAsyncKeyState( VK_INSERT ) & 1 )
        bShowMenu = !bShowMenu;

    if ( !bShowMenu )
        return;

    LoadStyles( );

    ImGui::PushFont( itx::fonts.Menu );
    ImGui::SetNextWindowSize( { 660, 440 } );
    ImGui::Begin( _( "Austins-Cheat" ), 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar );
    {
        ImGui::SetCursorPos( { 10, 10 } );
        ImGui::BeginChild( _( "Buttons" ), { 100, 420 } );
        {
            ImGui::SetCursorPos( { 20, 15 } );
            ImGui::TextColored( ImColor( 230, 230, 255 ), _( "Austin's\n  Cheat" ) );

            ImGui::SetCursorPos( { 15, 70 } );
            ImGui::PushStyleColor( ImGuiCol_Text, iMenuTab == 0 ? ImColor( 230, 230, 255 ).Value : ImColor( 180, 180, 180 ).Value );
            if ( ImGui::Button( _( "Aimbot" ), { 70, 70 } ) ) iMenuTab = 0;

            ImGui::SetCursorPos( { 15, 150 } );
            ImGui::PushStyleColor( ImGuiCol_Text, iMenuTab == 1 ? ImColor( 230, 230, 255 ).Value : ImColor( 180, 180, 180 ).Value );
            if ( ImGui::Button( _( "Trigger" ), { 70, 70 } ) ) iMenuTab = 1;

            ImGui::SetCursorPos( { 15, 230 } );
            ImGui::PushStyleColor( ImGuiCol_Text, iMenuTab == 2 ? ImColor( 230, 230, 255 ).Value : ImColor( 180, 180, 180 ).Value );
            if ( ImGui::Button( _( "Visuals" ), { 70, 70 } ) ) iMenuTab = 2;

            ImGui::SetCursorPos( { 15, 310 } );
            ImGui::PushStyleColor( ImGuiCol_Text, iMenuTab == 3 ? ImColor( 230, 230, 255 ).Value : ImColor( 180, 180, 180 ).Value );
            if ( ImGui::Button( _( "Extras" ), { 70, 70 } ) ) iMenuTab = 3;

            ImGui::PopStyleColor( 4 );
        }
        ImGui::EndChild( );

        ImGui::SetCursorPos( { 120, 10 } );
        ImGui::BeginChild( _( "Content" ), { 530, 420 } );
        {
            switch ( iMenuTab )
            {
            case 0:
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Aimbot" ), &itx::settings.bAimbot );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Prediction" ), &itx::settings.bPrediction );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Visible Check" ), &itx::settings.bVisibleCheck );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Humanization" ), &itx::settings.bHumanization );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Render FOV" ), &itx::settings.bRenderFOV );
                ImGui::Spacing( );
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "FOV Radius" ), &itx::settings.iFovRadius, 1, 180 );
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "Smooth" ), &itx::settings.iCustomDelay, 50, 300 );
                ImGui::Spacing( );
                ImGui::Combo( _( "Hitbox" ), &itx::settings.iHitBox, cHitboxes, IM_ARRAYSIZE( cHitboxes ), 4 );
                ImGui::Spacing( );
                ImGui::Spacing( );
                if ( itx::settings.bHumanization )
                {
                    ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "Mistake Size" ), &itx::settings.iMistakeSize, 1, 20 );
                    ImGui::Spacing( );
                    ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "Correction Delay" ), &itx::settings.iMistakeCorrection, 1, 20 );
                }
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::Keybind( _( "Keybind" ), &itx::settings.iAimbotKeybind, 0 );
                break;
            case 1:
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Triggerbot" ), &itx::settings.bTriggerbot );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Shotgun Only" ), &itx::settings.bTriggerbot );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Ignore Keybind" ), &itx::settings.bTriggerbot );
                ImGui::Spacing( );
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "Custom Delay" ), &itx::settings.iCustomDelay, 50, 300 );
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "Max Distance" ), &itx::settings.iMaxDistance, 5, 150 );
                ImGui::SetCursorPosX( 5 ); ImGui::Keybind( _( "Keybind" ), &itx::settings.iTriggerbotKeybind, 0 );
                break;
            case 2:
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Box" ), &itx::settings.bBox );
                if ( itx::settings.bBox )
                {
                    ImGui::SetCursorPosX( 15 ); if ( ImGui::Checkbox( _( "2D" ), &b2D ) ) { b2D = 1; bCornered = 0; }
                    ImGui::SetCursorPosX( 15 ); if ( ImGui::Checkbox( _( "Cornered" ), &bCornered ) ) { b2D = 0; bCornered = 1; }
                }
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Skeleton" ), &itx::settings.bSkeletons );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Usernames" ), &itx::settings.bUsername );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Distance" ), &itx::settings.bDistance );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Held Weapon" ), &itx::settings.bHeldWeapon );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Outlined Box" ), &itx::settings.bBoxOutline );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "Outlined Text" ), &itx::settings.bTextOutline );
                ImGui::Spacing( );
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "Box Thickness" ), &itx::settings.iBoxThickness, 1, 5 );
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::SliderInt( _( "Skeleton Thickness" ), &itx::settings.iSkeletonThickness, 1, 5 );

                ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, { 0, 0 } );
                ImGui::SetCursorPosX( 5 ); ImGui::Text( _( "Visible Color" ) );
                ImGui::SameLine( );
                ImGui::ColorEdit4( _( "##Visible Color" ), ( float* ) &itx::settings.iVisibleColor, ImGuiColorEditFlags_NoInputs );

                ImGui::SetCursorPosX( 5 ); ImGui::Text( _( "Invisible Color" ) );
                ImGui::SameLine( );
                ImGui::ColorEdit4( _( "##Invisible Color" ), ( float* ) &itx::settings.iInvisibleColor, ImGuiColorEditFlags_NoInputs );
                ImGui::PopStyleVar( );
                break;
            case 3:
                ImGui::Spacing( );
                ImGui::SetCursorPosX( 5 ); ImGui::Checkbox( _( "vSync" ), &itx::settings.bvSync );

                ImGui::SetCursorPosX( 5 );
                if ( ImGui::Checkbox( _( "Stream Proof" ), &itx::settings.bStreamProof ) )
                    itx::settings.bStreamProofFlip = true;

                break;
            }
        }
        ImGui::EndChild( );
    }
    ImGui::End( );
    ImGui::PopFont( );
}
auto itx::render_c::RenderThread( ) -> void
{
    ImVec4 vClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    const float fClearColor[ 4 ] = { vClearColor.x * vClearColor.w, vClearColor.y * vClearColor.w, vClearColor.z * vClearColor.w, vClearColor.w };
    auto& io = ImGui::GetIO( );

    SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_HIGHEST );

    uintptr_t pOffset = 0x0;

    for ( ;; )
    {
        MSG msg;
        while ( PeekMessage( &msg, hWindowHandle, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );

            if ( msg.message == WM_QUIT )
                return;
        }

        io.DeltaTime = 1.0f / 165.f;
        io.ImeWindowHandle = hWindowHandle;

        POINT p_cursor;
        GetCursorPos( &p_cursor );
        io.MousePos.x = p_cursor.x;
        io.MousePos.y = p_cursor.y;

        io.MouseDown[ 0 ] = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
        io.MouseClicked[ 0 ] = io.MouseDown[ 0 ];

        io.WantCaptureMouse = io.WantCaptureKeyboard = io.WantCaptureMouse || io.WantCaptureKeyboard;

        ImGui_ImplDX11_NewFrame( );
        ImGui_ImplWin32_NewFrame( );
        ImGui::NewFrame( );
        {
            itx::visuals.ActorLoop( );

            if ( itx::settings.bRenderFOV )
                ImGui::GetBackgroundDrawList( )->AddCircle( ImVec2( itx::screen.fWidth / 2, itx::screen.fHeight / 2 ), itx::settings.iFovRadius * 10, ImColor( 255, 255, 255 ), 64, 1.f );

            this->RenderMenu( );
        }
        ImGui::Render( );
        D3DDeviceContext->OMSetRenderTargets( 1, &D3DRenderTarget, nullptr );
        D3DDeviceContext->ClearRenderTargetView( D3DRenderTarget, fClearColor );
        ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

        int ivSync = itx::settings.bvSync ? 1 : 0;
        D3DSwapChain->Present( ivSync, 0 );
    }

    ImGui_ImplDX11_Shutdown( );
    ImGui_ImplWin32_Shutdown( );
    ImGui::DestroyContext( );

    if ( D3DRenderTarget )
        D3DRenderTarget->Release( );

    if ( D3DSwapChain )
        D3DSwapChain->Release( );

    if ( D3DDeviceContext )
        D3DDeviceContext->Release( );

    DestroyWindow( hWindowHandle );
}