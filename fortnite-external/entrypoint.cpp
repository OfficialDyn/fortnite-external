#include <core/core.hpp>

 auto main() -> int
 {
	if (!itx::core.Initialize())
	{
		MessageBoxA(GetForegroundWindow(), _("core error whilst initializing.\ncheck console output for error."), _("core error"), MB_OK | MB_ICONERROR);
		std::cin.get();
	}
}