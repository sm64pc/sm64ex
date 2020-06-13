#ifdef RAPI_D3D11

#if defined(_WIN32) || defined(_WIN64)

#include <cstdio>
#include <vector>
#include <cmath>

#include <windows.h>
#include <versionhelpers.h>
#include <wrl/client.h>

#include <dxgi1_3.h>
#include <d3d11.h>
#include <d3dcompiler.h>

extern "C" {
#include "../cliopts.h"
#include "../configfile.h"
#include "../platform.h"
#include "../pc_main.h"
}

#ifndef _LANGUAGE_C
# define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#include "gfx_cc.h"
#include "gfx_window_manager_api.h"
#include "gfx_rendering_api.h"
#include "gfx_direct3d_common.h"

#include "gfx_screen_config.h"

#define WINCLASS_NAME L"SUPERMARIO64"
#define WINDOW_CLIENT_MIN_WIDTH 320
#define WINDOW_CLIENT_MIN_HEIGHT 240
#define DEBUG_D3D 0

using namespace Microsoft::WRL; // For ComPtr

struct PerFrameCB {
    uint32_t noise_frame;
    float noise_scale_x;
    float noise_scale_y;
    uint32_t padding;
};

struct PerDrawCB {
    struct Texture {
        uint32_t width;
        uint32_t height;
        uint32_t linear_filtering;
        uint32_t padding;
    } textures[2];
};

struct TextureData {
    ComPtr<ID3D11ShaderResourceView> resource_view;
    ComPtr<ID3D11SamplerState> sampler_state;
    uint32_t width;
    uint32_t height;
    bool linear_filtering;
};

struct ShaderProgram {
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> input_layout;
    ComPtr<ID3D11BlendState> blend_state;

    uint32_t shader_id;
    uint8_t num_inputs;
    uint8_t num_floats;
    bool used_textures[2];
};

static struct {
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swap_chain;   // For Windows versions older than 8.1
    ComPtr<IDXGISwapChain2> swap_chain2; // For Windows version 8.1 or newer
    ComPtr<ID3D11RenderTargetView> backbuffer_view;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    ComPtr<ID3D11RasterizerState> rasterizer_state;
    ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    ComPtr<ID3D11Buffer> vertex_buffer;
    ComPtr<ID3D11Buffer> per_frame_cb;
    ComPtr<ID3D11Buffer> per_draw_cb;

#if DEBUG_D3D
    ComPtr<ID3D11Debug> debug;
#endif

    HANDLE frame_latency_waitable_object;

    DXGI_SAMPLE_DESC sample_description;

    PerFrameCB per_frame_cb_data;
    PerDrawCB per_draw_cb_data;

    struct ShaderProgram shader_program_pool[64];
    uint8_t shader_program_pool_size;

    std::vector<struct TextureData> textures;
    int current_tile;
    uint32_t current_texture_ids[2];

    // Current state

    struct ShaderProgram *shader_program;

    uint32_t current_width, current_height;

    int8_t depth_test;
    int8_t depth_mask;
    int8_t zmode_decal;

    // Previous states (to prevent setting states needlessly)

    struct ShaderProgram *last_shader_program = nullptr;
    uint32_t last_vertex_buffer_stride = 0;
    ComPtr<ID3D11BlendState> last_blend_state = nullptr;
    ComPtr<ID3D11ShaderResourceView> last_resource_views[2] = { nullptr, nullptr };
    ComPtr<ID3D11SamplerState> last_sampler_states[2] = { nullptr, nullptr };
    int8_t last_depth_test = -1;
    int8_t last_depth_mask = -1;
    int8_t last_zmode_decal = -1;
    D3D_PRIMITIVE_TOPOLOGY last_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

    // Game loop callback

    void (*run_one_game_iter)(void);
    bool (*on_key_down)(int scancode);
    bool (*on_key_up)(int scancode);
    void (*on_all_keys_up)(void);
} d3d;

static HWND h_wnd;
static bool lower_latency;
static LARGE_INTEGER last_time, accumulated_time, frequency;
static uint8_t sync_interval;
static RECT last_window_rect;
static bool is_full_screen, last_maximized_state;

static void toggle_borderless_window_full_screen(void) {
    if (is_full_screen) {
        // set this right away so the fucking wndproc doesn't bother with anything stupid
        is_full_screen = false;

        RECT r = last_window_rect;

        // Set in window mode with the last saved position and size
        SetWindowLongPtr(h_wnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);

        if (last_maximized_state) {
            SetWindowPos(h_wnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
            ShowWindow(h_wnd, SW_MAXIMIZE);
        } else {
            SetWindowPos(h_wnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_FRAMECHANGED);
            ShowWindow(h_wnd, SW_RESTORE);
        }
    } else {
        // set this right away so the fucking wndproc doesn't bother with anything stupid
        is_full_screen = true;

        // Save if window is maximized or not
        WINDOWPLACEMENT window_placement;
        window_placement.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(h_wnd, &window_placement);
        last_maximized_state = window_placement.showCmd == SW_SHOWMAXIMIZED;

        // Save window position and size if the window is not maximized
        GetWindowRect(h_wnd, &last_window_rect);
        configWindow.x = last_window_rect.left;
        configWindow.y = last_window_rect.top;
        configWindow.w = last_window_rect.right - last_window_rect.left;
        configWindow.h = last_window_rect.bottom - last_window_rect.top;

        // Get in which monitor the window is
        HMONITOR h_monitor = MonitorFromWindow(h_wnd, MONITOR_DEFAULTTONEAREST);

        // Get info from that monitor
        MONITORINFOEX monitor_info;
        monitor_info.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(h_monitor, &monitor_info);
        RECT r = monitor_info.rcMonitor;

        // Set borderless full screen to that monitor
        SetWindowLongPtr(h_wnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
        SetWindowPos(h_wnd, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_FRAMECHANGED);
    }
}

static void create_render_target_views(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return;
    }
    if (d3d.current_width == width && d3d.current_height == height) {
        return;
    }

    // Release previous stuff (if any)

    d3d.backbuffer_view.Reset();
    d3d.depth_stencil_view.Reset();

    // Resize swap chain

    if (lower_latency) {
        UINT swap_chain_flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        ThrowIfFailed(d3d.swap_chain2->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, swap_chain_flags),
                      h_wnd, "Failed to resize IDXGISwapChain2 buffers.");
    } else {
        UINT swap_chain_flags = 0;
        ThrowIfFailed(d3d.swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, swap_chain_flags),
                      h_wnd, "Failed to resize IDXGISwapChain buffers.");
    }

    // Create back buffer

    ComPtr<ID3D11Texture2D> backbuffer_texture;
    if (lower_latency) {
        ThrowIfFailed(d3d.swap_chain2->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *) backbuffer_texture.GetAddressOf()),
                      h_wnd, "Failed to get backbuffer from IDXGISwapChain2.");
    } else {
        ThrowIfFailed(d3d.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *) backbuffer_texture.GetAddressOf()),
                      h_wnd, "Failed to get backbuffer from IDXGISwapChain.");
    }

    ThrowIfFailed(d3d.device->CreateRenderTargetView(backbuffer_texture.Get(), NULL, d3d.backbuffer_view.GetAddressOf()),
                  h_wnd, "Failed to create render target view.");

    // Create depth buffer

    D3D11_TEXTURE2D_DESC depth_stencil_texture_desc;
    ZeroMemory(&depth_stencil_texture_desc, sizeof(D3D11_TEXTURE2D_DESC));

    depth_stencil_texture_desc.Width = width;
    depth_stencil_texture_desc.Height = height;
    depth_stencil_texture_desc.MipLevels = 1;
    depth_stencil_texture_desc.ArraySize = 1;
    depth_stencil_texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_texture_desc.SampleDesc = d3d.sample_description;
    depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_stencil_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth_stencil_texture_desc.CPUAccessFlags = 0;
    depth_stencil_texture_desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> depth_stencil_texture;
    ThrowIfFailed(d3d.device->CreateTexture2D(&depth_stencil_texture_desc, NULL, depth_stencil_texture.GetAddressOf()));
    ThrowIfFailed(d3d.device->CreateDepthStencilView(depth_stencil_texture.Get(), NULL, d3d.depth_stencil_view.GetAddressOf()));

    // Save resolution

    d3d.current_width = width;
    d3d.current_height = height;
}

