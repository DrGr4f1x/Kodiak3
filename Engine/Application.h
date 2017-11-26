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

namespace Kodiak
{

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
	virtual void Configure() {}
	virtual void Startup() {}
	virtual void Shutdown() {}

	virtual bool Update() { return true; }
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

	const std::string& GetDefaultShaderPath();

protected:
	const std::string m_name;

	uint32_t m_displayWidth{ 1280 };
	uint32_t m_displayHeight{ 720 };

	// Application state
	bool m_paused{ false };
	float m_frameTimer{ 0.0f };
	float m_timer{ 0.0f };
	float m_timerSpeed{ 1.0f };

	HINSTANCE m_hinst{ 0 };
	HWND m_hwnd{ 0 };

	std::unique_ptr<GraphicsDevice> m_graphicsDevice;

private:
	void Initialize();
	void Finalize();
	bool Tick();

	void CreateConsole(const std::string& title);
};

Application* GetApplication();

} // namespace Kodiak