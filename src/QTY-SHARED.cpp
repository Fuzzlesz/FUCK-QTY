#include "QTY-SHARED.h"

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
		FUCK::INI::SaveBool(ini, sec, "ShowInInventoryGP" , vis.showInInventoryGP,   def.showInInventoryGP);
	}
}

// ==================================================
// SharedWidgetAppearance
// ==================================================

void SharedWidgetAppearance::Reset()
{
	_cfg = _def;
}

void SharedWidgetAppearance::Load(CSimpleIniA& ini)
{
	_cfg.iconScale    = FUCK::INI::LoadFloat(ini, "Widget", "IconScale",  _def.iconScale);
	_cfg.labelScale   = FUCK::INI::LoadFloat(ini, "Widget", "LabelScale", _def.labelScale);
	_cfg.textColor[0] = FUCK::INI::LoadFloat(ini, "Widget", "TextColorR", _def.textColor[0]);
	_cfg.textColor[1] = FUCK::INI::LoadFloat(ini, "Widget", "TextColorG", _def.textColor[1]);
	_cfg.textColor[2] = FUCK::INI::LoadFloat(ini, "Widget", "TextColorB", _def.textColor[2]);

	LoadVis(ini, "Visibility_One", _cfg.oneVis, _def.oneVis);
	LoadVis(ini, "Visibility_All", _cfg.allVis, _def.allVis);

	FUCK::INI::LoadString(ini, "Appearance", "CustomOneLabel", _cfg.customOneLabel, _def.customOneLabel);
	FUCK::INI::LoadString(ini, "Appearance", "CustomAllLabel", _cfg.customAllLabel, _def.customAllLabel);
}

void SharedWidgetAppearance::Save(CSimpleIniA& ini) const
{
	FUCK::INI::SaveDouble(ini, "Widget", "IconScale", _cfg.iconScale,     _def.iconScale);
	FUCK::INI::SaveDouble(ini, "Widget", "LabelScale", _cfg.labelScale,   _def.labelScale);
	FUCK::INI::SaveDouble(ini, "Widget", "TextColorR", _cfg.textColor[0], _def.textColor[0]);
	FUCK::INI::SaveDouble(ini, "Widget", "TextColorG", _cfg.textColor[1], _def.textColor[1]);
	FUCK::INI::SaveDouble(ini, "Widget", "TextColorB", _cfg.textColor[2], _def.textColor[2]);

	SaveVis(ini, "Visibility_One", _cfg.oneVis, _def.oneVis);
	SaveVis(ini, "Visibility_All", _cfg.allVis, _def.allVis);

	FUCK::INI::SaveString(ini, "Appearance", "CustomOneLabel", _cfg.customOneLabel, _def.customOneLabel);
	FUCK::INI::SaveString(ini, "Appearance", "CustomAllLabel", _cfg.customAllLabel, _def.customAllLabel);
}

// ==================================================
// QtyToolManager
// ==================================================

bool QtyToolManager::ProcessInput(const void* e)
{
	auto& hk      = QtySharedHotkeys::GetSingleton();
	bool  handled = false;

	if (_localOne.isBinding && FUCK::UpdateManagedHotkey(e, _localOne)) {
		if (hk.one) {
			hk.one->kKey = _localOne.kKey;
			hk.one->gKey = _localOne.gKey;
		}
		handled = true;
	}
	if (_localAll.isBinding && FUCK::UpdateManagedHotkey(e, _localAll)) {
		if (hk.all) {
			hk.all->kKey = _localAll.kKey;
			hk.all->gKey = _localAll.gKey;
		}
		handled = true;
	}

	if (handled) {
		GetSettings().SaveKeybinds([&](CSimpleIniA& ini) {
			if (hk.one) {
				FUCK::INI::SaveInt(ini, "Hotkeys", "ONE_Key",   hk.one->kKey, 29);
				FUCK::INI::SaveInt(ini, "Hotkeys", "ONE_GPKey", hk.one->gKey, 0);
			}
			if (hk.all) {
				FUCK::INI::SaveInt(ini, "Hotkeys", "ALL_Key",   hk.all->kKey, 44);
				FUCK::INI::SaveInt(ini, "Hotkeys", "ALL_GPKey", hk.all->gKey, 0);
			}
		});
	}
	return handled;
}

