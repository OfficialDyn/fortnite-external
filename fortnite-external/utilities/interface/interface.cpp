#include "interface.hpp"

auto itx::screen_c::GetScreenSize( ) -> bool
{
	this->fWidth = GetSystemMetrics( SM_CXSCREEN );
	this->fHeight = GetSystemMetrics( SM_CYSCREEN );
	return ( this->fWidth != 0 && this->fHeight != 0 );
}