#define TOOLS_WINDOW
#include "tools.h"

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <d3d12.h>          // D3D12 interface
#include <dxgi1_6.h>        // DirectX graphics infrastructure (adapters, presentation, etc)
#include <d3dcompiler.h>    // Utilities to compile HLSL code at runtime
#include <d3d12sdklayers.h>
//#include <D3dx12.h>

/*
NOTE:
When using runtime compiled HLSL shaders using any of the D3DCompiler functions, do not
forget to link against the d3dcompiler.lib library and copy the D3dcompiler_47.dll to
the same folder as the binary executable when distributing your project.

A redistributable version of the D3dcompiler_47.dll file can be found in the Windows 10
SDK installation folder at C:\Program Files (x86)\Windows Kits\10\Redist\D3D\.

For more information, refer to the MSDN blog post at:
https://blogs.msdn.microsoft.com/chuckw/2012/05/07/hlsl-fxc-and-d3dcompile/
*/

static const UINT FRAME_COUNT = 2;

struct Vertex
{
	float3 position;
	float4 color;
};

struct GfxDevice
{
	ComPtr<ID3D12Device2> g_Device;
	ComPtr<IDXGISwapChain4> g_SwapChain;
	ComPtr<ID3D12Resource> g_BackBuffers[FRAME_COUNT];
	ComPtr<ID3D12CommandQueue> g_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> g_CommandList;
	ComPtr<ID3D12CommandAllocator> g_CommandAllocators[FRAME_COUNT];
	ComPtr<ID3D12DescriptorHeap> g_RTVDescriptorHeap; // Render Target Views (RTV)
	UINT g_RTVDescriptorSize;
	UINT g_CurrentBackBufferIndex;

	// Synchronization objects
	ComPtr<ID3D12Fence> g_Fence;
	uint64_t g_FenceValue = 0;
	uint64_t g_FrameFenceValues[FRAME_COUNT];
	HANDLE g_FenceEvent;

	// By default, enable V-Sync.
	// Can be toggled with the V key.
	bool g_VSync = true;
	bool g_TearingSupported = false;

	// By default, use windowed mode.
	// Can be toggled with the Alt+Enter or F11
	bool g_Fullscreen = false;
};

#define ThrowIfFailed(hRes) if( FAILED( hRes ) ) { return false; }

bool InitializeGraphics(Arena &arena, Window &window, GfxDevice &gfxDevice)
{
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();

	ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

	const bool useWarp = false;
    if (useWarp)
    {
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
	else
	{
	}

	return true;
}

void CleanupGraphics(GfxDevice &gfxDevice)
{
}

int main()
{
	// Create Window
	Window window = {};
	if ( !InitializeWindow(window) )
	{
		LOG(Error, "InitializeWindow failed!\n");
		return -1;
	}

	// Allocate base memory
	u32 baseMemorySize = MB(64);
	byte *baseMemory = (byte*)AllocateVirtualMemory(baseMemorySize);
	Arena arena = MakeArena(baseMemory, baseMemorySize);

	// Initialize graphics
	GfxDevice gfxDevice = {};
	if ( !InitializeGraphics(arena, window, gfxDevice) )
	{
		LOG(Error, "InitializeGraphics failed!\n");
		return -1;
	}

	// Application loop
	while ( 1 )
	{
		ProcessWindowEvents(window);

		if (window.flags & WindowFlags_Exiting)
		{
			break;
		}
	}

	CleanupGraphics(gfxDevice);

	CleanupWindow(window);

	return 0;
}

