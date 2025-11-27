#pragma once

// ==================================================
// Shared settings accessor.
// All widgets read/write the same INI file via this singleton.
// ==================================================

inline FUCK::PluginSettings& GetSettings()
{
	static FUCK::PluginSettings s;
	return s;
}

// ==================================================
// Shared hotkeys registry
// ==================================================

struct QtySharedHotkeys
{
	FUCK::ManagedHotkey* one = nullptr;
	FUCK::ManagedHotkey* all = nullptr;

	static QtySharedHotkeys& GetSingleton()
	{
		static QtySharedHotkeys s;
		return s;
	}
};

// ==================================================
// Shared visual appearance and visibility settings.
// ==================================================

enum class WidgetType
{
	kOne,
	kAll
};

struct VisibilityConfig
{
	bool hideWidget = false;

	bool showInContainerKBM = true;
	bool showInBarterKBM    = true;
	bool showInGiftKBM      = true;
	bool showInInventoryKBM = true;

	bool showInContainerGP = false;
	bool showInBarterGP    = false;
	bool showInGiftGP      = false;
	bool showInInventoryGP = false;
};

struct AppearanceConfig
{
	float                iconScale  = 1.0f;
	float                labelScale = 1.0f;
	std::array<float, 3> textColor{ 0.89f, 0.89f, 0.89f };

	// Visibility
	VisibilityConfig oneVis;
	VisibilityConfig allVis;

	// Custom Labels
	char customOneLabel[64] = "$QTY_OneButton";
	char customAllLabel[64] = "$QTY_AllButton";
};

struct SharedWidgetAppearance
{
	AppearanceConfig       _cfg;
	const AppearanceConfig _def;

	static SharedWidgetAppearance& GetSingleton()
	{
		static SharedWidgetAppearance s;
		return s;
	}

	ImVec4 TextColorVec4() const
	{
		return { _cfg.textColor[0], _cfg.textColor[1], _cfg.textColor[2], 1.0f };
	}

	bool IsMenuEnabled(const std::string& menuName, WidgetType type) const
	{
		const auto& vis       = (type == WidgetType::kOne) ? _cfg.oneVis : _cfg.allVis;
		bool        isGamepad = (FUCK::GetInputDevice() == FUCK::InputDevice::kGamepad);

		if (menuName == RE::ContainerMenu::MENU_NAME)
			return isGamepad ? vis.showInContainerGP : vis.showInContainerKBM;
		if (menuName == RE::BarterMenu::MENU_NAME)
			return isGamepad ? vis.showInBarterGP : vis.showInBarterKBM;
		if (menuName == RE::GiftMenu::MENU_NAME)
			return isGamepad ? vis.showInGiftGP : vis.showInGiftKBM;
		if (menuName == RE::InventoryMenu::MENU_NAME)
			return isGamepad ? vis.showInInventoryGP : vis.showInInventoryKBM;
		return true;
	}

	void Reset();
	void Load(CSimpleIniA& ini);
	void Save(CSimpleIniA& ini) const;

private:
	SharedWidgetAppearance() = default;
};

// ==================================================
// Tool UI Manager
// ==================================================

class QtyToolManager
{
public:
	static QtyToolManager* GetSingleton()
	{
		static QtyToolManager s;
		return &s;
	}

	bool ProcessInput(const void* e);
	void RenderToolUI();

private:
	FUCK::ManagedHotkey _localOne;
	FUCK::ManagedHotkey _localAll;
};
