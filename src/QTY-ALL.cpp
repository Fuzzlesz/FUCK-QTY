#include "QTY-ALL.h"

void AllAction::ProcessExtraLogic(RE::GFxMovieView* a_movie, bool a_hotkeyDown)
{
	// Whether SkyUI's quantity slider is up
	RE::GFxValue itemCard;
	if (!a_movie->GetVariable(&itemCard, "_root.Menu_mc.itemCard") || !itemCard.IsObject()) {
		_handledCurrentPrompt = false;
		return;
	}

	bool         promptVisible = false;
	RE::GFxValue quantitySlider;
	if (itemCard.GetMember("QuantitySlider_mc", &quantitySlider) && quantitySlider.IsObject()) {
		RE::GFxValue alpha;
		promptVisible = quantitySlider.GetMember("_alpha", &alpha) && alpha.IsNumber() && alpha.GetNumber() > 0.0;
	}

	if (!promptVisible) {
		_handledCurrentPrompt = false;
		return;
	}

	if (!a_hotkeyDown || _handledCurrentPrompt)
		return;

	_handledCurrentPrompt = true;

	// The payload SkyUI's ItemCard.HandleQuantityMenuInput expects for "Accept", the same on KBM and controller
	RE::GFxValue details;
	a_movie->CreateObject(&details);
	details.SetMember("value", "keyDown");

	RE::GFxValue enterCode;
	if (a_movie->GetVariable(&enterCode, "gfx.ui.NavigationCode.ENTER"))
		details.SetMember("navEquivalent", enterCode);
	else
		details.SetMember("navEquivalent", "enter");

	RE::GFxValue args[1]{ details };
	itemCard.Invoke("HandleQuantityMenuInput", nullptr, args, 1);
}
