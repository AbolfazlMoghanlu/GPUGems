#include <iostream>
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include <chrono>

#include <fstream>
#include <string>

#include "d3dx12.h"
#include "MathLibrary.h"
#include "Vector.h"
#include "Rotator.h"
#include "Plane.h"
#include "Matrix.h"


#include "Imgui/imgui.h"
#include "Imgui/backends/imgui_impl_win32.h"
#include "Imgui/backends/imgui_impl_dx12.h"