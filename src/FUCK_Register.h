#pragma once

#include "QTY-ALL.h"
#include "QTY-ONE.h"

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
		auto* one = OneWidget::GetSingleton();
		auto* all = AllWidget::GetSingleton();
		one->Initialize();
		all->Initialize();

		FUCK::RegisterWindow(one);
		FUCK::RegisterWindow(all);

		FUCK::RegisterTool(QTYTool::GetSingleton());
	}
}
