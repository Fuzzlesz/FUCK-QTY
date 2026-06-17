#include "QTY-BASE.h"

// ==================================================
// Constructor
// ==================================================

QtyWidgetBase::QtyWidgetBase(Config a_config) :
	_config(a_config)
{
	_state.anchorOffset = _config.defaultOffset;
	_state.autoAnchor   = true;
}

// ==================================================
// IWindow Overrides
// ==================================================

bool QtyWidgetBase::OnAsyncInput(const void* e)
{
	if (_hotkey.isBinding || FUCK::IsMenuOpen()) {
		if (FUCK::UpdateManagedHotkey(e, _hotkey)) {
			SaveSettings();
			return true;
		}
	}
	return false;
}

// ==================================================
// Common Lifecycle Methods
// ==================================================

void QtyWidgetBase::Initialize()
{
	_hotkey.kKey     = _config.defaultKey;
	_hotkey.gKey     = _config.defaultGPKey;
	_state.anchorPos = { -1.0f, -1.0f };
	_currentPos      = _state.anchorPos;
	LoadSettings();

	_menuListener = std::make_unique<FUCK::MenuEventListener>([this](const char* menuName, bool opening) {
		if (opening)
			OnMenuOpen(menuName);
		else
			OnMenuClose(menuName);
	});
}

void QtyWidgetBase::LoadSettings()
{
	GetSettings().Load([this](CSimpleIniA& ini) {
		float savedX = FUCK::INI::LoadFloat(ini, _config.iniSection, "X", -1.0f);
		float savedY = FUCK::INI::LoadFloat(ini, _config.iniSection, "Y", -1.0f);

		if (savedX >= 0.0f && savedY >= 0.0f) {
			_state.anchorPos.x = FUCK::Scale(savedX);
			_state.anchorPos.y = FUCK::Scale(savedY);
		} else {
			_state.anchorPos.x = -1.0f;
			_state.anchorPos.y = -1.0f;
		}

		_state.anchorOffset.x = FUCK::INI::LoadFloat(ini, _config.iniSection, "OffsetX", _config.defaultOffset.x);
		_state.anchorOffset.y = FUCK::INI::LoadFloat(ini, _config.iniSection, "OffsetY", _config.defaultOffset.y);

		_state.autoAnchor = FUCK::INI::LoadBool(ini, _config.iniSection, "AutoAnchor", true);
		SharedWidgetAppearance::GetSingleton().Load(ini);
	});

	GetSettings().LoadKeybinds([this](CSimpleIniA& ini) {
		_hotkey.kKey = FUCK::INI::LoadInt(ini, "Hotkeys", _config.iniKeyHotkey, _config.defaultKey);
		_hotkey.gKey = FUCK::INI::LoadInt(ini, "Hotkeys", _config.iniKeyGamepad, _config.defaultGPKey);
	});
}

void QtyWidgetBase::SaveSettings()
{
	GetSettings().Save([this](CSimpleIniA& ini) {
		float resScale = FUCK::GetResolutionScale();
		FUCK::INI::SaveDouble(ini, _config.iniSection, "X", _state.anchorPos.x / resScale, _config.defaultPos.x);
		FUCK::INI::SaveDouble(ini, _config.iniSection, "Y", _state.anchorPos.y / resScale, _config.defaultPos.y);
		FUCK::INI::SaveDouble(ini, _config.iniSection, "OffsetX", _state.anchorOffset.x, _config.defaultOffset.x);
		FUCK::INI::SaveDouble(ini, _config.iniSection, "OffsetY", _state.anchorOffset.y, _config.defaultOffset.y);
		FUCK::INI::SaveBool  (ini, _config.iniSection, "AutoAnchor", _state.autoAnchor, true);  // true is default

		SharedWidgetAppearance::GetSingleton().Save(ini);
	});

	GetSettings().SaveKeybinds([this](CSimpleIniA& ini) {
		FUCK::INI::SaveInt(ini, "Hotkeys", _config.iniKeyHotkey, _hotkey.kKey, _config.defaultKey);
		FUCK::INI::SaveInt(ini, "Hotkeys", _config.iniKeyGamepad, _hotkey.gKey, _config.defaultGPKey);
	});
}

void QtyWidgetBase::OnMenuOpen(const char* a_menuName)
{
	std::string menu(a_menuName);
	if (menu == RE::BarterMenu::MENU_NAME    || menu == RE::ContainerMenu::MENU_NAME ||
		menu == RE::GiftMenu::MENU_NAME      || menu == RE::InventoryMenu::MENU_NAME) {
		_activeMenu = menu;
		_wasDown    = false;
	}
}

