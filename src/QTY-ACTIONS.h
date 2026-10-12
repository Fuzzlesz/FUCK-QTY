#pragma once
#include "QTY-SETTINGS.h"

// ==================================================
// QtyAction
// ==================================================

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
		ActionType    type;
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

	// Whether the hotkey was held as of the last Update()
	bool IsHeld() const { return _wasDown; }

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

// ==================================================
// Actions
// ==================================================

// Hold to move one item at a time, skipping the quantity prompt
class OneAction final : public QtyAction
{
public:
	static OneAction* GetSingleton()
	{
		static OneAction s;
		return &s;
	}

protected:
	int GetOverrideCount() const override { return kSkipPromptCount; }

private:
	// Below 1, so the menu moves one without asking
	static constexpr int kSkipPromptCount = -694208008;

	OneAction() :
		QtyAction({ "ONE_Key", "ONE_GPKey",
			29, 0,  // Left Ctrl, gamepad unbound
			ActionType::kOne })
	{}
};

// Hold to move the whole stack. Accepts SkyUI's quantity prompt for you if it opens anyway
class AllAction final : public QtyAction
{
public:
	static AllAction* GetSingleton()
	{
		static AllAction s;
		return &s;
	}

protected:
	int  GetOverrideCount() const override { return 1; }
	void ProcessExtraLogic(RE::GFxMovieView* a_movie, bool a_hotkeyDown) override;
	void OnReset() override { _handledCurrentPrompt = false; }

private:
	bool _handledCurrentPrompt = false;

	AllAction() :
		QtyAction({ "ALL_Key", "ALL_GPKey",
			44, 0,  // Z, gamepad unbound
			ActionType::kAll })
	{}
};

// Both actions, in the order their hints show
inline std::array<QtyAction*, 2> QtyActions()
{
	return { OneAction::GetSingleton(), AllAction::GetSingleton() };
}
