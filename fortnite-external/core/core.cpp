#include "core.hpp"

//Made By Dyn.

auto itx::core_c::Initialize( ) -> bool
{
	if ( !itx::screen.GetScreenSize( ) )
	{
		itx::Interface.Print( _( "(!) failed to get screen size.\n" ) );
		return false;
	}

	if ( !itx::memory.initialize_handle( ) )
	{
		itx::Interface.Print( _( "(!) failed to establish a connection with the kernel module.\n" ) );
		return false;
	}

	auto iPid = itx::memory.get_process_pid( static_cast< std::wstring >( _( L"FortniteClient-Win64-Shipping.exe" ) ) );
	if ( !itx::memory.attach( iPid ) )
	{
		itx::Interface.Print( _( "(!) failed to find the target process.\n" ) );
		return false;
	}
	itx::Interface.Print( _( "(+) process pid: " ), iPid );

	if ( !itx::memory.get_image_base( nullptr ) )
	{
		itx::Interface.Print( _( "(!) failed to find the target's base address.\n" ) );
		return false;
	}
	itx::Interface.Print( _( "(+) base address: " ), itx::memory.image_base );

	if ( !itx::memory.get_cr3( itx::memory.image_base ) )
	{
		itx::Interface.Print( _( "(!) failed to resolve process cr3.\n" ) );
		return false;
	}
	itx::Interface.Print( _( "(+) cr3: " ), itx::memory.dtb );

	if ( !itx::memory.get_text_section( ) )
	{
		itx::Interface.Print( _( "(!) failed to get .text section.\n" ) );
		return false;
	}
	itx::Interface.Print( _( "(+) .text section: " ), itx::memory.text_section );

	std::thread( [ & ]( ) { itx::cache.Data( ); } ).detach( );
	std::thread( [ & ]( ) { itx::cache.Entities( ); } ).detach( );
	std::thread( [ & ]( ) { itx::combat.CombatThread( ); } ).detach( );

	if ( !itx::render.Initialize( ) )
	{
		itx::Interface.Print( _( "(!) failed to initialize dx11.\n" ) );
		return false;
	}
	else
		itx::render.RenderThread( );

	std::cin.get( );
}