void QtyWidgetBase::OnMenuClose(const char* a_menuName)
{
	if (_activeMenu == a_menuName) {
		RestoreDefaultCount();
		_activeMenu.clear();
		OnMenuStateReset();
	}
}

// ==================================================
// Draw
// ==================================================

void QtyWidgetBase::Draw()
{
	auto initRes = FUCK::InitializeCustomPosition(
		_state.anchorPos,
		GetDefaultPos(),
		GetDefaultSize(),
		_state.hasClampedPos);

	if (initRes == FUCK::PosInitResult::kNotReady) {
		return;
	}
	if (initRes == FUCK::PosInitResult::kChanged) {
		_currentPos = _state.anchorPos;
		SaveSettings();
	}

	// Process logic regardless of visual visibility so hidden hotkeys still work
	UpdateLogic();

	auto& appear = SharedWidgetAppearance::GetSingleton();

	const auto& vis             = (_config.widgetType == WidgetType::kOne) ? appear._cfg.oneVis : appear._cfg.allVis;
	bool        isVisibleInMenu = appear.IsMenuEnabled(_activeMenu, _config.widgetType);

	bool shouldDraw = !vis.hideWidget && isVisibleInMenu && IsItemHovered();

	// Abort visual drawing completely
	if (!shouldDraw) {
		if (!IsItemHovered() && _wasDown) {
			RestoreDefaultCount();
		}
		return;
	}

	bool isEditing = FUCK::IsMenuOpen();

	ImVec2 bottomBarPos;
	bool   hasBottomBar = GetBottomBarPos(bottomBarPos);
	ImVec2 expectedPos  = _state.autoAnchor && hasBottomBar ? ComputeAnchoredPos(bottomBarPos) : _state.anchorPos;

	if (isEditing) {
		HandlePositioning(expectedPos, bottomBarPos, hasBottomBar);
	}

	// Enforce lock to expected position when not actively dragging
	if (!isEditing || (!_isDragging && !FUCK::IsMouseDown(0))) {
		_currentPos = expectedPos;
		FUCK::SetWindowPos(_currentPos, ImGuiCond_Always);
	} else if (_isDragging) {
		// Update _currentPos during ImGui native drag so we can capture it upon release
		_currentPos = FUCK::GetWindowPos();
	}

	FUCK::PushStyleColor(ImGuiCol_Text, appear.TextColorVec4());

	FUCK::HotkeyFlags hkFlags =
		FUCK::HotkeyFlags::kAlignNear       |
		FUCK::HotkeyFlags::kLabelRight      |
		FUCK::HotkeyFlags::kCtrlToRebind    |
		FUCK::HotkeyFlags::kAlwaysHighlight |
		FUCK::HotkeyFlags::kNoModifiers     ;

	FUCK::DrawManagedHotkey(FUCK::Translate(GetCustomLabel()), _hotkey, hkFlags, appear._cfg.iconScale, appear._cfg.labelScale);

	// We pop style color before drawing the context menu so it reverts to the host theme
	FUCK::PopStyleColor();

	if (isEditing) {
		FUCK::SetTooltip("$QTY_RightClickConfigure"_T);
		DrawContextMenu(appear, expectedPos, bottomBarPos, hasBottomBar);
	}

	if (isEditing) {
		ImVec2 winMin  = FUCK::GetWindowPos();
		ImVec2 winSize = FUCK::GetWindowSize();
		ImVec2 winMax  = ImVec2(winMin.x + winSize.x, winMin.y + winSize.y);
		FUCK::DrawEditorBounds(winMin, winMax,
			_isHovered ? FUCK::EditorBoundsState::kHovered : FUCK::EditorBoundsState::kNormal,
			2.0f, true);
	}
}

// ==================================================
// Private Helpers
// ==================================================

ImVec2 QtyWidgetBase::ComputeAnchoredPos(const ImVec2& bottomBarPos) const
{
	return { bottomBarPos.x + FUCK::Scale(_state.anchorOffset.x),
		bottomBarPos.y + FUCK::Scale(_state.anchorOffset.y) };
}

