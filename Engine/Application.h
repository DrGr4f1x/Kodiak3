//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

#include "Graphics\Camera.h"
#include "Graphics\Framebuffer.h"
#include "Graphics\Grid.h"
#include "Graphics\UIOverlay.h"


namespace Kodiak
{

// Forward declarations
class GraphicsContext;
class GraphicsDevice;


class Application
{
public:
	Application();
	Application(const std::string& name);
	Application(const std::string& name, uint32_t displayWidth, uint32_t displayHeight);
	virtual ~Application();

	void Run();

	// Override these methods in derived classes
	virtual void Configure();
	virtual void Startup() {}
	virtual void Shutdown() {}

	virtual bool Update() { return true; }
	virtual void UpdateUI() {}
	virtual void Render() {}

	// Accessors
	const HINSTANCE GetHINSTANCE() const { return m_hinst; }
	const HWND GetHWND() const { return m_hwnd; }
	uint32_t GetWidth() const { return m_displayWidth; }
	uint32_t GetHeight() const { return m_displayHeight; }

	// Application state
	bool IsPaused() const { return m_paused; }
	void Pause() { m_paused = true; }
	void Unpause() { m_paused = false; }
	void TogglePause() { m_paused = !m_paused; }
	void Stop() { m_isRunning = false; }

	// Windows event callbacks
	void OnMouseMove(uint32_t x, uint32_t y);

	const std::string& GetDefaultShaderPath();

	// Utility methods to get the default color and depth buffers
	FrameBuffer& GetBackBuffer() const;
	ColorBuffer& GetColorBuffer() const;
	DepthBuffer& GetDepthBuffer() const;
	Format GetColorFormat() const;
	Format GetDepthFormat() const;
	uint32_t GetCurrentFrame() const;
	uint32_t GetFrameNumber() const;

	std::string GetWindowTitle() const;

	// Query external features/state
	bool IsDeveloperModeEnabled() const { return m_isDeveloperModeEnabled; }
	bool IsRenderDocAvailable() const { return m_isRenderDocAvailable; }

protected:
	void PrepareUI();
	void RenderUI(GraphicsContext& context);
	void RenderGrid(GraphicsContext& context);

	void CheckDeveloperMode();
	void CheckRenderDoc();

protected:
	const std::string m_name;

	uint32_t m_displayWidth{ 1280 };
	uint32_t m_displayHeight{ 720 };

	// Application state
	bool m_isRunning{ false };
	bool m_paused{ false };
	bool m_showUI{ true };
	bool m_showGrid{ false };
	float m_frameTimer{ 0.0f };
	float m_appElapsedTime{ 0.0f };
	float m_timer{ 0.0f };
	float m_timerSpeed{ 1.0f };
	uint32_t m_lastFps{ 0 };
	uint32_t m_frameCounter{ 0 };
	std::chrono::time_point<std::chrono::high_resolution_clock> m_appStartTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTimestamp;

	uint32_t m_mouseX{ 0 };
	uint32_t m_mouseY{ 0 };
	bool m_mouseMoveHandled{ false };

	Math::Camera m_camera;

	HINSTANCE	m_hinst{ 0 };
	HWND		m_hwnd{ 0 };

	std::unique_ptr<GraphicsDevice>			m_graphicsDevice;

	std::array<FrameBufferPtr, NumSwapChainBuffers>
											m_defaultFramebuffers;
	DepthBufferPtr							m_defaultDepthBuffer;

	std::unique_ptr<UIOverlay>				m_uiOverlay;
	std::unique_ptr<Grid>					m_grid;

	// External features/state
	bool m_isDeveloperModeEnabled{ false };
	bool m_isRenderDocAvailable{ false };

private:
	void Initialize();
	void Finalize();
	bool Tick();

	void InitFramebuffer();

	void CreateConsole(const std::string& title);
};

Application* GetApplication();

} // namespace Kodiak