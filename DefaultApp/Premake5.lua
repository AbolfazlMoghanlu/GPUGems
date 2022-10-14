outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"	
	
project "DefaultApp"
	location ""
	kind "WindowedApp"
	language "c++"
	cppdialect "c++17"
	staticruntime "on"
	
	targetdir ("../Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Source/**.hlsl"
	}

	links
	{
		"d3d12.lib",
		"D3DCompiler.lib",
		"DXGI.lib",
		"dxguid.lib"
	}
	
	postbuildcommands 
	{
		"{RMDIR} ../Bin/" .. outputdir .. "/Content/Common",
		"{COPY} ../Content/Common ../Bin/" .. outputdir .. "/Content/Common",

		"{RMDIR} ../Bin/" .. outputdir .. "/DefaultApp/Source/Shader",
		"{COPY} Source/Shader ../Bin/" .. outputdir .. "/DefaultApp/Source/Shader/"		
	}
	
	libdirs
	{
		"../ThirdParty/Library/**"
	}
	
	includedirs
	{
		"Source",
		"Source/imgui",
		"../ThirdParty/Header"
	}

	filter "files:**/imgui/**"
		flags {"NoPCH"}

	filter("files:**_ps.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Pixel")
		shadermodel("4.0")	

   filter("files:**_vs.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Vertex")
		shadermodel("4.0")	

	filter "configurations:Debug"
		defines
		{
			"DRN_DEBUG"
		}
			
		runtime "Debug"
		symbols "on"			
		
	filter "configurations:Release"
		defines
		{
			"DRN_RELEASE"
		}
		
		runtime "Release"
		optimize "on"