bool QtyWidgetBase::GetBottomBarPos(ImVec2& outPos) const
{
	if (_activeMenu.empty())
		return false;

	auto ui = RE::UI::GetSingleton();
	auto m  = ui ? ui->GetMenu(_activeMenu) : nullptr;
	if (!m || !m->uiMovie)
		return false;

	RE::GFxValue target;
	if (!m->uiMovie->GetVariable(&target, "_root.Menu_mc.bottomBar.buttonPanel") || !target.IsObject())
		return false;

	// Convert local (0,0) of the panel to global Stage coordinates
	RE::GFxValue pt;
	m->uiMovie->CreateObject(&pt);
	pt.SetMember("x", 0.0);
	pt.SetMember("y", 0.0);
	target.Invoke("localToGlobal", nullptr, &pt, 1);

	RE::GFxValue xVal, yVal;
	if (!pt.GetMember("x", &xVal) || !pt.GetMember("y", &yVal))
		return false;
	if (!xVal.IsNumber() || !yVal.IsNumber())
		return false;

	float stageX     = static_cast<float>(xVal.GetNumber());
	float stageY     = static_cast<float>(yVal.GetNumber());
	float stageWidth = 0.0f;

	// This is the width of the button stack in the actionscript, changes based on what controls are on bottom bar
	RE::GFxValue widthVal;
	if (target.GetMember("_width", &widthVal) && widthVal.IsNumber())
		stageWidth = static_cast<float>(widthVal.GetNumber());

	// Let the FUCK API calculate aspect ratio logic and resolution offsets
	outPos = FUCK::TranslateScaleformToScreen(stageX + stageWidth, stageY);
	return true;
}

bool QtyWidgetBase::IsItemHovered() const
{
	if (_activeMenu.empty())
		return false;

	auto ui = RE::UI::GetSingleton();
	auto m  = ui ? ui->GetMenu(_activeMenu) : nullptr;
	if (!m || !m->uiMovie)
		return false;

	RE::GFxValue selectedIndex;
	if (m->uiMovie->GetVariable(&selectedIndex, "_root.Menu_mc.inventoryLists.itemList.selectedIndex") && selectedIndex.IsNumber()) {
		if (selectedIndex.GetNumber() != -1.0)
			return true;
	}
	return false;
}

void QtyWidgetBase::RestoreDefaultCount()
{
	if (!_wasDown || _activeMenu.empty())
		return;

	auto ui = RE::UI::GetSingleton();
	if (auto m = ui ? ui->GetMenu(_activeMenu) : nullptr; m && m->uiMovie && _defaultCount.has_value()) {
		m->uiMovie->SetVariable("_root.Menu_mc._quantityMinCount", RE::GFxValue(*_defaultCount));
	}

	_wasDown      = false;
	_defaultCount = std::nullopt;
}

void QtyWidgetBase::UpdateLogic()
{
	if (_activeMenu.empty())
		return;

	auto ui = RE::UI::GetSingleton();
	if (!ui)
		return;

	if (!ui->IsMenuOpen(_activeMenu) || _hotkey.isBinding) {
		if (_wasDown)
			RestoreDefaultCount();
		_wasDown = false;
		OnMenuStateReset();
		return;
	}

	bool hotkeyDown = FUCK::IsManagedHotkeyDown(_hotkey);

	if (hotkeyDown && !_wasDown) {
		auto m = ui->GetMenu(_activeMenu);
		if (m && m->uiMovie) {
			RE::GFxValue currentVal;
			if (m->uiMovie->GetVariable(&currentVal, "_root.Menu_mc._quantityMinCount") && currentVal.IsNumber()) {
				int possibleDefault = static_cast<int>(currentVal.GetNumber());
				if (possibleDefault != GetOverrideCount())
					_defaultCount = possibleDefault;
			}
			m->uiMovie->SetVariable("_root.Menu_mc._quantityMinCount", RE::GFxValue(GetOverrideCount()));
		}
	} else if (!hotkeyDown && _wasDown) {
		RestoreDefaultCount();
	}

	_wasDown = hotkeyDown;
	ProcessExtraLogic(ui, hotkeyDown);
}

void QtyWidgetBase::HandlePositioning(ImVec2& expectedPos, const ImVec2& bottomBarPos, bool hasBottomBar)
{
	bool isHovered = FUCK::IsWindowHovered(0);
	_isHovered     = isHovered;

	// Handle WASD
	float nx = 0.0f, ny = 0.0f;
	if (FUCK::WASDNudge(nx, ny, isHovered)) {
		if (_state.autoAnchor && hasBottomBar) {
			_state.anchorOffset.x += nx;
			_state.anchorOffset.y += ny;
		} else {
			_state.anchorPos.x += FUCK::Scale(nx);
			_state.anchorPos.y += FUCK::Scale(ny);
		}
		expectedPos = _state.autoAnchor && hasBottomBar ? ComputeAnchoredPos(bottomBarPos) : _state.anchorPos;
		SaveSettings();
	}

	// Detect active drag
	if (isHovered && FUCK::IsMouseClicked(0)) {
		_isDragging = true;
	}

	// Save new position offset after a drag event finishes
	if (_isDragging && FUCK::IsMouseReleased(0)) {
		_isDragging = false;

		if (_state.autoAnchor && hasBottomBar) {
			float scale           = FUCK::GetResolutionScale();
			_state.anchorOffset.x = (_currentPos.x - bottomBarPos.x) / scale;
			_state.anchorOffset.y = (_currentPos.y - bottomBarPos.y) / scale;
		} else {
			_state.anchorPos = _currentPos;
		}
		SaveSettings();
		expectedPos = _currentPos;
	}

	// Failsafe
	if (!FUCK::IsMouseDown(0)) {
		_isDragging = false;
	}
}

