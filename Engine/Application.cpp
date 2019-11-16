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

#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "Input.h"

#include <iostream>

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


void Application::Configure()
{
	// Setup file system
	auto& filesystem = Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("..\\Data");
	filesystem.AddSearchPath("..\\Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("..\\Data\\Textures");
	filesystem.AddSearchPath("..\\Data\\Models");
}


FrameBuffer& Application::GetBackBuffer() const
{
	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();
	return *m_defaultFramebuffers[curFrame];
}


ColorBuffer& Application::GetColorBuffer() const
{
	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();
	return *m_defaultFramebuffers[curFrame]->GetColorBuffer(0);
}


DepthBuffer& Application::GetDepthBuffer() const
{
	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();
	return *m_defaultFramebuffers[curFrame]->GetDepthBuffer();
}


Format Application::GetColorFormat() const
{
	return g_graphicsDevice->GetColorFormat();
}


Format Application::GetDepthFormat() const
{
	return g_graphicsDevice->GetDepthFormat();
}


uint32_t Application::GetCurrentFrame() const
{
	return g_graphicsDevice->GetCurrentBuffer();
}


uint32_t Application::GetFrameNumber() const
{
	return g_graphicsDevice->GetFrameNumber();
}


string Application::GetWindowTitle() const
{
	string title = s_apiPrefixString + " " + m_name + " - " + std::to_string(m_frameCounter) + " fps";

	return title;
}


const string& Application::GetDefaultShaderPath()
{
	return s_defaultShaderPath;
}


void Application::Initialize()
{
#if ENABLE_VULKAN_VALIDATION || (defined(DX12) && _DEBUG)
	//CreateConsole("Validation output");
#endif

	Configure();

	m_graphicsDevice = make_unique<GraphicsDevice>();
	m_graphicsDevice->Initialize(m_name, m_hinst, m_hwnd, m_displayWidth, m_displayHeight);

	InitFramebuffer();

	g_input.Initialize(m_hwnd);

	m_uiOverlay = make_unique<UIOverlay>();
	m_uiOverlay->Startup(GetWidth(), GetHeight());

	Startup();

	m_isRunning = true;
}


void Application::Finalize()
{
	Shutdown();

	m_uiOverlay->Shutdown();
	m_uiOverlay.reset();

	for (int i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_defaultFramebuffers[i] = nullptr;
	}

	m_defaultDepthBuffer.reset();

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

	bool res = Update();
	if (res)
	{
		Render();

		m_graphicsDevice->SubmitFrame();
	}

	++m_frameCounter;

	auto timeEnd = chrono::high_resolution_clock::now();
	auto timeDiff = chrono::duration<double, std::milli>(timeEnd - timeStart).count();
	m_frameTimer = static_cast<float>(timeDiff) / 1000.0f;

	if (!m_paused)
	{
		m_timer += m_timerSpeed * m_frameTimer;
		if (m_timer > 1.0f)
		{
			m_timer -= 1.0f;
		}
	}

	float fpsTimer = (float)(std::chrono::duration<double, std::milli>(timeEnd - m_lastTimestamp).count());
	if (fpsTimer > 1000.0f)
	{
		m_lastFps = static_cast<uint32_t>((float)m_frameCounter * (1000.0f / fpsTimer));

		std::string windowTitle = GetWindowTitle();
		SetWindowText(m_hwnd, windowTitle.c_str());

		m_frameCounter = 0;
		m_lastTimestamp = timeEnd;
	}

	return res;
}


void Application::InitFramebuffer()
{
	// Depth stencil buffer for swap chain
	m_defaultDepthBuffer = make_shared<DepthBuffer>(1.0f);
	m_defaultDepthBuffer->Create("Depth Buffer", m_displayWidth, m_displayHeight, m_graphicsDevice->GetDepthFormat());

	// Framebuffers for swap chain
	for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_defaultFramebuffers[i] = make_shared<FrameBuffer>();
		
		m_defaultFramebuffers[i]->SetColorBuffer(0, m_graphicsDevice->GetBackBuffer(i));
		m_defaultFramebuffers[i]->SetDepthBuffer(m_defaultDepthBuffer);
		m_defaultFramebuffers[i]->Finalize();
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