static void update_screen_settings(void) {
    if (configWindow.fullscreen != is_full_screen)
        toggle_borderless_window_full_screen();
    if (!is_full_screen) {
        const int screen_width = GetSystemMetrics(SM_CXSCREEN);
        const int screen_height = GetSystemMetrics(SM_CYSCREEN);
        const int xpos = (configWindow.x == WAPI_WIN_CENTERPOS) ? (screen_width - configWindow.w) * 0.5 : configWindow.x;
        const int ypos = (configWindow.y == WAPI_WIN_CENTERPOS) ? (screen_height - configWindow.h) * 0.5 : configWindow.y;
        RECT wr = { xpos, ypos, xpos + (int)configWindow.w, ypos + (int)configWindow.h };
        AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
        SetWindowPos(h_wnd, NULL, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, SWP_NOACTIVATE | SWP_NOZORDER);
    }
}

static void calculate_sync_interval() {
    const POINT ptZero = { 0, 0 };
    HMONITOR h_monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);

    MONITORINFOEX monitor_info;
    monitor_info.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(h_monitor, &monitor_info);

    DEVMODE dev_mode;
    dev_mode.dmSize = sizeof(DEVMODE);
    dev_mode.dmDriverExtra = 0;
    EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS, &dev_mode);

    if (dev_mode.dmDisplayFrequency >= 29 && dev_mode.dmDisplayFrequency <= 31) {
        sync_interval = 1;
    } else if (dev_mode.dmDisplayFrequency >= 59 && dev_mode.dmDisplayFrequency <= 61) {
        sync_interval = 2;
    } else if (dev_mode.dmDisplayFrequency >= 89 && dev_mode.dmDisplayFrequency <= 91) {
        sync_interval = 3;
    } else if (dev_mode.dmDisplayFrequency >= 119 && dev_mode.dmDisplayFrequency <= 121) {
        sync_interval = 4;
    } else {
        sync_interval = 0;
    }
}

LRESULT CALLBACK gfx_d3d11_dxgi_wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param) {
    switch (message) {
        case WM_MOVE:
            if (!is_full_screen) {
                const int x = (short)LOWORD(l_param);
                const int y = (short)HIWORD(l_param);
                configWindow.x = (x < 0) ? 0 : x;
                configWindow.y = (y < 0) ? 0 : y;
            }
            break;
        case WM_SIZE: {
            RECT rect;
            GetClientRect(h_wnd, &rect);
            const int w = rect.right - rect.left;
            const int h = rect.bottom - rect.top;
            if (!is_full_screen) {
                configWindow.w = w;
                configWindow.h = h;
            }
            create_render_target_views(w, h);
            break;
        }
        case WM_EXITSIZEMOVE: {
            calculate_sync_interval();
            break;
        }
        case WM_GETMINMAXINFO: {
            RECT wr = { 0, 0, WINDOW_CLIENT_MIN_WIDTH, WINDOW_CLIENT_MIN_HEIGHT };
            AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
            LPMINMAXINFO lpMMI = (LPMINMAXINFO) l_param;
            lpMMI->ptMinTrackSize.x = wr.right - wr.left;
            lpMMI->ptMinTrackSize.y = wr.bottom - wr.top;
            break;
        }
        case WM_DISPLAYCHANGE: {
            calculate_sync_interval();
            break;
        }
        case WM_DESTROY: {
#if DEBUG_D3D
            d3d.debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
            game_exit();
            break;
        }
        case WM_ACTIVATEAPP: {
            if (d3d.on_all_keys_up != nullptr) {
                d3d.on_all_keys_up();
            }
            break;
        }
        case WM_SYSKEYDOWN: {
            if ((w_param == VK_RETURN) && ((l_param & 1 << 30) == 0)) {
                toggle_borderless_window_full_screen();
                break;
            } else {
                return DefWindowProcW(h_wnd, message, w_param, l_param);
            }
        }
        case WM_KEYDOWN: {
            if (d3d.on_key_down != nullptr) {
                d3d.on_key_down((l_param >> 16) & 0x1ff);
            }
            break;
        }
        case WM_KEYUP: {
            if (d3d.on_key_up != nullptr) {
                d3d.on_key_up((l_param >> 16) & 0x1ff);
            }
            break;
        }
        default:
            break;
    }

    // check if we should change size or fullscreen state

    if (configWindow.reset) {
        last_maximized_state = false;
        configWindow.reset = false;
        configWindow.x = WAPI_WIN_CENTERPOS;
        configWindow.y = WAPI_WIN_CENTERPOS;
        configWindow.w = DESIRED_SCREEN_WIDTH;
        configWindow.h = DESIRED_SCREEN_HEIGHT;
        configWindow.fullscreen = false;
        configWindow.settings_changed = true;
    }

    if (configWindow.settings_changed) {
        configWindow.settings_changed = false;
        update_screen_settings();
    }

    return DefWindowProcW(h_wnd, message, w_param, l_param);
}