void QtyWidgetBase::DrawContextMenu(SharedWidgetAppearance& appear, ImVec2& expectedPos, const ImVec2& bottomBarPos, bool hasBottomBar)
{
	if (!FUCK::BeginPopupContextItem(_config.contextId, 1))
		return;

	FUCK::Dummy(FUCK::UIScale(350.0f, 0.0f));

	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), FUCK::Translate(_config.setupTitle));
	FUCK::Separator();
	FUCK::Spacing();

	FUCK::TextColoredWrapped(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "$QTY_RebindInstruction"_T);
	FUCK::TextColoredWrapped(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "$QTY_LabelPositionTip"_T);
	FUCK::Spacing();

	bool saveSettings        = false;
	bool posChangedThisFrame = false;

	FUCK::SliderFloat("$QTY_IconScale"_T, &appear._cfg.iconScale, 0.5f, 3.0f, "%.2f");
	if (FUCK::IsItemDeactivatedAfterEdit())
		saveSettings = true;

	FUCK::SliderFloat("$QTY_LabelScale"_T, &appear._cfg.labelScale, 0.5f, 3.0f, "%.2f");
	if (FUCK::IsItemDeactivatedAfterEdit())
		saveSettings = true;

	if (FUCK::ColorEdit3("$QTY_TextColour"_T, appear._cfg.textColor.data(), 0)) {
		saveSettings = true;
	}

	FUCK::Spacing(2);
	FUCK::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "$QTY_Positioning"_T);
	FUCK::Separator();
	FUCK::Spacing();

	if (FUCK::Checkbox("$QTY_AutoAnchor"_T, &_state.autoAnchor)) {
		if (_state.autoAnchor && hasBottomBar)
			_state.anchorOffset = _config.defaultOffset;
		else
			_state.anchorPos = _currentPos;
		saveSettings        = true;
		posChangedThisFrame = true;
	}

	if (_state.autoAnchor) {
		if (FUCK::DragFloat("$QTY_OffsetX"_T, &_state.anchorOffset.x, 1.0f, -9999.0f, 9999.0f, "%.1f"))
			posChangedThisFrame = true;
		if (FUCK::IsItemDeactivatedAfterEdit())
			saveSettings = true;

		if (FUCK::DragFloat("$QTY_OffsetY"_T, &_state.anchorOffset.y, 1.0f, -9999.0f, 9999.0f, "%.1f"))
			posChangedThisFrame = true;
		if (FUCK::IsItemDeactivatedAfterEdit())
			saveSettings = true;
	} else {
		if (FUCK::DragFloat("$QTY_X"_T, &_state.anchorPos.x, 1.0f, -9999.0f, 9999.0f, "%.1f"))
			posChangedThisFrame = true;
		if (FUCK::IsItemDeactivatedAfterEdit())
			saveSettings = true;

		if (FUCK::DragFloat("$QTY_Y"_T, &_state.anchorPos.y, 1.0f, -9999.0f, 9999.0f, "%.1f"))
			posChangedThisFrame = true;
		if (FUCK::IsItemDeactivatedAfterEdit())
			saveSettings = true;
	}

	FUCK::Dummy(FUCK::UIScale(0.0f, 10.0f));
	FUCK::Separator();
	FUCK::Spacing();

	if (FUCK::Button("$QTY_ResetToDefault"_T)) {
		_state.anchorPos    = FUCK::Scale(_config.defaultPos);
		_state.anchorOffset = _config.defaultOffset;
		_state.autoAnchor   = true;
		appear.Reset();

		saveSettings        = true;
		posChangedThisFrame = true;
	}

	if (posChangedThisFrame) {
		expectedPos = _state.autoAnchor && hasBottomBar ? ComputeAnchoredPos(bottomBarPos) : _state.anchorPos;
		_currentPos = expectedPos;
	}

	if (saveSettings) {
		SaveSettings();
	}

	FUCK::EndPopup();

	// Apply position change to the main window, outside of the popup context
	if (posChangedThisFrame)
		FUCK::SetWindowPos(_currentPos, ImGuiCond_Always);
}
