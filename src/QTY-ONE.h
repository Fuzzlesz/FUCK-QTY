#pragma once
#include "QTY-BASE.h"

class OneWidget : public QtyWidgetBase
{
public:
	static OneWidget* GetSingleton()
	{
		static OneWidget s;
		return &s;
	}

protected:
	int         GetOverrideCount() const override { return kBigCount; }
	const char* GetCustomLabel() const override { return SharedWidgetAppearance::GetSingleton()._cfg.customOneLabel; }

private:
	// Sentinel
	static constexpr int kBigCount = -1333444555;

	OneWidget() :
		QtyWidgetBase({ "$QTY_One_Title",
			"ONE_Widget",
			"ONE_Key", "ONE_GPKey",
			29, 0,  // Ctrl is 29, Gamepad mapped offset is 0 (unmapped)
			{ 100.0f, 100.0f },
			{ 1.22f, -12.0f },
			"$QTY_OneButton",
			"QTY_One_Context",
			"$QTY_ConfigTitle",
			WidgetType::kOne })
	{
		QtySharedHotkeys::GetSingleton().one = &_hotkey;
	}
};
