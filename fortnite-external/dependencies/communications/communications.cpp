#ifndef DRIVER_CPP
#define DRIVER_CPP

#include <Windows.h>
#include <iostream>
#include <vector>
#include <TlHelp32.h>
#include "communications.hpp"

auto itx::memory_c::get_process_pid(
	const std::wstring& proc_name ) -> const std::uint32_t
{
	PROCESSENTRY32 proc_info;
	proc_info.dwSize = sizeof( proc_info );

	HANDLE proc_snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );
	if ( proc_snapshot == INVALID_HANDLE_VALUE ) {
		return 0;
	}

	Process32First( proc_snapshot, &proc_info );
	if ( !wcscmp( proc_info.szExeFile, proc_name.c_str( ) ) )
	{
		CloseHandle( proc_snapshot );
		this->m_pid = proc_info.th32ProcessID;
		return this->m_pid;
	}

	while ( Process32Next( proc_snapshot, &proc_info ) )
	{
		if ( !wcscmp( proc_info.szExeFile, proc_name.c_str( ) ) )
		{
			CloseHandle( proc_snapshot );
			this->m_pid = proc_info.th32ProcessID;
			return this->m_pid;
		}
	}

	CloseHandle( proc_snapshot );
	return 0;
}

auto itx::memory_c::send_cmd(
	void* data,
	requests code ) -> bool
{
	if ( !data || !code )
		return false;

	IO_STATUS_BLOCK block;
	invoke_data request { 0 };

	request.unique = requests::invoke_unique;
	request.data = data;
	request.code = code;

	auto result =
		direct_device_control(
		this->m_handle, 
		nullptr, 
		nullptr,
		nullptr, 
		&block, 
		0,
		&request,
		sizeof( request ), 
		&request, 
		sizeof( request ) );

	return result;
}

auto itx::memory_c::initialize_handle( ) -> bool
{
	this->m_handle = CreateFileA( device_name, GENERIC_READ, 0, 0, 3, 0x00000080, 0 );
	if ( this->m_handle != INVALID_HANDLE_VALUE ) {
		return true;
	}
	return false;
}

auto itx::memory_c::attach(
	int a_pid ) -> bool
{
	if ( !a_pid )
		return false;

	this->m_pid = a_pid;

	return true;
}

auto itx::memory_c::get_image_base(
	const char* module_name ) -> const std::uintptr_t
{
	base_invoke data { 0 };
	
	data.pid = this->m_pid;
	data.handle = 0;
	data.name = module_name;

	this->send_cmd(
		&data,
		invoke_base );

	this->image_base = data.handle;
	return this->image_base;
}

auto itx::memory_c::write_virtual(
	const std::uintptr_t address, 
	void* buffer,
	const std::size_t size ) -> bool
{
	write_invoke data { 0 };

	data.pid = this->m_pid;
	data.address = address;
	data.buffer = buffer;
	data.size = size;

	auto result = 
		this->send_cmd( 
			&data, 
			invoke_write );

	return result;
}

auto itx::memory_c::read_virtual(
	const std::uintptr_t address, 
	void* buffer,
	const std::size_t size ) -> bool
{
	read_invoke data { 0 };

	data.pid = this->m_pid;
	data.dtb = this->dtb;
	data.address = address;
	data.buffer = buffer;
	data.size = size;

	auto result = 
		this->send_cmd( 
			&data, 
			invoke_read );
	
	return result;
}

auto itx::memory_c::read_kernel(
	const uintptr_t address,
	void* buffer,
	const size_t size,
	const uint32_t memory_type ) -> bool
{

	read_kernel_invoke data { 0 };

	data.address = address;
	data.buffer = buffer;
	data.size = size;
	data.memory_type = memory_type;

	auto result =
		this->send_cmd(
			&data,
			invoke_read_kernel 
		);

	return result;
}

auto itx::memory_c::translate_address(
	std::uintptr_t virtual_address,
	std::uintptr_t directory_base ) -> const std::uintptr_t
{
	translate_invoke data { 0 };

	data.virtual_address = virtual_address;
	data.directory_base = directory_base;
	data.physical_address = nullptr;

	this->send_cmd(
		&data,
		invoke_translate 
	);

	return reinterpret_cast<std::uintptr_t>(data.physical_address);
}

auto itx::memory_c::get_dtb(
	std::uint32_t pid ) -> const std::uintptr_t
{
	dtb_invoke data { 0 };

	data.pid = pid;
	data.dtb = 0;

	auto result =
		this->send_cmd(
			&data,
			invoke_dtb
		);

	return data.dtb;
}

auto itx::memory_c::get_cr3(
	std::uintptr_t base_address ) -> bool
{
	auto ntdll_address = reinterpret_cast< std::uintptr_t >(GetModuleHandleA( "ntdll.dll" ));
	if ( !ntdll_address ) {
		return false;
	}

	auto current_dtb = this->get_dtb( GetCurrentProcessId( ) );
	if ( !current_dtb ) {
		return false;
	}

	auto nt_dll_physical = this->translate_address(
		ntdll_address,
		current_dtb
	);

	for ( std::uintptr_t i = 0; i != 0x50000000; i++ )
	{
		std::uintptr_t dtb = i << 12;

		if ( dtb == current_dtb )
			continue;

		auto phys_address = this->translate_address(
			ntdll_address,
			dtb
		);

		if ( !phys_address )
			continue;

		if ( phys_address == nt_dll_physical )
		{
			this->dtb = dtb;

			const auto bytes = this->read<char>( base_address );
			if ( bytes == 0x4D )
			{
				this->dtb = dtb;
				break;
			}
		}
	}

	FreeLibrary( reinterpret_cast< HMODULE >(ntdll_address) );
	return true;
}
	 
#endif // ! DRIVER_CPP
