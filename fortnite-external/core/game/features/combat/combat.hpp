#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>

namespace itx
{
	class combat_c
	{
	public:
		auto TriggerBot( ) -> void;
		auto CombatThread( ) -> void;
	};
	inline combat_c combat;
}