#include "GarrysMod/Lua/Interface.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <dinput.h>
#else
#include <SDL/SDL.h>
#endif
#include <iostream>

using namespace GarrysMod::Lua;

//=============================================================================//
//	Joystick Input Module
//	Version 1.2
//	Written by Night-Eagle
//=============================================================================//

// Libraries
#ifdef _WIN32
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "ole32.lib")
#endif

// Globals
#ifdef _WIN32
LPDIRECTINPUT8 din; //Root DirectInput Interface
LPDIRECTINPUTDEVICE8 dinkeyboard; //Keyboard Device
LPDIRECTINPUTDEVICE8 dinmouse; // Mouse Device
#endif
const int max_devices = 64; //Maximum number of devices

// Joystick globals
#ifdef _WIN32
LPDIRECTINPUTDEVICE8 joy_din[max_devices]; //Joystick Device
DIDEVCAPS joy_devcaps[max_devices]; //DIDEVCAPS Struct
char joy_name[max_devices][MAX_PATH]; //Name of joystick
float joy_guid[max_devices][3]; //GUID of joystick 1
float joy_guids[max_devices][8]; //GUID of joystick 2
#else
SDL_Joystick* joystick[max_devices];
#endif
int joy_n = 0;
const char* binaryversion = "1.2";

#ifdef _WIN32
DIJOYSTATE2 joy_state[max_devices]; //State of joystick
bool joy_active[max_devices]; //Joystick present
#endif
// End joystick globals

//
// Lua Functions
//

int keyboardstate(lua_State* state)
{
  #ifdef _WIN32
  char buffer[256];
  HRESULT hr;
  hr = dinkeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer);
  if (FAILED(hr))
  {
    if (hr == DIERR_INPUTLOST)
    {
      dinkeyboard->Acquire();
    }
    return 0;
  }

  if (LUA->GetTypeName(1) != "table")
    LUA->CreateTable();

    for (int i = 0; i < 256; i++)
    {
      bool temp = buffer[i] & 0x80;
      LUA->PushBool(temp);
    }
    LUA->Push(-2);
  #endif
  return 1;
}

int refresh(lua_State* state)
{
  #ifdef _WIN32
  int joy = (LUA->GetNumber(1));

  if ((joy >= 0) && (joy < joy_n))
  {
    HRESULT hr;
    if (joy_din[joy])
    {
      if (FAILED(hr = joy_din[joy]->Poll()))
      {
        hr = joy_din[joy]->Acquire();
      }
      else
      {
        hr = joy_din[joy]->GetDeviceState(sizeof(DIJOYSTATE2), &joy_state[joy]);
      }
    }
  }
  else
  {
    int i;
    for (i=0; i<joy_n; i++)
    {
      HRESULT hr;
      if (joy_din[i])
      {
        if (FAILED(hr = joy_din[i]->Poll()))
        {
          hr = joy_din[i]->Acquire();
        }
        else
        {
          hr = joy_din[i]->GetDeviceState(sizeof(DIJOYSTATE2), &joy_state[i]);
        }
      }
    }
  }
  #else
  SDL_JoystickUpdate();
  #endif
  return 0;
}

int axis(lua_State* state)
{
  int joy = (LUA->GetNumber(1));
  int axi = (LUA->GetNumber(2));
  long out;

  #ifdef _WIN32
  switch(axi)
  {
    case 0:
      out = joy_state[joy].lX;
      break;
    case 1:
      out = joy_state[joy].lY;
      break;
    case 2:
      out = joy_state[joy].lZ;
      break;
    case 3:
      out = joy_state[joy].lRx;
      break;
    case 4:
      out = joy_state[joy].lRy;
      break;
    case 5:
      out = joy_state[joy].lRz;
      break;
    case 6:
      out = joy_state[joy].rglSlider[0];
      break;
    case 7:
      out = joy_state[joy].rglSlider[1];
      break;
    default:
      out = 0;
  }
  #else
  out = SDL_JoystickGetAxis(joystick[joy], axi);
  #endif

  LUA->PushNumber((float)(out + 32768));
  return 1;
}

int button(lua_State* state)
{
  int joy = (LUA->GetNumber(1));
  int but = (LUA->GetNumber(2));
  #ifdef _WIN32
  LUA->PushNumber((float)(joy_state[joy].rgbButtons[but]));
  #else
  LUA->PushNumber((float)SDL_JoystickGetButton(joystick[joy], but));
  #endif
  return 1;
}

