using UnrealBuildTool;

public class CombatGameEditor : ModuleRules
{
	public CombatGameEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"AssetTools",
			"CombatGame"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore",
			"EditorScriptingUtilities",
			"EnhancedInput",
			"InputCore",
			"UMG",
			"UMGEditor"
		});
	}
}
