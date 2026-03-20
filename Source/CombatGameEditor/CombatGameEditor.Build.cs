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
			"KismetCompiler",
			"Kismet",
			"BlueprintGraph",
			"UMGEditor",
			"Blutility",
			"CombatGame"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore",
			"EditorScriptingUtilities",
			"EnhancedInput",
			"InputCore",
			"UMG"
		});
	}
}