static void gfx_d3d11_dxgi_init(const char *window_title) {
    // Prepare window title

    wchar_t w_title[512];
    mbstowcs(w_title, window_title, strlen(window_title) + 1);

    // Create window

    WNDCLASSEXW wcex;
    ZeroMemory(&wcex, sizeof(WNDCLASSEX));

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = gfx_d3d11_dxgi_wnd_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = nullptr;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WINCLASS_NAME;
    wcex.hIconSm = nullptr;

    RegisterClassExW(&wcex);

    RECT wr = { 0, 0, DESIRED_SCREEN_WIDTH, DESIRED_SCREEN_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    h_wnd = CreateWindowW(WINCLASS_NAME, w_title, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, 0, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr,
                          nullptr, nullptr);

    is_full_screen = false;

    // Center window if the current position in the config is set to auto, otherwise use that position

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    int xPos = (configWindow.x == WAPI_WIN_CENTERPOS) ? (screen_width - wr.right) * 0.5 : configWindow.x;
    int yPos = (configWindow.y == WAPI_WIN_CENTERPOS) ? (screen_height - wr.bottom) * 0.5 : configWindow.y;
    SetWindowPos(h_wnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    // Check if a lower latency flip model can be used

    lower_latency = IsWindows8Point1OrGreater();

    // Create D3D11 device

#if DEBUG_D3D
    UINT device_creation_flags = D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT device_creation_flags = 0;
#endif

    D3D_FEATURE_LEVEL FeatureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    ThrowIfFailed(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        device_creation_flags,
        FeatureLevels,
        ARRAYSIZE(FeatureLevels),
        D3D11_SDK_VERSION,
        d3d.device.GetAddressOf(),
        NULL,
        d3d.context.GetAddressOf()),
        h_wnd, "Failed to create D3D11 device.");

    // Sample description to be used in back buffer and depth buffer

    d3d.sample_description.Count = 1;
    d3d.sample_description.Quality = 0;

    // Create the swap chain

    if (lower_latency) {

        // Create swap chain description

        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1;
        ZeroMemory(&swap_chain_desc1, sizeof(DXGI_SWAP_CHAIN_DESC1));

        swap_chain_desc1.Width = DESIRED_SCREEN_WIDTH;
        swap_chain_desc1.Height = DESIRED_SCREEN_HEIGHT;
        swap_chain_desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc1.Stereo = FALSE;
        swap_chain_desc1.SampleDesc = d3d.sample_description;
        swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc1.BufferCount = 2;
        swap_chain_desc1.Scaling = DXGI_SCALING_STRETCH;
        swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swap_chain_desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swap_chain_desc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

        // Create DXGI Factory

        ComPtr<IDXGIDevice2> dxgi_device2;
        ThrowIfFailed(d3d.device.Get()->QueryInterface(__uuidof(IDXGIDevice2), (void **) dxgi_device2.GetAddressOf()),
                      h_wnd, "Failed to get IDXGIDevice2.");

        ComPtr<IDXGIAdapter> dxgi_adapter;
        ThrowIfFailed(dxgi_device2.Get()->GetAdapter(dxgi_adapter.GetAddressOf()),
                      h_wnd, "Failed to get IDXGIAdapter.");

        ComPtr<IDXGIFactory2> dxgi_factory2;
        ThrowIfFailed(dxgi_adapter.Get()->GetParent(__uuidof(IDXGIFactory2), (void **) dxgi_factory2.GetAddressOf()),
                      h_wnd, "Failed to get IDXGIFactory2.");

        // Create Swap Chain

        ComPtr<IDXGISwapChain1> swap_chain1;
        ThrowIfFailed(dxgi_factory2.Get()->CreateSwapChainForHwnd(d3d.device.Get(), h_wnd, &swap_chain_desc1, NULL, NULL, swap_chain1.GetAddressOf()),
                      h_wnd, "Failed to create IDXGISwapChain1.");

        ThrowIfFailed(swap_chain1.As(&d3d.swap_chain2),
                      h_wnd, "Failed to get IDXGISwapChain2 from IDXGISwapChain1.");

        ThrowIfFailed(d3d.swap_chain2.Get()->SetMaximumFrameLatency(1),
                      h_wnd, "Failed to Set Maximum Frame Latency to 1.");

        d3d.frame_latency_waitable_object = d3d.swap_chain2.Get()->GetFrameLatencyWaitableObject();

        // Prevent DXGI from intercepting Alt+Enter

        ThrowIfFailed(dxgi_factory2.Get()->MakeWindowAssociation(h_wnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER),
                      h_wnd, "Failed to call MakeWindowAssociation.");

    } else {

        // Create swap chain description

        DXGI_SWAP_CHAIN_DESC swap_chain_desc;
        ZeroMemory(&swap_chain_desc, sizeof(DXGI_SWAP_CHAIN_DESC));

        swap_chain_desc.BufferDesc.Width = DESIRED_SCREEN_WIDTH;
        swap_chain_desc.BufferDesc.Height = DESIRED_SCREEN_HEIGHT;
        swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
        swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swap_chain_desc.SampleDesc = d3d.sample_description;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = 1;
        swap_chain_desc.OutputWindow = h_wnd;
        swap_chain_desc.Windowed = TRUE;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swap_chain_desc.Flags = 0;

        // Create DXGI Factory

        ComPtr<IDXGIDevice> dxgi_device;
        ThrowIfFailed(d3d.device.Get()->QueryInterface(__uuidof(IDXGIDevice), (void **) dxgi_device.GetAddressOf()),
                      h_wnd, "Failed to get IDXGIDevice.");

        ComPtr<IDXGIAdapter> dxgi_adapter;
        ThrowIfFailed(dxgi_device.Get()->GetAdapter(dxgi_adapter.GetAddressOf()),
                      h_wnd, "Failed to get IDXGIAdapter.");

        ComPtr<IDXGIFactory> dxgi_factory;
        ThrowIfFailed(dxgi_adapter.Get()->GetParent(__uuidof(IDXGIFactory), (void **) dxgi_factory.GetAddressOf()),
                      h_wnd, "Failed to get IDXGIFactory.");

        // Create Swap Chain

        ThrowIfFailed(dxgi_factory.Get()->CreateSwapChain(d3d.device.Get(), &swap_chain_desc, d3d.swap_chain.GetAddressOf()),
                      h_wnd, "Failed to create IDXGISwapChain.");

        // Prevent DXGI from intercepting Alt+Enter

        ThrowIfFailed(dxgi_factory.Get()->MakeWindowAssociation(h_wnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER),
                      h_wnd, "Failed to call MakeWindowAssociation.");
    }

    // Create D3D Debug device if in debug mode

#if DEBUG_D3D
    ThrowIfFailed(d3d.device->QueryInterface(__uuidof(ID3D11Debug), (void **) d3d.debug.GetAddressOf()),
                  h_wnd, "Failed to get ID3D11Debug device.");
#endif

    // Create views

    create_render_target_views(DESIRED_SCREEN_WIDTH, DESIRED_SCREEN_HEIGHT);

    // Create main vertex buffer

    D3D11_BUFFER_DESC vertex_buffer_desc;
    ZeroMemory(&vertex_buffer_desc, sizeof(D3D11_BUFFER_DESC));

    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_buffer_desc.ByteWidth = 256 * 26 * 3 * sizeof(float); // Same as buf_vbo size in gfx_pc
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertex_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&vertex_buffer_desc, NULL, d3d.vertex_buffer.GetAddressOf()),
                  h_wnd, "Failed to create vertex buffer.");

    // Create per-frame constant buffer

    D3D11_BUFFER_DESC constant_buffer_desc;
    ZeroMemory(&constant_buffer_desc, sizeof(D3D11_BUFFER_DESC));

    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.ByteWidth = sizeof(PerFrameCB);
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&constant_buffer_desc, NULL, d3d.per_frame_cb.GetAddressOf()),
                  h_wnd, "Failed to create per-frame constant buffer.");

    d3d.context->PSSetConstantBuffers(0, 1, d3d.per_frame_cb.GetAddressOf());

    // Create per-draw constant buffer

    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.ByteWidth = sizeof(PerDrawCB);
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constant_buffer_desc.MiscFlags = 0;

    ThrowIfFailed(d3d.device->CreateBuffer(&constant_buffer_desc, NULL, d3d.per_draw_cb.GetAddressOf()),
                  h_wnd, "Failed to create per-draw constant buffer.");

    d3d.context->PSSetConstantBuffers(1, 1, d3d.per_draw_cb.GetAddressOf());

    // Initialize some timer values

    QueryPerformanceFrequency(&frequency);
    accumulated_time.QuadPart = 0;

    // Decide vsync interval

    calculate_sync_interval();

    // Reshape the window according to the config values

    update_screen_settings();

    if (!is_full_screen)
        ShowWindow(h_wnd, SW_SHOW);
}

