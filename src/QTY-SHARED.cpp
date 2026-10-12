#include "QTY-SHARED.h"
#include "QTY-ALL.h"
#include "QTY-ONE.h"

namespace
{
	void LoadVis(CSimpleIniA& ini, const char* sec, VisibilityConfig& vis, const VisibilityConfig& def)
	{
		vis.hideWidget         = FUCK::INI::LoadBool(ini, sec, "HideWidget",         def.hideWidget);
		vis.showInContainerKBM = FUCK::INI::LoadBool(ini, sec, "ShowInContainerKBM", def.showInContainerKBM);
		vis.showInBarterKBM    = FUCK::INI::LoadBool(ini, sec, "ShowInBarterKBM",    def.showInBarterKBM);
		vis.showInGiftKBM      = FUCK::INI::LoadBool(ini, sec, "ShowInGiftKBM",      def.showInGiftKBM);
		vis.showInInventoryKBM = FUCK::INI::LoadBool(ini, sec, "ShowInInventoryKBM", def.showInInventoryKBM);

		vis.showInContainerGP = FUCK::INI::LoadBool(ini, sec, "ShowInContainerGP",   def.showInContainerGP);
		vis.showInBarterGP    = FUCK::INI::LoadBool(ini, sec, "ShowInBarterGP",      def.showInBarterGP);
		vis.showInGiftGP      = FUCK::INI::LoadBool(ini, sec, "ShowInGiftGP",        def.showInGiftGP);
		vis.showInInventoryGP = FUCK::INI::LoadBool(ini, sec, "ShowInInventoryGP",   def.showInInventoryGP);
	}

	void SaveVis(CSimpleIniA& ini, const char* sec, const VisibilityConfig& vis, const VisibilityConfig& def)
	{
		FUCK::INI::SaveBool(ini, sec, "HideWidget",         vis.hideWidget,          def.hideWidget);
		FUCK::INI::SaveBool(ini, sec, "ShowInContainerKBM", vis.showInContainerKBM,  def.showInContainerKBM);
		FUCK::INI::SaveBool(ini, sec, "ShowInBarterKBM",    vis.showInBarterKBM,     def.showInBarterKBM);
		FUCK::INI::SaveBool(ini, sec, "ShowInGiftKBM",      vis.showInGiftKBM,       def.showInGiftKBM);
		FUCK::INI::SaveBool(ini, sec, "ShowInInventoryKBM", vis.showInInventoryKBM,  def.showInInventoryKBM);

		FUCK::INI::SaveBool(ini, sec, "ShowInContainerGP",  vis.showInContainerGP,   def.showInContainerGP);
		FUCK::INI::SaveBool(ini, sec, "ShowInBarterGP",     vis.showInBarterGP,      def.showInBarterGP);
		FUCK::INI::SaveBool(ini, sec, "ShowInGiftGP",       vis.showInGiftGP,        def.showInGiftGP);
		FUCK::INI::SaveBool(ini, sec, "ShowInInventoryGP",  vis.showInInventoryGP,   def.showInInventoryGP);
	}

	std::array<QtyAction*, 2> Actions()
	{
		return { OneAction::GetSingleton(), AllAction::GetSingleton() };
	}
}

// ==================================================
// QtySettings
// ==================================================

bool QtySettings::ShowsHint(std::string_view a_menuName, WidgetType a_type, bool a_gamepad) const
{
	const auto& vis = (a_type == WidgetType::kOne) ? _cfg.oneVis : _cfg.allVis;
	if (vis.hideWidget)
		return false;

	if (a_menuName == RE::ContainerMenu::MENU_NAME)
		return a_gamepad ? vis.showInContainerGP : vis.showInContainerKBM;
	if (a_menuName == RE::BarterMenu::MENU_NAME)
		return a_gamepad ? vis.showInBarterGP : vis.showInBarterKBM;
	if (a_menuName == RE::GiftMenu::MENU_NAME)
		return a_gamepad ? vis.showInGiftGP : vis.showInGiftKBM;
	if (a_menuName == RE::InventoryMenu::MENU_NAME)
		return a_gamepad ? vis.showInInventoryGP : vis.showInInventoryKBM;
	return false;
}

void QtySettings::Load()
{
	GetSettings().Load([this](CSimpleIniA& ini) {
		LoadVis(ini, "Visibility_One", _cfg.oneVis, _def.oneVis);
		LoadVis(ini, "Visibility_All", _cfg.allVis, _def.allVis);

		FUCK::INI::LoadString(ini, "Appearance", "CustomOneLabel", _cfg.customOneLabel, _def.customOneLabel);
		FUCK::INI::LoadString(ini, "Appearance", "CustomAllLabel", _cfg.customAllLabel, _def.customAllLabel);
	});

	GetSettings().LoadKeybinds([](CSimpleIniA& ini) {
		for (auto* action : Actions())
			action->LoadKeybinds(ini);
	});
}

void QtySettings::Save() const
{
	GetSettings().Save([this](CSimpleIniA& ini) {
		SaveVis(ini, "Visibility_One", _cfg.oneVis, _def.oneVis);
		SaveVis(ini, "Visibility_All", _cfg.allVis, _def.allVis);

		FUCK::INI::SaveString(ini, "Appearance", "CustomOneLabel", _cfg.customOneLabel, _def.customOneLabel);
		FUCK::INI::SaveString(ini, "Appearance", "CustomAllLabel", _cfg.customAllLabel, _def.customAllLabel);
	});
}

void QtySettings::SaveKeybinds() const
{
	GetSettings().SaveKeybinds([](CSimpleIniA& ini) {
		for (const auto* action : Actions())
			action->SaveKeybinds(ini);
	});
}

// ==================================================
// QtyToolManager
// ==================================================

bool QtyToolManager::ProcessInput(const void* e)
{
	bool handled = false;
	for (auto* action : Actions()) {
		auto& hotkey = action->GetHotkey();
		if (hotkey.isBinding && FUCK::UpdateManagedHotkey(e, hotkey))
			handled = true;
	}

	if (handled)
		QtySettings::GetSingleton().SaveKeybinds();
	return handled;
}

void QtyToolManager::RenderToolUI()
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

		if (FUCK::Checkbox("$QTY_HideWidget"_T, &vis.hideWidget))
			saveSettings = true;
		FUCK::SetTooltip("$QTY_HideWidgetsTip"_T);

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
	FUCK::DrawManagedHotkey(settings.GetLabel(WidgetType::kOne), one->GetHotkey(), FUCK::HotkeyFlags::kNoModifiers);
	FUCK::DrawManagedHotkey(settings.GetLabel(WidgetType::kAll), all->GetHotkey(), FUCK::HotkeyFlags::kNoModifiers);

	FUCK::Dummy(FUCK::UIScale(0.0f, 15.0f));

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_WidgetLabels"_T);
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
		for (auto* action : Actions())
			action->GetHotkey().Clear();
		saveKeybinds = true;
	}

	FUCK::SameLine();

	if (FUCK::Button("$QTY_ResetToDefault"_T)) {
		for (auto* action : Actions())
			action->ResetKeybinds();
		saveKeybinds = true;
	}

	if (saveSettings)
		settings.Save();
	if (saveKeybinds)
		settings.SaveKeybinds();
}
