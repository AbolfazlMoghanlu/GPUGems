outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"	
	
project "B1C1_EffectiveWaterSimulation"
	location ""
	kind "WindowedApp"
	language "c++"
	cppdialect "c++17"
	staticruntime "on"
	
	targetdir ("../Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../Intermediate/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Source/**.hlsl"
	}

	links
	{
		--"d3d12.lib",
		--"D3DCompiler.lib",
		--"DXGI.lib",
		--"dxguid.lib"
	}
	
	--	postbuildcommands 
	--{
	--	"{RMDIR} ../Bin/" .. outputdir .. "/Content/",
	--	"{COPY} ../Content/ ../Bin/" .. outputdir .. "/Content/",
	--
	--	"{RMDIR} ../Bin/" .. outputdir .. "/playground/Source/Shader",
	--	"{COPY} Source/Shader ../Bin/" .. outputdir .. "/playground/Source/Shader/"		
	--}
	
	libdirs
	{
		"ThirdParty/Library/**"
	}
	
	includedirs
	{
		"%{prj.name}/Source",
		"%{prj.name}/Source/imgui",
		"ThirdParty/Header"
	}


--	filter("files:**.hlsl")
--		  flags("ExcludeFromBuild")
--		  shaderobjectfileoutput(shader_dir.."%{file.basename}"..".cso")
--		  shaderassembleroutput(shader_dir.."%{file.basename}"..".asm")

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