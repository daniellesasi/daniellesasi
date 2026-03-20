using UnrealBuildTool;

public class CombatGameEditorTarget : TargetRules
{
	public CombatGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("CombatGame");
		ExtraModuleNames.Add("CombatGameEditor");
	}
}
