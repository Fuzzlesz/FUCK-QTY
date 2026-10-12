#include "QTY-SETTINGS.h"
#include "QTY-ACTIONS.h"

namespace
{
	void LoadVis(CSimpleIniA& ini, const char* sec, VisibilityConfig& vis, const VisibilityConfig& def)
	{
		vis.hideHint           = FUCK::INI::LoadBool(ini, sec, "HideWidget",         def.hideHint);
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
		FUCK::INI::SaveBool(ini, sec, "HideWidget",         vis.hideHint,            def.hideHint);
		FUCK::INI::SaveBool(ini, sec, "ShowInContainerKBM", vis.showInContainerKBM,  def.showInContainerKBM);
		FUCK::INI::SaveBool(ini, sec, "ShowInBarterKBM",    vis.showInBarterKBM,     def.showInBarterKBM);
		FUCK::INI::SaveBool(ini, sec, "ShowInGiftKBM",      vis.showInGiftKBM,       def.showInGiftKBM);
		FUCK::INI::SaveBool(ini, sec, "ShowInInventoryKBM", vis.showInInventoryKBM,  def.showInInventoryKBM);

		FUCK::INI::SaveBool(ini, sec, "ShowInContainerGP",  vis.showInContainerGP,   def.showInContainerGP);
		FUCK::INI::SaveBool(ini, sec, "ShowInBarterGP",     vis.showInBarterGP,      def.showInBarterGP);
		FUCK::INI::SaveBool(ini, sec, "ShowInGiftGP",       vis.showInGiftGP,        def.showInGiftGP);
		FUCK::INI::SaveBool(ini, sec, "ShowInInventoryGP",  vis.showInInventoryGP,   def.showInInventoryGP);
	}
}

bool QtySettings::ShowsHint(std::string_view a_menuName, ActionType a_type, bool a_gamepad) const
{
	const auto& vis = (a_type == ActionType::kOne) ? _cfg.oneVis : _cfg.allVis;
	if (vis.hideHint)
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
		for (auto* action : QtyActions())
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
		for (const auto* action : QtyActions())
			action->SaveKeybinds(ini);
	});
}