int pov(lua_State* state)
{
  int joy = (LUA->GetNumber(1));
  int pov = (LUA->GetNumber(2));
  #ifdef _WIN32
  LUA->PushNumber((float)(joy_state[joy].rgdwPOV[pov]));
  #else
  const int out[13] = { -1, 0, 9000, 4500, 18000, -1, 13500, -1, 27000, 31500, -1, -1, 22500 };
  LUA->PushNumber((float)out[SDL_JoystickGetHat(joystick[joy], pov)]);
  #endif
  return 1;
}

int count(lua_State* state)
{
  int joy = (LUA->GetNumber(1));
  int opt = (LUA->GetNumber(2));
  float out;
  switch(opt)
  {
    case 1:
      #ifdef _WIN32
      out = (float)joy_devcaps[joy].dwAxes;
      #else
      out = (float)SDL_JoystickNumAxes(joystick[joy]);
      #endif
      break;
    case 2:
      #ifdef _WIN32
      out = (float)joy_devcaps[joy].dwButtons;
      #else
      out = (float)SDL_JoystickNumButtons(joystick[joy]);
      #endif
      break;
    case 3:
      #ifdef _WIN32
      out = (float)joy_devcaps[joy].dwPOVs;
      #else
      out = (float)SDL_JoystickNumHats(joystick[joy]);
      #endif
      break;
    default:
      out = (float)joy_n;
  }
  LUA->PushNumber(out);
  return 1;
}

int name(lua_State* state)
{
  int joy = (LUA->GetNumber(1));
  #ifdef _WIN32
  LUA->PushString((char*)joy_name[joy]);
  #else
  LUA->PushString(SDL_JoystickName(joy));
  #endif
  return 1;
}

int guidm(lua_State* state)
{
  #ifdef _WIN32
  int joy = (LUA->GetNumber(1));
  LUA->PushNumber((float)joy_guid[joy][0]);
  LUA->PushNumber((float)joy_guid[joy][1]);
  LUA->PushNumber((float)joy_guid[joy][2]);
  int i;
  for (i=0; i<8; i++)
  {
    LUA->PushNumber((float)joy_guids[joy][i]);
  }
  #endif
  return 11;
}

//
// Axis Initialization
//

#ifdef _WIN32
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{

  HRESULT hr;
  DIPROPRANGE diprg;

  diprg.diph.dwSize       = sizeof(DIPROPRANGE);
  diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  diprg.diph.dwHow        = DIPH_BYID;
  diprg.diph.dwObj        = pdidoi->dwType;
  if ((pdidoi->dwType)&DIDFT_AXIS)
  {
    diprg.lMin              = 0;
    diprg.lMax              = +65535;
  }
  else
  {
    diprg.lMin              = 0;
    diprg.lMax              = +1;
  }

  if (FAILED(hr = joy_din[joy_n]->SetProperty(
    DIPROP_RANGE,
    &diprg.diph
  )))
  {
    return DIENUM_STOP;
  }
  else
  {
    return DIENUM_CONTINUE;
  }
}
#endif

//
// Joystick Initialization
//

