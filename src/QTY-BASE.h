#pragma once
#include "QTY-SHARED.h"

// A held hotkey that changes how many items the next transfer moves.
// Driven from the item menus' own AdvanceMovie, see QTY-MENUS
class QtyAction
{
public:
	struct Config
	{
		const char*   iniKeyHotkey;
		const char*   iniKeyGamepad;
		std::uint32_t defaultKey;
		std::uint32_t defaultGPKey;
		WidgetType    type;
	};

	explicit QtyAction(Config a_config) :
		_config(a_config)
	{
		_hotkey.kKey = _config.defaultKey;
		_hotkey.gKey = _config.defaultGPKey;
	}
	virtual ~QtyAction() = default;

	const Config&        GetConfig() const { return _config; }
	FUCK::ManagedHotkey& GetHotkey() { return _hotkey; }

	void LoadKeybinds(CSimpleIniA& a_ini);
	void SaveKeybinds(CSimpleIniA& a_ini) const;
	void ResetKeybinds();

	// Each frame while an item menu is open
	void Update(RE::GFxMovieView* a_movie);

	// The menu closed or changed. Its movie is gone, so there's nothing to restore in it
	void Reset();

protected:
	virtual int  GetOverrideCount() const = 0;
	virtual void ProcessExtraLogic(RE::GFxMovieView* /*a_movie*/, bool /*a_hotkeyDown*/) {}
	virtual void OnReset() {}

private:
	void RestoreDefaultCount(RE::GFxMovieView* a_movie);

	Config              _config;
	FUCK::ManagedHotkey _hotkey;

	bool               _wasDown = false;
	std::optional<int> _defaultCount;
};
