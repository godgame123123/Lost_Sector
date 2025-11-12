// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LostSector : ModuleRules
{
	public LostSector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject",
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"OnlineSubsystem",        // ✅ 기본 온라인 시스템
			"OnlineSubsystemUtils",   // ✅ 유틸리티
			"UMG",
			"Json", 
			"JsonUtilities",
			"AIModule"
		});
		
		// ✅ Steam은 동적 로드만 사용 (PublicDependency에서 제거)
		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
