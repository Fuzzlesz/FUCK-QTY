#pragma once
#include "QTY-BASE.h"

// Hold to move one item at a time, skipping the quantity prompt
class OneAction final : public QtyAction
{
public:
	static OneAction* GetSingleton()
	{
		static OneAction s;
		return &s;
	}

protected:
	int GetOverrideCount() const override { return kBigCount; }

private:
	// Sentinel
	static constexpr int kBigCount = -1333444555;

	OneAction() :
		QtyAction({ "ONE_Key", "ONE_GPKey",
			29, 0,  // Left Ctrl, gamepad unbound
			WidgetType::kOne })
	{}
};
