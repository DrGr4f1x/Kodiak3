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

	Shutdown();
}


void Application::Initialize()
{
#if ENABLE_VULKAN_VALIDATION || (defined(DX12) && _DEBUG)
	CreateConsole("Validation output");
#endif

	Configure();

	m_graphicsDevice = make_unique<GraphicsDevice>();
	m_graphicsDevice->Initialize(m_name, m_hinst, m_hwnd, m_displayWidth, m_displayHeight);

	Startup();
}


void Application::Finalize()
{
	Shutdown();

	m_graphicsDevice->Destroy();
	m_graphicsDevice.reset();

	g_application = nullptr;
}


bool Application::Tick()
{
	bool res = Update();
	if (res)
	{
		m_graphicsDevice->PrepareFrame();

		Render();

		m_graphicsDevice->SubmitFrame();
	}
	return res;
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