static void gfx_d3d11_dxgi_shutdown(void) {
    if (d3d.swap_chain) d3d.swap_chain.Get()->SetFullscreenState(false, nullptr);
    if (d3d.swap_chain2) d3d.swap_chain2.Get()->SetFullscreenState(false, nullptr);

    for (unsigned int i = 0; i < sizeof(d3d.shader_program_pool) / sizeof(d3d.shader_program_pool[0]); ++i) {
        d3d.shader_program_pool[i].vertex_shader.Reset();
        d3d.shader_program_pool[i].pixel_shader.Reset();
        d3d.shader_program_pool[i].input_layout.Reset();
        d3d.shader_program_pool[i].blend_state.Reset();
    }

    d3d.rasterizer_state.Reset();
    d3d.backbuffer_view.Reset();
    d3d.depth_stencil_view.Reset();
    d3d.depth_stencil_state.Reset();
    d3d.context.Reset();
    d3d.device.Reset();
    d3d.swap_chain.Reset();
    d3d.swap_chain2.Reset();

    if (h_wnd) {
        DestroyWindow(h_wnd);
        h_wnd = nullptr;
    }
}

static void gfx_d3d11_dxgi_set_keyboard_callbacks(bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode), void (*on_all_keys_up)(void)) {
    d3d.on_key_down = on_key_down;
    d3d.on_key_up = on_key_up;
    d3d.on_all_keys_up = on_all_keys_up;
}

static void gfx_d3d11_dxgi_main_loop(void (*run_one_game_iter)(void)) {
    MSG msg = { 0 };

    bool quit = false;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            quit = true;
        }
    }

    if (quit) {
        return;
    }

    if (IsIconic(h_wnd)) {
        Sleep(50);
        return;
    }

    d3d.run_one_game_iter = run_one_game_iter;

    if (sync_interval == 0) {
        LARGE_INTEGER current_time;
        QueryPerformanceCounter(&current_time);

        LARGE_INTEGER elapsed_time_microseconds;
        elapsed_time_microseconds.QuadPart = current_time.QuadPart - last_time.QuadPart;
        elapsed_time_microseconds.QuadPart *= 1000000;
        elapsed_time_microseconds.QuadPart /= frequency.QuadPart;

        accumulated_time.QuadPart += elapsed_time_microseconds.QuadPart;
        last_time = current_time;

        const uint32_t FRAME_TIME = 1000000 / 30;

        if (accumulated_time.QuadPart >= FRAME_TIME) {
            accumulated_time.QuadPart %= FRAME_TIME;

            if (lower_latency) {
                WaitForSingleObjectEx(d3d.frame_latency_waitable_object, 1000, true);
            }

            if (d3d.run_one_game_iter != nullptr) {
                d3d.run_one_game_iter();
            }

            if (lower_latency) {
                d3d.swap_chain2->Present(1, 0);
            } else {
                d3d.swap_chain->Present(1, 0);
            }
        } else {
            Sleep(1);
        }
    } else {
        if (lower_latency) {
            WaitForSingleObjectEx(d3d.frame_latency_waitable_object, 1000, true);
        }

        if (d3d.run_one_game_iter != nullptr) {
            d3d.run_one_game_iter();
        }

        if (lower_latency) {
            d3d.swap_chain2->Present(sync_interval, 0);
        } else {
            d3d.swap_chain->Present(sync_interval, 0);
        }
    }
}

