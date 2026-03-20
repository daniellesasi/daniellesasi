using UnrealBuildTool;

public class CombatGameTarget : TargetRules
{
	public CombatGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("CombatGame");
	}
}
