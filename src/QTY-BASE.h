#pragma once
#include "QTY-SHARED.h"

class QtyWidgetBase : public FUCK::IWindow
{
public:
	// Static Configuration loaded on construction
	struct Config
	{
		const char*   title;
		const char*   iniSection;
		const char*   iniKeyHotkey;
		const char*   iniKeyGamepad;
		std::uint32_t defaultKey;
		std::uint32_t defaultGPKey;
		ImVec2        defaultPos;
		ImVec2        defaultOffset;  // Offset relative to the bottom bar when Auto-Anchored (Resolution independent)
		const char*   buttonLabel;
		const char*   contextId;
		const char*   setupTitle;
		WidgetType    widgetType;
	};

	struct State
	{
		ImVec2 anchorPos;
		ImVec2 anchorOffset;
		bool   autoAnchor    = true;
		bool   hasClampedPos = false;
	};

	explicit QtyWidgetBase(Config a_config);
	virtual ~QtyWidgetBase() = default;

	// --- IWindow Overrides ---
	const char* Id() const override { return _config.iniSection; }  // Dynamically uses ONE_Widget / ALL_Widget
	const char* Title() const override { return FUCK::Translate(_config.title); }

	bool IsOpen() const override
	{
		// Hide if no container menu is active
		if (_activeMenu.empty()) {
			return false;
		}

		if (auto ui = RE::UI::GetSingleton(); ui) {
			// Hide the widgets if a book/note is opened from inventory
			if (ui->IsMenuOpen(RE::BookMenu::MENU_NAME)) {
				return false;
			}
		}

		// Hide/Show instantly based on 3D item zoom direction.
		if (auto inv3d = RE::Inventory3DManager::GetSingleton(); inv3d) {
			static float s_lastZoomProgress = 0.0f;
			static bool  s_isZoomingOut     = false;

			float currentZoom = inv3d->zoomProgress;

			if (currentZoom < s_lastZoomProgress) {
				s_isZoomingOut = true;
			} else if (currentZoom > s_lastZoomProgress) {
				s_isZoomingOut = false;
			}

			s_lastZoomProgress = currentZoom;

			if (currentZoom > 0.001f) {
				// Hide immediately if we are zooming in or fully zoomed
				if (!s_isZoomingOut) {
					return false;
				}
			}
		}

		return true;
	}

	void SetOpen(bool /*a_open*/) override {}

	FUCK::WindowFlags GetFlags() const override
	{
		FUCK::WindowFlags flags =
			FUCK::WindowFlags::kNoDecoration    |
			FUCK::WindowFlags::kNoBackground    |
			FUCK::WindowFlags::kIgnoreUserScale |
			FUCK::WindowFlags::kNoResize        |
			FUCK::WindowFlags::kCustomPosition  ;

		if (!FUCK::IsMenuOpen())
			flags = flags | FUCK::WindowFlags::kPassInputToGame;

		return flags;
	}

	ImVec2 GetDefaultPos() const override { return FUCK::Scale(_config.defaultPos); }
	ImVec2 GetDefaultSize() const override { return { 0.0f, 0.0f }; }

	bool OnAsyncInput(const void* e) override;

	// --- Common Lifecycle Methods ---
	void Initialize();
	void LoadSettings();
	void SaveSettings();

	void Draw() override;

	void OnMenuOpen(const char* a_menuName);
	void OnMenuClose(const char* a_menuName);

protected:
	// Virtual behavior extensions
	virtual int         GetOverrideCount() const = 0;
	virtual const char* GetCustomLabel() const   = 0;
	virtual void        ProcessExtraLogic(RE::UI* /*ui*/, bool /*hotkeyDown*/) {}
	virtual void        OnMenuStateReset() {}

	Config _config;
	State  _state;

	FUCK::ManagedHotkey _hotkey;
	ImVec2              _currentPos;
	bool                _isHovered  = false;
	bool                _isDragging = false;

	// Game State
	std::string        _activeMenu;
	bool               _wasDown = false;
	std::optional<int> _defaultCount;

	std::unique_ptr<FUCK::MenuEventListener> _menuListener;

private:
	ImVec2 ComputeAnchoredPos(const ImVec2& bottomBarPos) const;
	bool   GetBottomBarPos(ImVec2& outPos) const;
	bool   IsItemHovered() const;
	void   RestoreDefaultCount();
	void   UpdateLogic();
	void   HandlePositioning(ImVec2& expectedPos, const ImVec2& bottomBarPos, bool hasBottomBar);
	void   DrawContextMenu(SharedWidgetAppearance& appear, ImVec2& expectedPos, const ImVec2& bottomBarPos, bool hasBottomBar);
};