static void gfx_d3d11_dxgi_get_dimensions(uint32_t *width, uint32_t *height) {
    *width = d3d.current_width;
    *height = d3d.current_height;
}

static void gfx_d3d11_dxgi_handle_events(void) {
}

static bool gfx_d3d11_dxgi_start_frame(void) {
    return true;
}

static void gfx_d3d11_dxgi_swap_buffers_begin(void) {
}

static void gfx_d3d11_dxgi_swap_buffers_end(void) {
}

double gfx_d3d11_dxgi_get_time(void) {
    return 0.0;
}

static bool gfx_d3d11_z_is_from_0_to_1(void) {
    return true;
}

static void gfx_d3d11_unload_shader(struct ShaderProgram *old_prg) {
}

static void gfx_d3d11_load_shader(struct ShaderProgram *new_prg) {
    d3d.shader_program = new_prg;
}

static struct ShaderProgram *gfx_d3d11_create_and_load_new_shader(uint32_t shader_id) {
    CCFeatures cc_features;
    get_cc_features(shader_id, &cc_features);

    char buf[4096];
    size_t len = 0;
    size_t num_floats = 4;

    // Pixel shader input struct

    append_line(buf, &len, "struct PSInput {");
    append_line(buf, &len, "    float4 position : SV_POSITION;");

    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        append_line(buf, &len, "    float2 uv : TEXCOORD;");
        num_floats += 2;
    }

    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "    float4 screenPos : TEXCOORD1;");
    }

    if (cc_features.opt_fog) {
        append_line(buf, &len, "    float4 fog : FOG;");
        num_floats += 4;
    }
    for (uint32_t i = 0; i < cc_features.num_inputs; i++) {
        len += sprintf(buf + len, "    float%d input%d : INPUT%d;\r\n", cc_features.opt_alpha ? 4 : 3, i + 1, i);
        num_floats += cc_features.opt_alpha ? 4 : 3;
    }
    append_line(buf, &len, "};");

    // Textures and samplers

    if (cc_features.used_textures[0]) {
        append_line(buf, &len, "Texture2D g_texture0 : register(t0);");
        append_line(buf, &len, "SamplerState g_sampler0 : register(s0);");
    }
    if (cc_features.used_textures[1]) {
        append_line(buf, &len, "Texture2D g_texture1 : register(t1);");
        append_line(buf, &len, "SamplerState g_sampler1 : register(s1);");
    }

    // Constant buffer and random function

    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "cbuffer PerFrameCB : register(b0) {");
        append_line(buf, &len, "    uint noise_frame;");
        append_line(buf, &len, "    float2 noise_scale;");
        append_line(buf, &len, "}");

        append_line(buf, &len, "float random(in float3 value) {");
        append_line(buf, &len, "    float random = dot(value, float3(12.9898, 78.233, 37.719));");
        append_line(buf, &len, "    return frac(sin(random) * 143758.5453);");
        append_line(buf, &len, "}");
    }

    // 3 point texture filtering
    // Original author: ArthurCarvalho
    // Based on GLSL implementation by twinaphex, mupen64plus-libretro project.

    if (configFiltering == 2) {
        if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
            append_line(buf, &len, "cbuffer PerDrawCB : register(b1) {");
            append_line(buf, &len, "    struct {");
            append_line(buf, &len, "        uint width;");
            append_line(buf, &len, "        uint height;");
            append_line(buf, &len, "        bool linear_filtering;");
            append_line(buf, &len, "    } textures[2];");
            append_line(buf, &len, "}");
            append_line(buf, &len, "#define TEX_OFFSET(tex, tSampler, texCoord, off, texSize) tex.Sample(tSampler, texCoord - off / texSize)");
            append_line(buf, &len, "float4 tex2D3PointFilter(in Texture2D tex, in SamplerState tSampler, in float2 texCoord, in float2 texSize) {");
            append_line(buf, &len, "    float2 offset = frac(texCoord * texSize - float2(0.5, 0.5));");
            append_line(buf, &len, "    offset -= step(1.0, offset.x + offset.y);");
            append_line(buf, &len, "    float4 c0 = TEX_OFFSET(tex, tSampler, texCoord, offset, texSize);");
            append_line(buf, &len, "    float4 c1 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x - sign(offset.x), offset.y), texSize);");
            append_line(buf, &len, "    float4 c2 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x, offset.y - sign(offset.y)), texSize);");
            append_line(buf, &len, "    return c0 + abs(offset.x)*(c1-c0) + abs(offset.y)*(c2-c0);");
            append_line(buf, &len, "}");
        }
    }

    // Vertex shader

    append_str(buf, &len, "PSInput VSMain(float4 position : POSITION");
    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        append_str(buf, &len, ", float2 uv : TEXCOORD");
    }
    if (cc_features.opt_fog) {
        append_str(buf, &len, ", float4 fog : FOG");
    }
    for (uint32_t i = 0; i < cc_features.num_inputs; i++) {
        len += sprintf(buf + len, ", float%d input%d : INPUT%d", cc_features.opt_alpha ? 4 : 3, i + 1, i);
    }
    append_line(buf, &len, ") {");
    append_line(buf, &len, "    PSInput result;");
    append_line(buf, &len, "    result.position = position;");
    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "    result.screenPos = position;");
    }
    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        append_line(buf, &len, "    result.uv = uv;");
    }
    if (cc_features.opt_fog) {
        append_line(buf, &len, "    result.fog = fog;");
    }
    for (uint32_t i = 0; i < cc_features.num_inputs; i++) {
        len += sprintf(buf + len, "    result.input%d = input%d;\r\n", i + 1, i + 1);
    }
    append_line(buf, &len, "    return result;");
    append_line(buf, &len, "}");

    // Pixel shader

    append_line(buf, &len, "float4 PSMain(PSInput input) : SV_TARGET {");
    if (cc_features.used_textures[0]) {
        if (configFiltering == 2) {
            append_line(buf, &len, "    float4 texVal0;");
            append_line(buf, &len, "    if (textures[0].linear_filtering)");
            append_line(buf, &len, "        texVal0 = tex2D3PointFilter(g_texture0, g_sampler0, input.uv, float2(textures[0].width, textures[0].height));");
            append_line(buf, &len, "    else");
            append_line(buf, &len, "        texVal0 = g_texture0.Sample(g_sampler0, input.uv);");
        } else {
            append_line(buf, &len, "    float4 texVal0 = g_texture0.Sample(g_sampler0, input.uv);");
        }
    }
    if (cc_features.used_textures[1]) {
        if (configFiltering == 2) {
            append_line(buf, &len, "    float4 texVal1;");
            append_line(buf, &len, "    if (textures[1].linear_filtering)");
            append_line(buf, &len, "        texVal1 = tex2D3PointFilter(g_texture1, g_sampler1, input.uv, float2(textures[1].width, textures[1].height));");
            append_line(buf, &len, "    else");
            append_line(buf, &len, "        texVal1 = g_texture1.Sample(g_sampler1, input.uv);");
        } else {
            append_line(buf, &len, "    float4 texVal1 = g_texture1.Sample(g_sampler1, input.uv);");
        }
    }

    append_str(buf, &len, cc_features.opt_alpha ? "    float4 texel = " : "    float3 texel = ");
    if (!cc_features.color_alpha_same && cc_features.opt_alpha) {
        append_str(buf, &len, "float4(");
        append_formula(buf, &len, cc_features.c, cc_features.do_single[0], cc_features.do_multiply[0], cc_features.do_mix[0], false, false, true);
        append_str(buf, &len, ", ");
        append_formula(buf, &len, cc_features.c, cc_features.do_single[1], cc_features.do_multiply[1], cc_features.do_mix[1], true, true, true);
        append_str(buf, &len, ")");
    } else {
        append_formula(buf, &len, cc_features.c, cc_features.do_single[0], cc_features.do_multiply[0], cc_features.do_mix[0], cc_features.opt_alpha, false, cc_features.opt_alpha);
    }
    append_line(buf, &len, ";");

    if (cc_features.opt_texture_edge && cc_features.opt_alpha) {
        append_line(buf, &len, "    if (texel.a > 0.3) texel.a = 1.0; else discard;");
    }
    // TODO discard if alpha is 0?
    if (cc_features.opt_fog) {
        if (cc_features.opt_alpha) {
            append_line(buf, &len, "    texel = float4(lerp(texel.rgb, input.fog.rgb, input.fog.a), texel.a);");
        } else {
            append_line(buf, &len, "    texel = lerp(texel, input.fog.rgb, input.fog.a);");
        }
    }

    if (cc_features.opt_alpha && cc_features.opt_noise) {
        append_line(buf, &len, "    float2 coords = (input.screenPos.xy / input.screenPos.w) * noise_scale;");
        append_line(buf, &len, "    texel.a *= round(random(float3(floor(coords), noise_frame)));");
    }

    if (cc_features.opt_alpha) {
        append_line(buf, &len, "    return texel;");
    } else {
        append_line(buf, &len, "    return float4(texel, 1.0);");
    }
    append_line(buf, &len, "}");

    ComPtr<ID3DBlob> vs, ps;
    ComPtr<ID3DBlob> error_blob;

