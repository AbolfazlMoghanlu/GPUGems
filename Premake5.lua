workspace "GPUGems"
	architecture "x64"
	startproject "DefaultApp"

	configurations
	{
		"Debug",
		"Release"
	}
		
include "B1C1_EffectiveWaterSimulation/Premake5.lua"
include "DefaultApp/Premake5.lua"