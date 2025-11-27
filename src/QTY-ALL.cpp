#include "QTY-ALL.h"

void AllWidget::ProcessExtraLogic(RE::UI* ui, bool hotkeyDown)
{
	// Detect whether SkyUI's quantity slider is currently visible.
	// If it is, we handle acceptance ourselves rather than letting the base logic do it.
	bool isSkyUIPromptVisible = false;

	auto m = ui->GetMenu(_activeMenu);
	if (m && m->uiMovie) {
		RE::GFxValue itemCard;
		if (m->uiMovie->GetVariable(&itemCard, "_root.Menu_mc.itemCard") && itemCard.IsObject()) {
			RE::GFxValue quantitySlider;
			if (itemCard.GetMember("QuantitySlider_mc", &quantitySlider) && quantitySlider.IsObject()) {
				RE::GFxValue alpha;
				if (quantitySlider.GetMember("_alpha", &alpha) && alpha.IsNumber() && alpha.GetNumber() > 0.0) {
					isSkyUIPromptVisible = true;
				}
			}
		}
	}

	if (!isSkyUIPromptVisible) {
		_handledCurrentPrompt = false;
	} else if (hotkeyDown && !_handledCurrentPrompt) {
		_handledCurrentPrompt = true;

		// We directly construct the payload Expected by SkyUI's ActionScript ItemCard.HandleQuantityMenuInput.
		// This flawlessly replicates the "Accept" prompt across KBM and Controller mappings.
		RE::GFxValue itemCard;
		if (m->uiMovie->GetVariable(&itemCard, "_root.Menu_mc.itemCard") && itemCard.IsObject()) {
			RE::GFxValue details;
			m->uiMovie->CreateObject(&details);
			details.SetMember("value", "keyDown");

			RE::GFxValue enterCode;
			if (m->uiMovie->GetVariable(&enterCode, "gfx.ui.NavigationCode.ENTER")) {
				details.SetMember("navEquivalent", enterCode);
			} else {
				// Fallback to literal if gfx isn't accessible in root
				details.SetMember("navEquivalent", "enter");
			}

			RE::GFxValue args[1]{ details };
			itemCard.Invoke("HandleQuantityMenuInput", nullptr, args, 1);
		}
	}
}
