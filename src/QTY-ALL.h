#pragma once
#include "QTY-BASE.h"

// Hold to move the whole stack. Accepts SkyUI's quantity prompt for you if it opens anyway
class AllAction final : public QtyAction
{
public:
	static AllAction* GetSingleton()
	{
		static AllAction s;
		return &s;
	}

protected:
	int  GetOverrideCount() const override { return 1; }
	void ProcessExtraLogic(RE::GFxMovieView* a_movie, bool a_hotkeyDown) override;
	void OnReset() override { _handledCurrentPrompt = false; }

private:
	bool _handledCurrentPrompt = false;

	AllAction() :
		QtyAction({ "ALL_Key", "ALL_GPKey",
			44, 0,  // Z, gamepad unbound
			WidgetType::kAll })
	{}
};
