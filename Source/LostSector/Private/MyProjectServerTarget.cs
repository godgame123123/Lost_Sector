using UnrealBuildTool;
using System.Collections.Generic;

public class MyProjectServerTarget : TargetRules
{
    public MyProjectServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("MyProject");
    }
}
