#include <MinHook.h>
#include <thread>
#include <Windows.h>

#pragma comment(lib, "libMinHook.x64.lib")

//////////////////////////////////////////////////////////////////////////
// Definitions

struct SomeStruct
{
  int someValue;
};

// A type definition for a function so tFunction can be used to describe pointers to functions like this
// __fastcall is a calling convention that can be used for almost any function in 64-bit Windows processes
typedef void(__fastcall* tFunction)(int a1, int a2, SomeStruct* a3, __int64 a4, __int64 a5);
typedef float(__fastcall* tFieldOfView)();

//////////////////////////////////////////////////////////////////////////
// Declarations

tFunction origFunction = nullptr;
tFieldOfView origFieldOfView = nullptr;

bool changeStuff = false;
float ourCustomFov = 50.f;

void __fastcall hookFunction(int a1, int a2, SomeStruct* a3, __int64 a4, __int64 a5)
{
  if (changeStuff)
  {
    a1 = 10;
    a2 = 30430421;
    a3->someValue = 0;
    // and so on...
  }

  // Call original function with modified arguments and return
  return origFunction(a1, a2, a3, a4, a5);
}

float __fastcall hookFieldOfView()
{
  // Maybe the original function does other critical stuff that needs to be executed to prevent crashing or w/e
  float result = origFieldOfView();

  if (changeStuff)
    return 25.f; // very cinematic fov

  return result;
}

//////////////////////////////////////////////////////////////////////////
// Our really awesome main loop

void main()
{
  AllocConsole(); // Creates a console window
  freopen("CONOUT$", "w", stdout); // Some magical code to make printf and std::cout to actually print stuff to a console opened with AllocConsole

  printf("Hello World!\n"); // The classic

  // MinHook initialization
  MH_STATUS status = MH_Initialize();

  // Create and enable hooks
  MH_CreateHook((LPVOID)0x1312355, (PVOID)hookFunction, (LPVOID*)&origFunction);
  MH_EnableHook((LPVOID)0x1312355);

  __int64 gameBase = reinterpret_cast<__int64>( GetModuleHandleA("game.exe") ); // gameHandle will now be the base address of the game

  MH_CreateHook((LPVOID)(gameBase + 0x12A12A), (PVOID)hookFieldOfView, (LPVOID*)&origFieldOfView);
  MH_EnableHook((LPVOID)(gameBase + 0x12A12A));

  // A very fancy loop
  while (true)
  {
    // Is Page up pressed down
    if (GetAsyncKeyState(VK_PRIOR) & 0x8000)
    {
      ourCustomFov += 0.001;
    }

    // Page down
    if (GetAsyncKeyState(VK_NEXT) & 0x8000)
    {
      ourCustomFov += 0.001;
    }

    if (GetAsyncKeyState('E') & 0x8000)
    {
      // exit
      break;
    }

    Sleep(1);
  }


}

// START HERE
// DLL Entry point, Windows will automatically call this every time when for example
// the DLL is loaded or unloaded.
BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
  {
    // When the DLL is injected for the first time, create a new thread that starts at main()
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)main, NULL, NULL, NULL);
  }

  return TRUE;
}
