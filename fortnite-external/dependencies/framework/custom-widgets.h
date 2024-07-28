#include <Windows.h>
#include <iostream>
#include <map>
#include "imgui.h"

using namespace ImGui;

namespace ImGui
{
    const char* keys[ ] =
    {
        "None",
        "Mouse 1",
        "Mouse 2",
        "CN",
        "Mouse 3",
        "Mouse 4",
        "Mouse 5",
        "-",
        "Back",
        "Tab",
        "-",
        "-",
        "CLR",
        "Enter",
        "-",
        "-",
        "Shift",
        "CTL",
        "Menu",
        "Pause",
        "Caps",
        "KAN",
        "-",
        "JUN",
        "FIN",
        "KAN",
        "-",
        "Escape",
        "CON",
        "NCO",
        "ACC",
        "MAD",
        "Space",
        "PGU",
        "PGD",
        "End",
        "Home",
        "Left",
        "Up",
        "Right",
        "Down",
        "SEL",
        "PRI",
        "EXE",
        "PRI",
        "INS",
        "Delete",
        "HEL",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "WIN",
        "WIN",
        "APP",
        "-",
        "SLE",
        "Num 0",
        "Num 1",
        "Num 2",
        "Num 3",
        "Num 4",
        "Num 5",
        "Num 6",
        "Num 7",
        "Num 8",
        "Num 9",
        "MUL",
        "ADD",
        "SEP",
        "MIN",
        "Delete",
        "DIV",
        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",
        "F13",
        "F14",
        "F15",
        "F16",
        "F17",
        "F18",
        "F19",
        "F20",
        "F21",
        "F22",
        "F23",
        "F24",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "NUM",
        "SCR",
        "EQU",
        "MAS",
        "TOY",
        "OYA",
        "OYA",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "-",
        "Shift",
        "Shift",
        "Ctrl",
        "Ctrl",
        "Alt",
        "Alt"
    };

    ImU32 GetColor( const ImVec4& col, float alpha )
    {
        ImGuiStyle& style = GImGui->Style;
        ImVec4 c = col;
        c.w *= style.Alpha * alpha;
        return ColorConvertFloat4ToU32( c );
    }

    static float CalcMaxPopupHeightFromItemCount( int items_count, float item_size )
    {
        ImGuiContext& g = *GImGui;
        if ( items_count <= 0 )
            return FLT_MAX;
        return item_size * items_count + g.Style.ItemSpacing.y * ( items_count - 1 );
    }

    bool SelectableEx( const char* label, bool active )
    {
        ImGuiWindow* window = GetCurrentWindow( );
        if ( window->SkipItems )
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID( label );

        const float width = GetWindowWidth( );
        const ImVec2 pos = window->DC.CursorPos;
        const ImRect rect( pos, pos + ImVec2( width, 24 ) );
        ItemSize( rect, style.FramePadding.y );
        if ( !ItemAdd( rect, id ) )
            return false;

        bool hovered = IsItemHovered( );
        bool pressed = hovered && g.IO.MouseClicked[ 0 ];
        if ( pressed )
            MarkItemEdited( id );

        static std::map<ImGuiID, ImVec4> anim;
        ImVec4& state = anim[ id ];

        state = ImLerp( state, active ? ImColor( 255, 255, 255 ) : ImColor( 180, 180, 180 ), 4.f );

        window->DrawList->AddText( rect.Min + ImVec2( 5, 5 ), GetColor( state, 1.0f ), label );

        IMGUI_TEST_ENGINE_ITEM_INFO( id, label, g.LastItemData.StatusFlags );
        return pressed;
    }

    bool Selectable( const char* label, bool* p_selected )
    {
        if ( SelectableEx( label, *p_selected ) )
        {
            *p_selected = !*p_selected;
            return true;
        }
        return false;
    }

