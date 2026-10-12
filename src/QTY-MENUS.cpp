#include "QTY-MENUS.h"
#include "QTY-ALL.h"
#include "QTY-ONE.h"

namespace
{
	// Where the menu's own updateBottomBar is kept once ours replaces it. Also marks the menu as done
	constexpr const char* kOriginalMember = "_qtyUpdateBottomBar";

	std::array<QtyAction*, 2> Actions()
	{
		return { OneAction::GetSingleton(), AllAction::GetSingleton() };
	}

	// The open item menu, set from its AdvanceMovie
	struct ActiveMenu
	{
		RE::GFxMovieView* movie = nullptr;
		std::string_view  name;
		bool              hooked = false;
		int               tries  = 0;
	};
	ActiveMenu g_active;

	bool IsItemSelected(const RE::GFxValue& a_menu)
	{
		RE::GFxValue index;
		RE::GFxValue lists, itemList;
		return a_menu.GetMember("inventoryLists", &lists) && lists.IsObject() &&
		       lists.GetMember("itemList", &itemList) && itemList.IsObject() &&
		       itemList.GetMember("selectedIndex", &index) && index.IsNumber() && index.GetNumber() != -1.0;
	}

	// Adds a hint for each action this menu and device should show
	void AddHints(RE::GFxMovie* a_movie, RE::GFxValue& a_menu)
	{
		RE::GFxValue navPanel;
		if (!a_menu.GetMember("navPanel", &navPanel) || !navPanel.IsObject())
			return;

		// The menu's own platform, so the hint switches with the rest of the bar. 0 is keyboard and mouse
		RE::GFxValue platform;
		const bool   gamepad = a_menu.GetMember("_platform", &platform) && platform.IsNumber() ?
		                           platform.GetNumber() != 0.0 :
		                           FUCK::GetInputDevice() == FUCK::InputDevice::kGamepad;

		const auto& settings = QtySettings::GetSingleton();
		bool        added    = false;

		for (auto* action : Actions()) {
			const auto& config = action->GetConfig();
			if (!settings.ShowsHint(g_active.name, config.type, gamepad))
				continue;

			// FUCK's key codes are SKSE's, which is what SkyUI's buttons take
			const auto& hotkey = action->GetHotkey();
			const auto  key    = gamepad ? hotkey.gKey : hotkey.kKey;
			if (key == 0)
				continue;

			RE::GFxValue controls;
			a_movie->CreateObject(&controls);
			controls.SetMember("keyCode", static_cast<double>(key));

			// Already translated, so the menu shows it as it is
			RE::GFxValue data;
			a_movie->CreateObject(&data);
			data.SetMember("text", settings.GetLabel(config.type));
			data.SetMember("controls", controls);

			RE::GFxValue button;
			navPanel.Invoke("addButton", &button, &data, 1);
			if (button.IsUndefined()) {
				// Every button slot is taken
				static bool s_warnedFull = false;
				if (!s_warnedFull) {
					s_warnedFull = true;
					logger::warn("{}'s bottom bar is full, so a QTY hint was left out", g_active.name);
				}
				break;
			}
			added = true;
		}

		if (added) {
			RE::GFxValue instant(true);
			navPanel.Invoke("updateButtons", nullptr, &instant, 1);
		}
	}

	// Stands in for the menu's updateBottomBar: runs the menu's own, then adds our hints after its buttons
	class UpdateBottomBarHandler : public RE::GFxFunctionHandler
	{
	public:
		void Call(Params& a_params) override
		{
			if (!a_params.thisPtr)
				return;

			RE::GFxValue& menu = *a_params.thisPtr;

			RE::GFxValue original;
			if (menu.GetMember(kOriginalMember, &original) && original.IsObject()) {
				// Function.call(this, args...). argsWithThisRef is exactly that list
				original.Invoke("call", a_params.retVal, a_params.argsWithThisRef, a_params.argCount + 1);
			}

			// Only with an item selected, where the menu shows its item buttons
			const bool selected = a_params.argCount > 0 && a_params.args[0].IsBool() && a_params.args[0].GetBool();
			if (selected && IsItemSelected(menu))
				AddHints(a_params.movie, menu);
		}
	};

	// Swaps the menu's updateBottomBar for ours. Returns false while the menu isn't ready, to try again next frame
	bool HookBottomBar(RE::GFxMovieView* a_movie)
	{
		RE::GFxValue menu;
		if (!a_movie->GetVariable(&menu, "_root.Menu_mc") || !menu.IsObject())
			return false;

		if (menu.HasMember(kOriginalMember))
			return true;

		RE::GFxValue original, navPanel;
		if (!menu.GetMember("updateBottomBar", &original) || !original.IsObject() ||
			!menu.GetMember("navPanel", &navPanel) || !navPanel.IsObject()) {
			// It can take a frame or two to be set up. Still missing after a second, it's not a bar we know
			if (++g_active.tries == 60)
				logger::warn("{} has no SkyUI bottom bar QTY knows, so no hints there. The hotkeys still work", g_active.name);
			return false;
		}

		// Kept for the session, so the function never outlives its handler
		static auto s_handler = RE::make_gptr<UpdateBottomBarHandler>();

		RE::GFxValue replacement;
		a_movie->CreateFunction(&replacement, s_handler.get());
		menu.SetMember(kOriginalMember, original);
		menu.SetMember("updateBottomBar", replacement);

		// Redraw the bar now, so the hints are there without waiting for the selection to change
		RE::GFxValue selected(IsItemSelected(menu));
		menu.Invoke("updateBottomBar", nullptr, &selected, 1);
		return true;
	}

	void OnAdvance(std::string_view a_menuName, RE::GFxMovieView* a_movie)
	{
		// A new menu: start clean
		if (a_movie != g_active.movie) {
			g_active = { a_movie, a_menuName };
			for (auto* action : Actions())
				action->Reset();
		}

		if (!g_active.hooked)
			g_active.hooked = HookBottomBar(a_movie);

		for (auto* action : Actions())
			action->Update(a_movie);
	}

	template <class M>
	struct AdvanceMovieHook
	{
		static void thunk(M* a_this, float a_interval, std::uint32_t a_currentTime)
		{
			func(a_this, a_interval, a_currentTime);
			if (a_this->uiMovie)
				OnAdvance(M::MENU_NAME, a_this->uiMovie.get());
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static constexpr std::size_t                   size = 0x05;
	};

	template <class M>
	void HookMenu()
	{
		REL::Relocation<std::uintptr_t> vtbl{ M::VTABLE[0] };
		AdvanceMovieHook<M>::func = vtbl.write_vfunc(AdvanceMovieHook<M>::size, AdvanceMovieHook<M>::thunk);
	}

	// Forgets the menu once it closes, so a new one at the same address still starts clean
	std::unique_ptr<FUCK::MenuEventListener> g_menuListener;
}

namespace QtyMenus
{
	void Install()
	{
		HookMenu<RE::ContainerMenu>();
		HookMenu<RE::BarterMenu>();
		HookMenu<RE::GiftMenu>();
		HookMenu<RE::InventoryMenu>();

		g_menuListener = std::make_unique<FUCK::MenuEventListener>([](const char* a_menuName, bool a_opening) {
			if (!a_opening && g_active.movie && g_active.name == a_menuName) {
				g_active = {};
				for (auto* action : Actions())
					action->Reset();
			}
		});
	}
}
