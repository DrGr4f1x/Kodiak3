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

#include "Input.h"


using namespace Kodiak;
using namespace std;


Input Kodiak::g_input;


namespace
{
float FilterAnalogInput(int val, int deadZone)
{
	if (val < 0)
	{
		if (val > -deadZone)
		{
			return 0.0f;
		}
		else
		{
			return (val + deadZone) / (32768.0f - deadZone);
		}
	}
	else
	{
		if (val < deadZone)
		{
			return 0.0f;
		}
		else
		{
			return (val - deadZone) / (32767.0f - deadZone);
		}
	}
}
} // anonymous namespace


void Input::Initialize(HWND hwnd)
{
	m_hwnd = hwnd;

	KbmBuildKeyMapping();

	ZeroMemory(m_buttons, sizeof(m_buttons));
	ZeroMemory(m_analogs, sizeof(m_analogs));

	if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_di, nullptr)))
	{
		assert_msg(false, "DirectInput8 initialization failed.");
	}

	if (FAILED(m_di->CreateDevice(GUID_SysKeyboard, &m_keyboard, nullptr)))
	{
		assert_msg(false, "Keyboard CreateDevice failed.");
	}
	if (FAILED(m_keyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		assert_msg(false, "Keyboard SetDataFormat failed.");
	}
	if (FAILED(m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		assert_msg(false, "Keyboard SetCooperativeLevel failed.");
	}

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 10;

	if (FAILED(m_keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		assert_msg(false, "Keyboard set buffer size failed.");
	}

	if (FAILED(m_di->CreateDevice(GUID_SysMouse, &m_mouse, nullptr)))
	{
		assert_msg(false, "Mouse CreateDevice failed.");
	}
	if (FAILED(m_mouse->SetDataFormat(&c_dfDIMouse2)))
	{
		assert_msg(false, "Mouse SetDataFormat failed.");
	}
	if (FAILED(m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
	{
		assert_msg(false, "Mouse SetCooperativeLevel failed.");
	}

	KbmZeroInputs();
}


void Input::Shutdown()
{
	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = nullptr;
	}

	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = nullptr;
	}

	if (m_di)
	{
		m_di->Release();
		m_di = nullptr;
	}
}


void Input::Update(float deltaT)
{
	memcpy(m_buttons[1], m_buttons[0], sizeof(m_buttons[0]));
	memset(m_buttons[0], 0, sizeof(m_buttons[0]));
	memset(m_analogs, 0, sizeof(m_analogs));

	XINPUT_STATE newInputState;
	if (ERROR_SUCCESS == XInputGetState(0, &newInputState))
	{
		if (newInputState.Gamepad.wButtons & (1 << 0)) m_buttons[0][(int)DigitalInput::kDPadUp] = true;
		if (newInputState.Gamepad.wButtons & (1 << 1)) m_buttons[0][(int)DigitalInput::kDPadDown] = true;
		if (newInputState.Gamepad.wButtons & (1 << 2)) m_buttons[0][(int)DigitalInput::kDPadLeft] = true;
		if (newInputState.Gamepad.wButtons & (1 << 3)) m_buttons[0][(int)DigitalInput::kDPadRight] = true;
		if (newInputState.Gamepad.wButtons & (1 << 4)) m_buttons[0][(int)DigitalInput::kStartButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 5)) m_buttons[0][(int)DigitalInput::kBackButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 6)) m_buttons[0][(int)DigitalInput::kLThumbClick] = true;
		if (newInputState.Gamepad.wButtons & (1 << 7)) m_buttons[0][(int)DigitalInput::kRThumbClick] = true;
		if (newInputState.Gamepad.wButtons & (1 << 8)) m_buttons[0][(int)DigitalInput::kLShoulder] = true;
		if (newInputState.Gamepad.wButtons & (1 << 9)) m_buttons[0][(int)DigitalInput::kRShoulder] = true;
		if (newInputState.Gamepad.wButtons & (1 << 12)) m_buttons[0][(int)DigitalInput::kAButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 13)) m_buttons[0][(int)DigitalInput::kBButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 14)) m_buttons[0][(int)DigitalInput::kXButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 15)) m_buttons[0][(int)DigitalInput::kYButton] = true;

		SetAnalog(AnalogInput::kAnalogLeftTrigger, newInputState.Gamepad.bLeftTrigger / 255.0f);
		SetAnalog(AnalogInput::kAnalogRightTrigger, newInputState.Gamepad.bRightTrigger / 255.0f);
		SetAnalog(AnalogInput::kAnalogLeftStickX, FilterAnalogInput(newInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE));
		SetAnalog(AnalogInput::kAnalogLeftStickY, FilterAnalogInput(newInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE));
		SetAnalog(AnalogInput::kAnalogRightStickX, FilterAnalogInput(newInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE));
		SetAnalog(AnalogInput::kAnalogRightStickY, FilterAnalogInput(newInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE));
	}

	KbmUpdate();

	for (uint32_t i = 0; i < (uint32_t)DigitalInput::kNumKeys; ++i)
	{
		m_buttons[0][i] = (m_keybuffer[m_dxKeyMapping[i]] & 0x80) != 0;
	}

	for (uint32_t i = 0; i < 8; ++i)
	{
		if (m_mouseState.rgbButtons[i] > 0) m_buttons[0][(int)DigitalInput::kMouse0 + i] = true;
	}

	SetAnalog(AnalogInput::kAnalogMouseX, (float)m_mouseState.lX * .0018f);
	SetAnalog(AnalogInput::kAnalogMouseY, (float)m_mouseState.lY * -.0018f);

	if (m_mouseState.lZ > 0)
	{
		SetAnalog(AnalogInput::kAnalogMouseScroll, 1.0f);
	}
	else if (m_mouseState.lZ < 0)
	{
		SetAnalog(AnalogInput::kAnalogMouseScroll, -1.0f);
	}
}


