outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"	
	
project "B1C1_EffectiveWaterSimulation"
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
		"{RMDIR} ../Bin/" .. outputdir .. "/Content/B1C1",
		"{COPY} ../Content/B1C1 ../Bin/" .. outputdir .. "/Content/B1C1",

		"{RMDIR} ../Bin/" .. outputdir .. "/B1C1_EffectiveWaterSimulation/Source/Shader",
		"{COPY} Source/Shader ../Bin/" .. outputdir .. "/B1C1_EffectiveWaterSimulation/Source/Shader/"		
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
		shadermodel("5.0")	

   filter("files:**_vs.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Vertex")
		shadermodel("5.0")	

	filter("files:**_hl.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Hull")
		shadermodel("5.0")	
		
	filter("files:**_dm.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Domain")
		shadermodel("5.0")	

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