#if DEBUG_D3D
    UINT compile_flags = D3DCOMPILE_DEBUG;
#else
    UINT compile_flags = D3DCOMPILE_OPTIMIZATION_LEVEL2;
#endif

    HRESULT hr = D3DCompile(buf, len, nullptr, nullptr, nullptr, "VSMain", "vs_4_0_level_9_1", compile_flags, 0, vs.GetAddressOf(), error_blob.GetAddressOf());

    if (FAILED(hr))
        sys_fatal("%s", (char *) error_blob->GetBufferPointer());

    hr = D3DCompile(buf, len, nullptr, nullptr, nullptr, "PSMain", "ps_4_0_level_9_1", compile_flags, 0, ps.GetAddressOf(), error_blob.GetAddressOf());

    if (FAILED(hr))
        sys_fatal("%s", (char *) error_blob->GetBufferPointer());

    struct ShaderProgram *prg = &d3d.shader_program_pool[d3d.shader_program_pool_size++];

    ThrowIfFailed(d3d.device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, prg->vertex_shader.GetAddressOf()));
    ThrowIfFailed(d3d.device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, prg->pixel_shader.GetAddressOf()));

    // Input Layout

    D3D11_INPUT_ELEMENT_DESC ied[7];
    uint8_t ied_index = 0;
    ied[ied_index++] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    if (cc_features.used_textures[0] || cc_features.used_textures[1]) {
        ied[ied_index++] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    }
    if (cc_features.opt_fog) {
        ied[ied_index++] = { "FOG", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    }
    for (unsigned int i = 0; i < cc_features.num_inputs; i++) {
        DXGI_FORMAT format = cc_features.opt_alpha ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32G32B32_FLOAT;
        ied[ied_index++] = { "INPUT", i, format, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    }

    ThrowIfFailed(d3d.device->CreateInputLayout(ied, ied_index, vs->GetBufferPointer(), vs->GetBufferSize(), prg->input_layout.GetAddressOf()));

    // Blend state

    D3D11_BLEND_DESC blend_desc;
    ZeroMemory(&blend_desc, sizeof(D3D11_BLEND_DESC));

    if (cc_features.opt_alpha) {
        blend_desc.RenderTarget[0].BlendEnable = true;
        blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    } else {
        blend_desc.RenderTarget[0].BlendEnable = false;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    ThrowIfFailed(d3d.device->CreateBlendState(&blend_desc, prg->blend_state.GetAddressOf()));

    // Save some values

    prg->shader_id = shader_id;
    prg->num_inputs = cc_features.num_inputs;
    prg->num_floats = num_floats;
    prg->used_textures[0] = cc_features.used_textures[0];
    prg->used_textures[1] = cc_features.used_textures[1];

    return d3d.shader_program = prg;
}

static struct ShaderProgram *gfx_d3d11_lookup_shader(uint32_t shader_id) {
    for (size_t i = 0; i < d3d.shader_program_pool_size; i++) {
        if (d3d.shader_program_pool[i].shader_id == shader_id) {
            return &d3d.shader_program_pool[i];
        }
    }
    return NULL;
}

static void gfx_d3d11_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    *num_inputs = prg->num_inputs;
    used_textures[0] = prg->used_textures[0];
    used_textures[1] = prg->used_textures[1];
}

static uint32_t gfx_d3d11_new_texture(void) {
    d3d.textures.resize(d3d.textures.size() + 1);
    return (uint32_t)(d3d.textures.size() - 1);
}

static void gfx_d3d11_select_texture(int tile, uint32_t texture_id) {
    d3d.current_tile = tile;
    d3d.current_texture_ids[tile] = texture_id;
}

static D3D11_TEXTURE_ADDRESS_MODE gfx_cm_to_d3d11(uint32_t val) {
    if (val & G_TX_CLAMP) {
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    }
    return (val & G_TX_MIRROR) ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
}

static void gfx_d3d11_upload_texture(uint8_t *rgba32_buf, int width, int height) {
    // Create texture

    D3D11_TEXTURE2D_DESC texture_desc;
    ZeroMemory(&texture_desc, sizeof(D3D11_TEXTURE2D_DESC));

    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = 0; // D3D11_RESOURCE_MISC_GENERATE_MIPS ?
    texture_desc.ArraySize = 1;
    texture_desc.MipLevels = 1;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA resource_data;
    resource_data.pSysMem = rgba32_buf;
    resource_data.SysMemPitch = width * 4;
    resource_data.SysMemSlicePitch = resource_data.SysMemPitch * height;

    ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(d3d.device->CreateTexture2D(&texture_desc, &resource_data, texture.GetAddressOf()));

    // Create shader resource view from texture

    D3D11_SHADER_RESOURCE_VIEW_DESC resource_view_desc;
    ZeroMemory(&resource_view_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    resource_view_desc.Format = texture_desc.Format;
    resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resource_view_desc.Texture2D.MostDetailedMip = 0;
    resource_view_desc.Texture2D.MipLevels = -1;

    TextureData *texture_data = &d3d.textures[d3d.current_texture_ids[d3d.current_tile]];
    texture_data->width = width;
    texture_data->height = height;

    ThrowIfFailed(d3d.device->CreateShaderResourceView(texture.Get(), &resource_view_desc, texture_data->resource_view.GetAddressOf()));
}

static void gfx_d3d11_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
    D3D11_SAMPLER_DESC sampler_desc;
    ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));

    if (configFiltering == 2)
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    else
        sampler_desc.Filter = linear_filter ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;

    sampler_desc.AddressU = gfx_cm_to_d3d11(cms);
    sampler_desc.AddressV = gfx_cm_to_d3d11(cmt);
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    TextureData *texture_data = &d3d.textures[d3d.current_texture_ids[tile]];
    texture_data->linear_filtering = linear_filter;

    // This function is called twice per texture, the first one only to set default values.
    // Maybe that could be skipped? Anyway, make sure to release the first default sampler
    // state before setting the actual one.
    texture_data->sampler_state.Reset();

    ThrowIfFailed(d3d.device->CreateSamplerState(&sampler_desc, texture_data->sampler_state.GetAddressOf()));
}

