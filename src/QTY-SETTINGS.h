#pragma once

// Everything reads/writes the same INI file through this
inline FUCK::PluginSettings& GetSettings()
{
	static FUCK::PluginSettings s;
	return s;
}

enum class ActionType
{
	kOne,
	kAll
};

// Where each hint shows in the bottom bar. The hotkeys work either way
struct VisibilityConfig
{
	bool hideHint = false;

	bool showInContainerKBM = true;
	bool showInBarterKBM    = true;
	bool showInGiftKBM      = true;
	bool showInInventoryKBM = true;

	bool showInContainerGP = false;
	bool showInBarterGP    = false;
	bool showInGiftGP      = false;
	bool showInInventoryGP = false;
};

struct QtyConfig
{
	VisibilityConfig oneVis;
	VisibilityConfig allVis;

	char customOneLabel[64] = "$QTY_OneButton";
	char customAllLabel[64] = "$QTY_AllButton";
};

struct QtySettings
{
	QtyConfig       _cfg;
	const QtyConfig _def;

	static QtySettings& GetSingleton()
	{
		static QtySettings s;
		return s;
	}

	bool ShowsHint(std::string_view a_menuName, ActionType a_type, bool a_gamepad) const;

	const char* GetLabel(ActionType a_type) const
	{
		return FUCK::Translate(a_type == ActionType::kOne ? _cfg.customOneLabel : _cfg.customAllLabel);
	}

	// Settings and both actions' hotkeys
	void Load();
	void Save() const;
	void SaveKeybinds() const;

private:
	QtySettings() = default;
};