bool Input::IsAnyPressed() const
{
	return m_buttons[0] != 0;
}


bool Input::IsPressed(DigitalInput di) const
{
	int index = static_cast<int>(di);
	return m_buttons[0][index];
}


bool Input::IsFirstPressed(DigitalInput di) const
{
	int index = static_cast<int>(di);
	return m_buttons[0][index] && !m_buttons[1][index];
}


bool Input::IsReleased(DigitalInput di) const
{
	int index = static_cast<int>(di);
	return !m_buttons[0][index];
}


bool Input::IsFirstReleased(DigitalInput di) const
{
	int index = static_cast<int>(di);
	return !m_buttons[0][index] && m_buttons[1][index];
}


float Input::GetDurationPressed(DigitalInput di) const
{
	int index = static_cast<int>(di);
	return m_holdDuration[index];
}


float Input::GetAnalogInput(AnalogInput ai) const
{
	int index = static_cast<int>(ai);
	return m_analogs[index];
}


float Input::GetTimeCorrectedAnalogInput(AnalogInput ai) const
{
	int index = static_cast<int>(ai);
	return m_analogsTC[index];
}


void Input::KbmBuildKeyMapping()
{
	m_dxKeyMapping[(int)DigitalInput::kKey_escape] = 1;
	m_dxKeyMapping[(int)DigitalInput::kKey_1] = 2;
	m_dxKeyMapping[(int)DigitalInput::kKey_2] = 3;
	m_dxKeyMapping[(int)DigitalInput::kKey_3] = 4;
	m_dxKeyMapping[(int)DigitalInput::kKey_4] = 5;
	m_dxKeyMapping[(int)DigitalInput::kKey_5] = 6;
	m_dxKeyMapping[(int)DigitalInput::kKey_6] = 7;
	m_dxKeyMapping[(int)DigitalInput::kKey_7] = 8;
	m_dxKeyMapping[(int)DigitalInput::kKey_8] = 9;
	m_dxKeyMapping[(int)DigitalInput::kKey_9] = 10;
	m_dxKeyMapping[(int)DigitalInput::kKey_0] = 11;
	m_dxKeyMapping[(int)DigitalInput::kKey_minus] = 12;
	m_dxKeyMapping[(int)DigitalInput::kKey_equals] = 13;
	m_dxKeyMapping[(int)DigitalInput::kKey_back] = 14;
	m_dxKeyMapping[(int)DigitalInput::kKey_tab] = 15;
	m_dxKeyMapping[(int)DigitalInput::kKey_q] = 16;
	m_dxKeyMapping[(int)DigitalInput::kKey_w] = 17;
	m_dxKeyMapping[(int)DigitalInput::kKey_e] = 18;
	m_dxKeyMapping[(int)DigitalInput::kKey_r] = 19;
	m_dxKeyMapping[(int)DigitalInput::kKey_t] = 20;
	m_dxKeyMapping[(int)DigitalInput::kKey_y] = 21;
	m_dxKeyMapping[(int)DigitalInput::kKey_u] = 22;
	m_dxKeyMapping[(int)DigitalInput::kKey_i] = 23;
	m_dxKeyMapping[(int)DigitalInput::kKey_o] = 24;
	m_dxKeyMapping[(int)DigitalInput::kKey_p] = 25;
	m_dxKeyMapping[(int)DigitalInput::kKey_lbracket] = 26;
	m_dxKeyMapping[(int)DigitalInput::kKey_rbracket] = 27;
	m_dxKeyMapping[(int)DigitalInput::kKey_return] = 28;
	m_dxKeyMapping[(int)DigitalInput::kKey_lcontrol] = 29;
	m_dxKeyMapping[(int)DigitalInput::kKey_a] = 30;
	m_dxKeyMapping[(int)DigitalInput::kKey_s] = 31;
	m_dxKeyMapping[(int)DigitalInput::kKey_d] = 32;
	m_dxKeyMapping[(int)DigitalInput::kKey_f] = 33;
	m_dxKeyMapping[(int)DigitalInput::kKey_g] = 34;
	m_dxKeyMapping[(int)DigitalInput::kKey_h] = 35;
	m_dxKeyMapping[(int)DigitalInput::kKey_j] = 36;
	m_dxKeyMapping[(int)DigitalInput::kKey_k] = 37;
	m_dxKeyMapping[(int)DigitalInput::kKey_l] = 38;
	m_dxKeyMapping[(int)DigitalInput::kKey_semicolon] = 39;
	m_dxKeyMapping[(int)DigitalInput::kKey_apostrophe] = 40;
	m_dxKeyMapping[(int)DigitalInput::kKey_grave] = 41;
	m_dxKeyMapping[(int)DigitalInput::kKey_lshift] = 42;
	m_dxKeyMapping[(int)DigitalInput::kKey_backslash] = 43;
	m_dxKeyMapping[(int)DigitalInput::kKey_z] = 44;
	m_dxKeyMapping[(int)DigitalInput::kKey_x] = 45;
	m_dxKeyMapping[(int)DigitalInput::kKey_c] = 46;
	m_dxKeyMapping[(int)DigitalInput::kKey_v] = 47;
	m_dxKeyMapping[(int)DigitalInput::kKey_b] = 48;
	m_dxKeyMapping[(int)DigitalInput::kKey_n] = 49;
	m_dxKeyMapping[(int)DigitalInput::kKey_m] = 50;
	m_dxKeyMapping[(int)DigitalInput::kKey_comma] = 51;
	m_dxKeyMapping[(int)DigitalInput::kKey_period] = 52;
	m_dxKeyMapping[(int)DigitalInput::kKey_slash] = 53;
	m_dxKeyMapping[(int)DigitalInput::kKey_rshift] = 54;
	m_dxKeyMapping[(int)DigitalInput::kKey_multiply] = 55;
	m_dxKeyMapping[(int)DigitalInput::kKey_lalt] = 56;
	m_dxKeyMapping[(int)DigitalInput::kKey_space] = 57;
	m_dxKeyMapping[(int)DigitalInput::kKey_capital] = 58;
	m_dxKeyMapping[(int)DigitalInput::kKey_f1] = 59;
	m_dxKeyMapping[(int)DigitalInput::kKey_f2] = 60;
	m_dxKeyMapping[(int)DigitalInput::kKey_f3] = 61;
	m_dxKeyMapping[(int)DigitalInput::kKey_f4] = 62;
	m_dxKeyMapping[(int)DigitalInput::kKey_f5] = 63;
	m_dxKeyMapping[(int)DigitalInput::kKey_f6] = 64;
	m_dxKeyMapping[(int)DigitalInput::kKey_f7] = 65;
	m_dxKeyMapping[(int)DigitalInput::kKey_f8] = 66;
	m_dxKeyMapping[(int)DigitalInput::kKey_f9] = 67;
	m_dxKeyMapping[(int)DigitalInput::kKey_f10] = 68;
	m_dxKeyMapping[(int)DigitalInput::kKey_numlock] = 69;
	m_dxKeyMapping[(int)DigitalInput::kKey_scroll] = 70;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad7] = 71;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad8] = 72;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad9] = 73;
	m_dxKeyMapping[(int)DigitalInput::kKey_subtract] = 74;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad4] = 75;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad5] = 76;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad6] = 77;
	m_dxKeyMapping[(int)DigitalInput::kKey_add] = 78;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad1] = 79;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad2] = 80;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad3] = 81;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpad0] = 82;
	m_dxKeyMapping[(int)DigitalInput::kKey_decimal] = 83;
	m_dxKeyMapping[(int)DigitalInput::kKey_f11] = 87;
	m_dxKeyMapping[(int)DigitalInput::kKey_f12] = 88;
	m_dxKeyMapping[(int)DigitalInput::kKey_numpadenter] = 156;
	m_dxKeyMapping[(int)DigitalInput::kKey_rcontrol] = 157;
	m_dxKeyMapping[(int)DigitalInput::kKey_divide] = 181;
	m_dxKeyMapping[(int)DigitalInput::kKey_sysrq] = 183;
	m_dxKeyMapping[(int)DigitalInput::kKey_ralt] = 184;
	m_dxKeyMapping[(int)DigitalInput::kKey_pause] = 197;
	m_dxKeyMapping[(int)DigitalInput::kKey_home] = 199;
	m_dxKeyMapping[(int)DigitalInput::kKey_up] = 200;
	m_dxKeyMapping[(int)DigitalInput::kKey_pgup] = 201;
	m_dxKeyMapping[(int)DigitalInput::kKey_left] = 203;
	m_dxKeyMapping[(int)DigitalInput::kKey_right] = 205;
	m_dxKeyMapping[(int)DigitalInput::kKey_end] = 207;
	m_dxKeyMapping[(int)DigitalInput::kKey_down] = 208;
	m_dxKeyMapping[(int)DigitalInput::kKey_pgdn] = 209;
	m_dxKeyMapping[(int)DigitalInput::kKey_insert] = 210;
	m_dxKeyMapping[(int)DigitalInput::kKey_delete] = 211;
	m_dxKeyMapping[(int)DigitalInput::kKey_lwin] = 219;
	m_dxKeyMapping[(int)DigitalInput::kKey_rwin] = 220;
	m_dxKeyMapping[(int)DigitalInput::kKey_apps] = 221;
}


void Input::KbmZeroInputs()
{
	memset(&m_mouseState, 0, sizeof(DIMOUSESTATE2));
	memset(m_keybuffer, 0, sizeof(m_keybuffer));
}


void Input::KbmUpdate()
{
	HWND foreground = GetForegroundWindow();
	bool visible = IsWindowVisible(foreground) != 0;

	if (foreground != m_hwnd // wouldn't be able to acquire
		|| !visible)
	{
		KbmZeroInputs();
	}
	else
	{
		m_mouse->Acquire();
		m_mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &m_mouseState);
		m_keyboard->Acquire();
		m_keyboard->GetDeviceState(sizeof(m_keybuffer), m_keybuffer);
	}
}