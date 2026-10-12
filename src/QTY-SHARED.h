#pragma once

// ==================================================
// Shared settings accessor.
// ==================================================

inline FUCK::PluginSettings& GetSettings()
{
	static FUCK::PluginSettings s;
	return s;
}

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

struct QtyConfig
{
	VisibilityConfig oneVis;
	VisibilityConfig allVis;

	char customOneLabel[64] = "$QTY_OneButton";
	char customAllLabel[64] = "$QTY_AllButton";
};

// ==================================================
// Settings
// ==================================================

struct QtySettings
{
	QtyConfig       _cfg;
	const QtyConfig _def;

	static QtySettings& GetSingleton()
	{
		static QtySettings s;
		return s;
	}

	bool ShowsHint(std::string_view a_menuName, WidgetType a_type, bool a_gamepad) const;

	const char* GetLabel(WidgetType a_type) const
	{
		return FUCK::Translate(a_type == WidgetType::kOne ? _cfg.customOneLabel : _cfg.customAllLabel);
	}

	void Load();
	void Save() const;
	void SaveKeybinds() const;

private:
	QtySettings() = default;
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
};
