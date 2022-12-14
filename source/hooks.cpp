#include "hooks.h"

#include <stdexcept>
#include <intrin.h>

#include "../externals/minhook/minhook.h"

#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_win32.h"
#include "../externals/imgui/imgui_impl_dx9.h"

void hooks::Setup()
{

	// the reason you would throw a runtime error for this is beccause if minhook is successful it returns to zero.

	if (MH_Initialize())
		throw std::runtime_error("Minhook initialization failed!");

	if (MH_CreateHook(
		VirtualFunction(gui::device, 42), &EndScene, reinterpret_cast<void**>(&EndSceneOriginal)
	))
		throw std::runtime_error("Unable to hook EndScene()");

	if (MH_CreateHook(
		VirtualFunction(gui::device, 16), &Reset, reinterpret_cast<void**>(&ResetOriginal)
	))
		throw std::runtime_error("Unable to hook Reset()");

	if (MH_EnableHook(MH_ALL_HOOKS))
		throw std::runtime_error("Unable to enable hooks!");

	// You no longer need the DirectX device that was created
	
	gui::DestroyDirectX();
}

void hooks::Destroy() noexcept
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}

long __stdcall hooks::EndScene(IDirect3DDevice9* device) noexcept
{
	static const auto returnAdress = _ReturnAddress();

	const auto result = EndSceneOriginal(device, device);

	// This makes it so everything isnt being called twice

	if (_ReturnAddress() == returnAdress)
		return result;

	if (!gui::setup)
		gui::SetupMenu(device);

	if (gui::open)
		gui::Render();

}

HRESULT __stdcall hooks::Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = ResetOriginal(device, device, params);
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}