void QtyToolManager::RenderToolUI()
{
	auto& appear = SharedWidgetAppearance::GetSingleton();
	auto& hk     = QtySharedHotkeys::GetSingleton();

	bool saveSettings = false;

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_ConfigTitle"_T);
	FUCK::Separator();
	FUCK::Spacing();

	auto drawVisConfig = [&](const char* title, VisibilityConfig& vis, const char* colsId) {
		if (FUCK::TreeNode(title)) {
			if (FUCK::Checkbox("$QTY_HideWidget"_T, &vis.hideWidget))
				saveSettings = true;
			FUCK::SetTooltip("$QTY_HideWidgetsTip"_T);

			FUCK::Dummy(FUCK::UIScale(0.0f, 5.0f));
			FUCK::Columns(2, colsId, false);

			// Column 1: KBM
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

			FUCK::NextColumn();

			// Column 2: Controller
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

			FUCK::Columns(1, nullptr, false);
			FUCK::TreePop();
		}
	};

	drawVisConfig("$QTY_VisibilityOne"_T, appear._cfg.oneVis, "VisColsOne");
	drawVisConfig("$QTY_VisibilityAll"_T, appear._cfg.allVis, "VisColsAll");

	FUCK::Dummy(FUCK::UIScale(0.0f, 15.0f));

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_Keybinds"_T);
	FUCK::Separator();
	FUCK::Spacing();

	FUCK::HotkeyFlags hkFlags =
		FUCK::HotkeyFlags::kNoModifiers;

	// Sync local hotkeys silently when not actively binding
	if (!_localOne.isBinding && hk.one) {
		_localOne.kKey = hk.one->kKey;
		_localOne.gKey = hk.one->gKey;
	}
	if (!_localAll.isBinding && hk.all) {
		_localAll.kKey = hk.all->kKey;
		_localAll.gKey = hk.all->gKey;
	}

	// Draw hotkey rebinding replicas using local structs
	if (hk.one)
		FUCK::DrawManagedHotkey(FUCK::Translate(appear._cfg.customOneLabel), _localOne, hkFlags, 1.0f, 1.0f);
	if (hk.all)
		FUCK::DrawManagedHotkey(FUCK::Translate(appear._cfg.customAllLabel), _localAll, hkFlags, 1.0f, 1.0f);

	FUCK::Dummy(FUCK::UIScale(0.0f, 15.0f));

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_WidgetLabels"_T);
	FUCK::Separator();
	FUCK::Spacing();

	FUCK::InputText("$QTY_LabelOne"_T, appear._cfg.customOneLabel, sizeof(appear._cfg.customOneLabel));
	if (FUCK::IsItemDeactivatedAfterEdit())
		saveSettings = true;

	FUCK::InputText("$QTY_LabelAll"_T, appear._cfg.customAllLabel, sizeof(appear._cfg.customAllLabel));
	if (FUCK::IsItemDeactivatedAfterEdit())
		saveSettings = true;

	FUCK::Dummy(FUCK::UIScale(0.0f, 15.0f));
	FUCK::Separator();
	FUCK::Spacing();

	if (FUCK::Button("$QTY_Unmap"_T)) {
		if (hk.one)
			hk.one->Clear();
		if (hk.all)
			hk.all->Clear();

		saveSettings = true;

		GetSettings().SaveKeybinds([&](CSimpleIniA& ini) {
			if (hk.one) {
				FUCK::INI::SaveInt(ini, "Hotkeys", "ONE_Key",   0, 29);
				FUCK::INI::SaveInt(ini, "Hotkeys", "ONE_GPKey", 0, 0);
			}
			if (hk.all) {
				FUCK::INI::SaveInt(ini, "Hotkeys", "ALL_Key",   0, 44);
				FUCK::INI::SaveInt(ini, "Hotkeys", "ALL_GPKey", 0, 0);
			}
		});
	}

	FUCK::SameLine();

	if (FUCK::Button("$QTY_ResetToDefault"_T)) {
		if (hk.one) {
			hk.one->Clear();
			hk.one->kKey = 29;
			hk.one->gKey = 0;
		}
		if (hk.all) {
			hk.all->Clear();
			hk.all->kKey = 44;
			hk.all->gKey = 0;
		}

		saveSettings = true;

		GetSettings().SaveKeybinds([&](CSimpleIniA& ini) {
			if (hk.one) {
				FUCK::INI::SaveInt(ini, "Hotkeys", "ONE_Key",   hk.one->kKey, 29);
				FUCK::INI::SaveInt(ini, "Hotkeys", "ONE_GPKey", hk.one->gKey, 0);
			}
			if (hk.all) {
				FUCK::INI::SaveInt(ini, "Hotkeys", "ALL_Key",   hk.all->kKey, 44);
				FUCK::INI::SaveInt(ini, "Hotkeys", "ALL_GPKey", hk.all->gKey, 0);
			}
		});
	}

	if (saveSettings) {
		GetSettings().Save([&](CSimpleIniA& ini) {
			appear.Save(ini);
		});
	}
}
