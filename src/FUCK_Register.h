#pragma once

#include "QTY-MENUS.h"
#include "QTY-SETTINGS.h"
#include "QTY-TOOL.h"

namespace FUCK_Register
{
	inline void Install()
	{
		// Settings and hotkeys first, so they're right from the first menu
		QtySettings::GetSingleton().Load();

		QtyMenus::Install();

		FUCK::RegisterTool(QTYTool::GetSingleton());
	}
}
