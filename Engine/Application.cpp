//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "Application.h"

#include "GraphicsDevice.h"
#include "Input.h"
#include "SwapChain.h"


#pragma comment(lib, "runtimeobject.lib")


using namespace std;
using namespace Kodiak;


namespace
{

Application* g_application{ nullptr };

} // anonymous namespace


Application::Application()
	: m_name("Unnamed")
{}


Application::Application(const string& name)
	: m_name(name)
{}


Application::Application(const string& name, uint32_t displayWidth, uint32_t displayHeight)
	: m_name(name)
	, m_displayWidth(displayWidth)
	, m_displayHeight(displayHeight)
{}


Application::~Application()
{
	Finalize();
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void Application::Run()
{
	Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
	assert_succeeded(InitializeWinRT);

	m_hinst = GetModuleHandle(0);

	string appNameWithAPI = s_apiPrefixString + " " + m_name;

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hinst;
	wcex.hIcon = LoadIcon(m_hinst, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = appNameWithAPI.c_str();
	wcex.hIconSm = LoadIcon(m_hinst, IDI_APPLICATION);
	assert_msg(0 != RegisterClassEx(&wcex), "Unable to register a window");

	// Create window
	RECT rc = { 0, 0, (LONG)m_displayWidth, (LONG)m_displayHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindow(appNameWithAPI.c_str(), appNameWithAPI.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, m_hinst, nullptr);

	assert(m_hwnd != 0);

	g_application = this;

	Initialize();

	ShowWindow(m_hwnd, SW_SHOWDEFAULT);

	do
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;
	} while (Tick());	// Returns false to quit loop

	m_graphicsDevice->WaitForGpuIdle();

	Shutdown();
}


const string& Application::GetDefaultShaderPath()
{
	return s_defaultShaderPath;
}


void Application::Initialize()
{
#if ENABLE_VULKAN_VALIDATION || (defined(DX12) && _DEBUG)
	CreateConsole("Validation output");
#endif

	Configure();

	m_graphicsDevice = make_unique<GraphicsDevice>();
	m_graphicsDevice->Initialize(m_name, m_hinst, m_hwnd, m_displayWidth, m_displayHeight);

	InitFramebuffer();

	g_input.Initialize(m_hwnd);

	Startup();

	m_isRunning = true;
}


void Application::Finalize()
{
	Shutdown();

	m_framebuffers.clear();
	m_depthBuffer.reset();
	m_renderPass.Destroy();

	m_graphicsDevice->Destroy();
	m_graphicsDevice.reset();

	g_input.Shutdown();

	g_application = nullptr;
}


bool Application::Tick()
{
	if (!m_isRunning)
		return false;

	auto timeStart = chrono::high_resolution_clock::now();

	g_input.Update(m_frameTimer);

	// Close on Escape key
	if (g_input.IsFirstPressed(DigitalInput::kKey_escape))
		return false;

	// Pause or resume application
	if (g_input.IsFirstPressed(DigitalInput::kKey_pause))
		TogglePause();

	HandleInputs();

	bool res = Update();
	if (res)
	{
		m_camera.Update();

		m_graphicsDevice->PrepareFrame();

		Render();

		m_graphicsDevice->SubmitFrame();
	}

	auto timeEnd = chrono::high_resolution_clock::now();
	auto timeDiff = chrono::duration<double, std::milli>(timeEnd - timeStart).count();
	m_frameTimer = static_cast<float>(timeDiff * 0.001);

	if (!m_paused)
	{
		m_timer += m_timerSpeed * m_frameTimer;
		if (m_timer > 1.0f)
		{
			m_timer -= 1.0f;
		}
	}

	return res;
}


void Application::InitFramebuffer()
{
	auto swapChain = m_graphicsDevice->GetSwapChain();
	auto colorFormat = swapChain->GetColorFormat();
	auto depthFormat = m_graphicsDevice->GetDepthFormat();
	m_renderPass.AddColorAttachment(colorFormat, ResourceState::Undefined, ResourceState::Present);
	m_renderPass.AddDepthAttachment(depthFormat, ResourceState::Undefined, ResourceState::DepthWrite);
	m_renderPass.Finalize();

	// Depth stencil buffer for swap chain
	m_depthBuffer = make_shared<DepthBuffer>(1.0f);
	m_depthBuffer->Create("Depth Buffer", m_displayWidth, m_displayHeight, m_graphicsDevice->GetDepthFormat());

	// Framebuffers for swap chain
	const uint32_t imageCount = swapChain->GetImageCount();
	m_framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		m_framebuffers[i] = make_shared<FrameBuffer>();
		m_framebuffers[i]->Create(swapChain->GetColorBuffer(i), m_depthBuffer, m_renderPass);
	}
}


void Application::CreateConsole(const string& title)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* stream{ nullptr };
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	freopen_s(&stream, "CONOUT$", "w+", stderr);
	SetConsoleTitle(TEXT(title.c_str()));
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_SIZE:
		//Graphics::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
		break;

	case WM_CLOSE:
		GetApplication()->Stop(); // TODO - Can we detect device removed in Vulkan?  This is hacky...
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


Application* Kodiak::GetApplication()
{
	return g_application;
}