#ifdef _WIN32
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
  HRESULT hr;

  // Get interface
  hr = din->CreateDevice(
    pdidInstance->guidInstance,
    &joy_din[joy_n],
    NULL
  );

  if (FAILED(hr))
  {
    return DIENUM_CONTINUE;
  }

  if (FAILED(hr = joy_din[joy_n]->SetDataFormat(&c_dfDIJoystick2)))
  {
    return DIENUM_CONTINUE;
  }

  if (FAILED(hr = joy_din[joy_n]->SetCooperativeLevel(NULL, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
  {
    return DIENUM_CONTINUE;
  }

  joy_devcaps[joy_n].dwSize = sizeof(DIDEVCAPS);
  if (FAILED(joy_din[joy_n]->GetCapabilities(&joy_devcaps[joy_n])))
  {
    return DIENUM_CONTINUE;
  }

  if (FAILED(joy_din[joy_n]->EnumObjects(
    EnumAxesCallback,
    NULL,
    NULL
  )))
  {
    return DIENUM_CONTINUE;
  }

  // User stuff
  if ((joy_devcaps[joy_n].dwFlags & DIDC_ATTACHED) > 0)
  {
    joy_active[joy_n] = true;
  }

  //Get device info
  int i;
  for (i=0;i<MAX_PATH;i++)
  {
    joy_name[joy_n][i] = pdidInstance->tszProductName[i];
  }

  joy_guid[joy_n][0] = (pdidInstance->guidInstance).Data1;
  joy_guid[joy_n][1] = (pdidInstance->guidInstance).Data2;
  joy_guid[joy_n][2] = (pdidInstance->guidInstance).Data3;
  for (i=0;i<8;i++)
  {
    joy_guids[joy_n][i] = (pdidInstance->guidInstance).Data4[i];
  }

  // Acquire the joystick
  joy_din[joy_n]->Acquire();

  // Increment joy_n
  joy_n++;

  // We want to enumerate all joysticks, so keep on enumerating until we are out of joysticks
  return DIENUM_CONTINUE;
}
#endif

//
// DirectInput Start/Stop
//

#ifdef _WIN32
bool StopDI(void)
{
  dinkeyboard->Unacquire();
  din->Release();

  joy_n = 0;

  return true;
}
#else
bool StopSDL(void) {
  SDL_Quit();
  return true;
}
#endif

#ifdef _WIN32
bool InitDI(void)
{
  HRESULT hr;

  // Start DirectInput
  DirectInput8Create(
    GetModuleHandle(NULL),
    DIRECTINPUT_VERSION,
    IID_IDirectInput8,
    (void**)&din,
    NULL
  );

  if (din == NULL)
  {
    return false;
  }

  hr = din->CreateDevice(
    GUID_SysKeyboard,
    &dinkeyboard,
    NULL
  );

  if (FAILED(hr))
  {
  } else {
    hr = dinkeyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr))
    {
    } else {
      dinkeyboard->SetCooperativeLevel(
        NULL,
        DISCL_FOREGROUND | DISCL_EXCLUSIVE
      );
      dinkeyboard->Acquire();
    }
  }

  //
  // Joystick Meat
  //

  din->EnumDevices(
    DI8DEVCLASS_GAMECTRL,
    EnumJoysticksCallback,
    NULL,
    DIEDFL_ATTACHEDONLY
  );

  return true;
}
#else
bool InitSDL(void) {
  int result = SDL_Init(SDL_INIT_JOYSTICK);

  if (!result) {
    joy_n = SDL_NumJoysticks();

    for (int i = 0; i < joy_n; i++)
      joystick[i] = SDL_JoystickOpen(i);

    return true;
  }

  return false;
}
#endif

int restart(lua_State* state)
{
  #ifdef _WIN32
  bool result = StopDI();
  #else
  bool result = StopSDL();
  #endif
  if (!result)
  {
    LUA->PushBool(false);
    return 1;
  }
  #ifdef _WIN32
  result = InitDI();
  #else
  result = InitSDL();
  #endif
  LUA->PushBool(result);
  return 1;
}

//
// Initialization
//

GMOD_MODULE_OPEN()
{
  LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA->CreateTable();
      LUA->PushCFunction(refresh); LUA->SetField(-2, "refresh");
      LUA->PushCFunction(axis); LUA->SetField(-2, "axis");
      LUA->PushCFunction(button); LUA->SetField(-2, "button");
      LUA->PushCFunction(pov); LUA->SetField(-2, "pov");
      LUA->PushCFunction(count); LUA->SetField(-2, "count");
      LUA->PushCFunction(name); LUA->SetField(-2, "name");
      LUA->PushCFunction(guidm); LUA->SetField(-2, "guidm");
      LUA->PushCFunction(restart); LUA->SetField(-2, "restart");
      LUA->PushString(binaryversion); LUA->SetField(-2, "binaryversion");
      LUA->PushCFunction(keyboardstate); LUA->SetField(-2, "keyboardstate");
    LUA->SetField(-2, "joystick");
  LUA->Pop();

  #ifdef _WIN32
  bool result = InitDI();
  #else
  InitSDL();
  #endif
  return 0;
}

GMOD_MODULE_CLOSE()
{
  #ifdef _WIN32
  bool result = StopDI();
  #else
  StopSDL();
  #endif
  return 0;
}
