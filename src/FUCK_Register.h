#pragma once

#include "QTY-MENUS.h"
#include "QTY-SHARED.h"

class QTYTool : public FUCK::ITool
{
public:
	static QTYTool* GetSingleton()
	{
		static QTYTool s;
		return &s;
	}

	const char* Name() const override { return "$QTY_ConfigTitle"_T; }

	void Draw() override
	{
		QtyToolManager::GetSingleton()->RenderToolUI();
	}

	bool OnAsyncInput(const void* e) override
	{
		return QtyToolManager::GetSingleton()->ProcessInput(e);
	}
};

namespace FUCK_Register
{
	inline void Install()
	{
		QtySettings::GetSingleton().Load();

		QtyMenus::Install();

		FUCK::RegisterTool(QTYTool::GetSingleton());
	}
}
