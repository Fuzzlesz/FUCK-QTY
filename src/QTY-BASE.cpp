#include "QTY-BASE.h"

namespace
{
	constexpr const char* kMinCountPath = "_root.Menu_mc._quantityMinCount";
}

// ==================================================
// Keybinds
// ==================================================

void QtyAction::LoadKeybinds(CSimpleIniA& a_ini)
{
	_hotkey.kKey = FUCK::INI::LoadInt(a_ini, "Hotkeys", _config.iniKeyHotkey,  _config.defaultKey);
	_hotkey.gKey = FUCK::INI::LoadInt(a_ini, "Hotkeys", _config.iniKeyGamepad, _config.defaultGPKey);
}

void QtyAction::SaveKeybinds(CSimpleIniA& a_ini) const
{
	FUCK::INI::SaveInt(a_ini, "Hotkeys", _config.iniKeyHotkey,  _hotkey.kKey, _config.defaultKey);
	FUCK::INI::SaveInt(a_ini, "Hotkeys", _config.iniKeyGamepad, _hotkey.gKey, _config.defaultGPKey);
}

void QtyAction::ResetKeybinds()
{
	_hotkey.Clear();
	_hotkey.kKey = _config.defaultKey;
	_hotkey.gKey = _config.defaultGPKey;
}

// ==================================================
// Menu Logic
// ==================================================

void QtyAction::Update(RE::GFxMovieView* a_movie)
{
	if (_hotkey.isBinding) {
		RestoreDefaultCount(a_movie);
		OnReset();
		return;
	}

	const bool hotkeyDown = FUCK::IsManagedHotkeyDown(_hotkey);

	if (hotkeyDown && !_wasDown) {
		// Keep the menu's own count to put back on release, unless it's already ours
		RE::GFxValue currentVal;
		if (a_movie->GetVariable(&currentVal, kMinCountPath) && currentVal.IsNumber()) {
			const int current = static_cast<int>(currentVal.GetNumber());
			if (current != GetOverrideCount())
				_defaultCount = current;
		}
		a_movie->SetVariable(kMinCountPath, RE::GFxValue(GetOverrideCount()));
		_wasDown = true;
	} else if (!hotkeyDown && _wasDown) {
		RestoreDefaultCount(a_movie);
	}

	ProcessExtraLogic(a_movie, hotkeyDown);
}

void QtyAction::Reset()
{
	_wasDown = false;
	_defaultCount.reset();
	OnReset();
}

void QtyAction::RestoreDefaultCount(RE::GFxMovieView* a_movie)
{
	if (!_wasDown)
		return;

	if (_defaultCount.has_value())
		a_movie->SetVariable(kMinCountPath, RE::GFxValue(*_defaultCount));

	_wasDown = false;
	_defaultCount.reset();
}