static void gfx_d3d11_set_depth_test(bool depth_test) {
    d3d.depth_test = depth_test;
}

static void gfx_d3d11_set_depth_mask(bool depth_mask) {
    d3d.depth_mask = depth_mask;
}

static void gfx_d3d11_set_zmode_decal(bool zmode_decal) {
    d3d.zmode_decal = zmode_decal;
}

static void gfx_d3d11_set_viewport(int x, int y, int width, int height) {
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = x;
    viewport.TopLeftY = d3d.current_height - y - height;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    d3d.context->RSSetViewports(1, &viewport);
}

static void gfx_d3d11_set_scissor(int x, int y, int width, int height) {
    D3D11_RECT rect;
    rect.left = x;
    rect.top = d3d.current_height - y - height;
    rect.right = x + width;
    rect.bottom = d3d.current_height - y;

    d3d.context->RSSetScissorRects(1, &rect);
}

static void gfx_d3d11_set_use_alpha(bool use_alpha) {
    // Already part of the pipeline state from shader info
}

static void gfx_d3d11_draw_triangles(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {

    if (d3d.last_depth_test != d3d.depth_test || d3d.last_depth_mask != d3d.depth_mask) {
        d3d.last_depth_test = d3d.depth_test;
        d3d.last_depth_mask = d3d.depth_mask;

        d3d.depth_stencil_state.Reset();

        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
        ZeroMemory(&depth_stencil_desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

        depth_stencil_desc.DepthEnable = d3d.depth_test;
        depth_stencil_desc.DepthWriteMask = d3d.depth_mask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depth_stencil_desc.StencilEnable = false;

        ThrowIfFailed(d3d.device->CreateDepthStencilState(&depth_stencil_desc, d3d.depth_stencil_state.GetAddressOf()));
        d3d.context->OMSetDepthStencilState(d3d.depth_stencil_state.Get(), 0);
    }

    if (d3d.last_zmode_decal != d3d.zmode_decal) {
        d3d.last_zmode_decal = d3d.zmode_decal;

        d3d.rasterizer_state.Reset();

        D3D11_RASTERIZER_DESC rasterizer_desc;
        ZeroMemory(&rasterizer_desc, sizeof(D3D11_RASTERIZER_DESC));

        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = true;
        rasterizer_desc.DepthBias = 0;
        rasterizer_desc.SlopeScaledDepthBias = d3d.zmode_decal ? -2.0f : 0.0f;
        rasterizer_desc.DepthBiasClamp = 0.0f;
        rasterizer_desc.DepthClipEnable = true;
        rasterizer_desc.ScissorEnable = true;
        rasterizer_desc.MultisampleEnable = false;
        rasterizer_desc.AntialiasedLineEnable = false;

        ThrowIfFailed(d3d.device->CreateRasterizerState(&rasterizer_desc, d3d.rasterizer_state.GetAddressOf()));
        d3d.context->RSSetState(d3d.rasterizer_state.Get());
    }

    bool textures_changed = false;

    for (int i = 0; i < 2; i++) {
        if (d3d.shader_program->used_textures[i]) {
            if (d3d.last_resource_views[i].Get() != d3d.textures[d3d.current_texture_ids[i]].resource_view.Get()) {
                d3d.last_resource_views[i] = d3d.textures[d3d.current_texture_ids[i]].resource_view.Get();
                d3d.context->PSSetShaderResources(i, 1, d3d.textures[d3d.current_texture_ids[i]].resource_view.GetAddressOf());

                if (configFiltering == 2) {
                    d3d.per_draw_cb_data.textures[i].width = d3d.textures[d3d.current_texture_ids[i]].width;
                    d3d.per_draw_cb_data.textures[i].height = d3d.textures[d3d.current_texture_ids[i]].height;
                    d3d.per_draw_cb_data.textures[i].linear_filtering = d3d.textures[d3d.current_texture_ids[i]].linear_filtering;
                    textures_changed = true;
                }

                if (d3d.last_sampler_states[i].Get() != d3d.textures[d3d.current_texture_ids[i]].sampler_state.Get()) {
                    d3d.last_sampler_states[i] = d3d.textures[d3d.current_texture_ids[i]].sampler_state.Get();
                    d3d.context->PSSetSamplers(i, 1, d3d.textures[d3d.current_texture_ids[i]].sampler_state.GetAddressOf());
                }
            }
        }
    }

    // Set per-draw constant buffer

    if (textures_changed) {
        D3D11_MAPPED_SUBRESOURCE ms;
        ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
        d3d.context->Map(d3d.per_draw_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, &d3d.per_draw_cb_data, sizeof(PerDrawCB));
        d3d.context->Unmap(d3d.per_draw_cb.Get(), 0);
    }

    // Set vertex buffer data

    D3D11_MAPPED_SUBRESOURCE ms;
    ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3d.context->Map(d3d.vertex_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, buf_vbo, buf_vbo_len * sizeof(float));
    d3d.context->Unmap(d3d.vertex_buffer.Get(), 0);

    uint32_t stride = d3d.shader_program->num_floats * sizeof(float);
    uint32_t offset = 0;

    if (d3d.last_vertex_buffer_stride != stride) {
        d3d.last_vertex_buffer_stride = stride;
        d3d.context->IASetVertexBuffers(0, 1, d3d.vertex_buffer.GetAddressOf(), &stride, &offset);
    }

    if (d3d.last_shader_program != d3d.shader_program) {
        d3d.last_shader_program = d3d.shader_program;
        d3d.context->IASetInputLayout(d3d.shader_program->input_layout.Get());
        d3d.context->VSSetShader(d3d.shader_program->vertex_shader.Get(), 0, 0);
        d3d.context->PSSetShader(d3d.shader_program->pixel_shader.Get(), 0, 0);

        if (d3d.last_blend_state.Get() != d3d.shader_program->blend_state.Get()) {
            d3d.last_blend_state = d3d.shader_program->blend_state.Get();
            d3d.context->OMSetBlendState(d3d.shader_program->blend_state.Get(), 0, 0xFFFFFFFF);
        }
    }

    if (d3d.last_primitive_topology != D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
        d3d.last_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        d3d.context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    d3d.context->Draw(buf_vbo_num_tris * 3, 0);
}

static void gfx_d3d11_init(void) { }

static void gfx_d3d11_shutdown(void) { }

static void gfx_d3d11_start_frame(void) {
    // Set render targets

    d3d.context->OMSetRenderTargets(1, d3d.backbuffer_view.GetAddressOf(), d3d.depth_stencil_view.Get());

    // Clear render targets

    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    d3d.context->ClearRenderTargetView(d3d.backbuffer_view.Get(), clearColor);
    d3d.context->ClearDepthStencilView(d3d.depth_stencil_view.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Set per-frame constant buffer

    d3d.per_frame_cb_data.noise_frame++;
    if (d3d.per_frame_cb_data.noise_frame > 150) {
        // No high values, as noise starts to look ugly
        d3d.per_frame_cb_data.noise_frame = 0;
    }

    d3d.per_frame_cb_data.noise_scale_x = (float) d3d.current_width;
    d3d.per_frame_cb_data.noise_scale_y = (float) d3d.current_height;

    D3D11_MAPPED_SUBRESOURCE ms;
    ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3d.context->Map(d3d.per_frame_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, &d3d.per_frame_cb_data, sizeof(PerFrameCB));
    d3d.context->Unmap(d3d.per_frame_cb.Get(), 0);
}

struct GfxRenderingAPI gfx_d3d11_api = {
    gfx_d3d11_z_is_from_0_to_1,
    gfx_d3d11_unload_shader,
    gfx_d3d11_load_shader,
    gfx_d3d11_create_and_load_new_shader,
    gfx_d3d11_lookup_shader,
    gfx_d3d11_shader_get_info,
    gfx_d3d11_new_texture,
    gfx_d3d11_select_texture,
    gfx_d3d11_upload_texture,
    gfx_d3d11_set_sampler_parameters,
    gfx_d3d11_set_depth_test,
    gfx_d3d11_set_depth_mask,
    gfx_d3d11_set_zmode_decal,
    gfx_d3d11_set_viewport,
    gfx_d3d11_set_scissor,
    gfx_d3d11_set_use_alpha,
    gfx_d3d11_draw_triangles,
    gfx_d3d11_init,
    gfx_d3d11_start_frame,
    gfx_d3d11_shutdown,
};

struct GfxWindowManagerAPI gfx_dxgi = {
    gfx_d3d11_dxgi_init,
    gfx_d3d11_dxgi_set_keyboard_callbacks,
    gfx_d3d11_dxgi_main_loop,
    gfx_d3d11_dxgi_get_dimensions,
    gfx_d3d11_dxgi_handle_events,
    gfx_d3d11_dxgi_start_frame,
    gfx_d3d11_dxgi_swap_buffers_begin,
    gfx_d3d11_dxgi_swap_buffers_end,
    gfx_d3d11_dxgi_get_time,
    gfx_d3d11_dxgi_shutdown,
};

#else

#error "D3D11 is only supported on Windows"

#endif // _WIN32

#endif // RAPI_D3D11
