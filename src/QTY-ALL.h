#pragma once
#include "QTY-BASE.h"

class AllWidget : public QtyWidgetBase
{
public:
	static AllWidget* GetSingleton()
	{
		static AllWidget s;
		return &s;
	}

protected:
	int         GetOverrideCount() const override { return 1; }
	const char* GetCustomLabel() const override { return SharedWidgetAppearance::GetSingleton()._cfg.customAllLabel; }

	void ProcessExtraLogic(RE::UI* ui, bool hotkeyDown) override;
	void OnMenuStateReset() override { _handledCurrentPrompt = false; }

private:
	bool _handledCurrentPrompt = false;

	AllWidget() :
		QtyWidgetBase({ "$QTY_All_Title",
			"ALL_Widget",
			"ALL_Key", "ALL_GPKey",
		  	44, 0,  // Z is 44, Gamepad mapped offset is 0 (unmapped)
			{ 200.0f, 100.0f },
			{ 182.0f, -12.0f },
			"$QTY_AllButton",
			"QTY_All_Context",
			"$QTY_ConfigTitle",
			WidgetType::kAll })
	{
		QtySharedHotkeys::GetSingleton().all = &_hotkey;
	}
};