    bool Keybind( const char* label, int* key, int* mode )
    {
        struct key_state
        {
            bool active = false;
            bool hovered = false;
            float alpha = 0.f;
        };

        ImGuiWindow* window = ImGui::GetCurrentWindow( );
        if ( window->SkipItems )
            return false;

        ImGuiContext& g = *GImGui;
        ImGuiIO& io = g.IO;
        const ImGuiStyle& style = g.Style;

        const ImGuiID id = window->GetID( label );

        const ImVec2 pos = window->DC.CursorPos;
        const float width = GetWindowWidth( ) / 3;
        const ImRect rect( pos, pos + ImVec2( width, 40 ) );
        ImGui::ItemSize( rect, style.FramePadding.y );
        if ( !ImGui::ItemAdd( rect, id, &rect ) )
            return false;

        static std::map<ImGuiID, key_state> anim;
        key_state& state = anim[ id ];

        char buf_display[ 64 ] = "Select";

        bool value_changed = false;
        int k = *key;

        std::string active_key = "";
        active_key += keys[ *key ];

        if ( *key != 0 && g.ActiveId != id ) {
            strcpy_s( buf_display, active_key.c_str( ) );
        }
        else if ( g.ActiveId == id ) {
            strcpy_s( buf_display, "Select" );
        }

        bool hovered = ItemHoverable( rect, id, 0 );

        if ( hovered && GetAsyncKeyState( VK_LBUTTON ) )
        {
            if ( g.ActiveId != id ) {
                // Start edition
                memset( io.MouseDown, 0, sizeof( io.MouseDown ) );
                memset( io.KeysDown, 0, sizeof( io.KeysDown ) );
                *key = 0;
            }
            ImGui::SetActiveID( id, window );
            ImGui::FocusWindow( window );
        }
        else if ( GetAsyncKeyState( VK_LBUTTON ) ) {
            // Release focus when we click outside
            if ( g.ActiveId == id )
                ImGui::ClearActiveID( );
        }

        if ( g.ActiveId == id ) {
            if ( !value_changed ) {
                for ( auto i = 0x00; i <= 0xA5; i++ ) {
                    if ( GetAsyncKeyState( i ) & 1 ) {
                        k = i;
                        value_changed = true;
                        ImGui::ClearActiveID( );
                    }
                }
            }

            if ( IsKeyPressedMap( ImGuiKey_Escape ) ) {
                *key = 0;
                ImGui::ClearActiveID( );
            }
            else {
                *key = k;
            }
        }

        const float buf_width = CalcTextSize( buf_display ).x;

        window->DrawList->AddRectFilled( rect.Min, rect.Max, ImColor( 30, 30, 30 ), 3.f );
        window->DrawList->AddText( rect.Min + ImVec2( 12, 13 ), ImColor( 255, 255, 255 ), label );
        window->DrawList->AddRectFilled( ImVec2( rect.Max.x - 22 - buf_width, rect.Min.y + 8 ), rect.Max - ImVec2( 12, 8 ), ImColor( 30, 30, 30 ), 4.f, state.active ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersAll );

        RenderTextClipped( ImVec2( rect.Max.x - 22 - buf_width, rect.Min.y + 7 ), rect.Max - ImVec2( 12, 9 ), buf_display, NULL, NULL, ImVec2( 0.5f, 0.5f ) );

        if ( hovered && g.IO.MouseClicked[ 1 ] || state.active && ( g.IO.MouseClicked[ 0 ] || g.IO.MouseClicked[ 1 ] ) && !state.hovered )
            state.active = !state.active;

        state.alpha = ImClamp( state.alpha + ( 8.f * g.IO.DeltaTime * ( state.active ? 1.f : -1.f ) ), 0.f, 1.f );

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing;

        if ( state.alpha >= 0.01f )
        {
            PushStyleVar( ImGuiStyleVar_Alpha, state.alpha );
            PushStyleVar( ImGuiStyleVar_WindowRounding, 4.f );
            PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
            PushStyleColor( ImGuiCol_WindowBg, ImColor( 30, 30, 30 ).Value );
            SetNextWindowSize( ImVec2( 100, CalcMaxPopupHeightFromItemCount( 3, 24.f ) ) );
            SetNextWindowPos( ImRect( ImVec2( rect.Max.x - 22 - buf_width, rect.Min.y + 8 ), rect.Max - ImVec2( 12, 8 ) ).GetCenter( ) - ImVec2( 100 / 2, -12 ) );

            Begin( label, NULL, window_flags );
            {
                if ( SelectableEx( "Hold", *mode == 0 ) )
                {
                    *mode = 0;
                    state.active = false;
                }
                if ( SelectableEx( "Toggle", *mode == 1 ) )
                {
                    *mode = 1;
                    state.active = false;
                }
                if ( SelectableEx( "Always", *mode == 2 ) )
                {
                    *mode = 2;
                    state.active = false;
                }
            }
            End( );
            PopStyleVar( 3 );
            PopStyleColor( );
        }

        return value_changed;
    }
}