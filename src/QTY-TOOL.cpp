#include "QTY-TOOL.h"
#include "QTY-ACTIONS.h"

bool QTYTool::OnAsyncInput(const void* e)
{
	bool handled = false;
	for (auto* action : QtyActions()) {
		auto& hotkey = action->GetHotkey();
		if (hotkey.isBinding && FUCK::UpdateManagedHotkey(e, hotkey))
			handled = true;
	}

	if (handled)
		QtySettings::GetSingleton().SaveKeybinds();
	return handled;
}

void QTYTool::Draw()
{
	auto& settings = QtySettings::GetSingleton();

	bool saveSettings = false;
	bool saveKeybinds = false;

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_ConfigTitle"_T);
	FUCK::Separator();
	FUCK::Spacing();

	auto drawVisConfig = [&](const char* title, VisibilityConfig& vis, const char* tableId) {
		if (!FUCK::TreeNode(title))
			return;

		if (FUCK::Checkbox("$QTY_HideHint"_T, &vis.hideHint))
			saveSettings = true;
		FUCK::SetTooltip("$QTY_HideHintTip"_T);

		FUCK::Dummy(FUCK::UIScale(0.0f, 5.0f));
		if (FUCK::BeginTable(tableId, 2, FUCK::TableFlags::kSizingStretchSame)) {
			// Column 1: KBM
			FUCK::TableNextColumn();
			FUCK::PushID("KBM");
			FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_KBM"_T);
			FUCK::Dummy(FUCK::UIScale(0.0f, 5.0f));
			if (FUCK::Checkbox("$QTY_ContainerMenu"_T, &vis.showInContainerKBM, false, false))
				saveSettings = true;
			if (FUCK::Checkbox("$QTY_BarterMenu"_T, &vis.showInBarterKBM, false, false))
				saveSettings = true;
			if (FUCK::Checkbox("$QTY_GiftMenu"_T, &vis.showInGiftKBM, false, false))
				saveSettings = true;
			if (FUCK::Checkbox("$QTY_InventoryMenu"_T, &vis.showInInventoryKBM, false, false))
				saveSettings = true;
			FUCK::PopID();

			// Column 2: Controller
			FUCK::TableNextColumn();
			FUCK::PushID("GP");
			FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_Controller"_T);
			FUCK::Dummy(FUCK::UIScale(0.0f, 5.0f));
			if (FUCK::Checkbox("$QTY_ContainerMenu"_T, &vis.showInContainerGP, false, false))
				saveSettings = true;
			if (FUCK::Checkbox("$QTY_BarterMenu"_T, &vis.showInBarterGP, false, false))
				saveSettings = true;
			if (FUCK::Checkbox("$QTY_GiftMenu"_T, &vis.showInGiftGP, false, false))
				saveSettings = true;
			if (FUCK::Checkbox("$QTY_InventoryMenu"_T, &vis.showInInventoryGP, false, false))
				saveSettings = true;
			FUCK::PopID();

			FUCK::EndTable();
		}
		FUCK::TreePop();
	};

	drawVisConfig("$QTY_VisibilityOne"_T, settings._cfg.oneVis, "VisTableOne");
	drawVisConfig("$QTY_VisibilityAll"_T, settings._cfg.allVis, "VisTableAll");

	FUCK::Dummy(FUCK::UIScale(0.0f, 15.0f));

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_Keybinds"_T);
	FUCK::Separator();
	FUCK::Spacing();

	// Bound straight onto the actions. ProcessInput() finishes the binding and saves it
	auto* one = OneAction::GetSingleton();
	auto* all = AllAction::GetSingleton();
	FUCK::DrawManagedHotkey(settings.GetLabel(ActionType::kOne), one->GetHotkey(), FUCK::HotkeyFlags::kNoModifiers);
	FUCK::DrawManagedHotkey(settings.GetLabel(ActionType::kAll), all->GetHotkey(), FUCK::HotkeyFlags::kNoModifiers);

	FUCK::Dummy(FUCK::UIScale(0.0f, 15.0f));

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_HintLabels"_T);
	FUCK::Separator();
	FUCK::Spacing();

	FUCK::InputText("$QTY_LabelOne"_T, settings._cfg.customOneLabel, sizeof(settings._cfg.customOneLabel));
	if (FUCK::IsItemDeactivatedAfterEdit())
		saveSettings = true;

	FUCK::InputText("$QTY_LabelAll"_T, settings._cfg.customAllLabel, sizeof(settings._cfg.customAllLabel));
	if (FUCK::IsItemDeactivatedAfterEdit())
		saveSettings = true;

	FUCK::Dummy(FUCK::UIScale(0.0f, 15.0f));
	FUCK::Separator();
	FUCK::Spacing();

	if (FUCK::Button("$QTY_Unmap"_T)) {
		for (auto* action : QtyActions())
			action->GetHotkey().Clear();
		saveKeybinds = true;
	}

	FUCK::SameLine();

	if (FUCK::Button("$QTY_ResetToDefault"_T)) {
		for (auto* action : QtyActions())
			action->ResetKeybinds();
		saveKeybinds = true;
	}

	if (saveSettings)
		settings.Save();
	if (saveKeybinds)
		settings.SaveKeybinds();
}
