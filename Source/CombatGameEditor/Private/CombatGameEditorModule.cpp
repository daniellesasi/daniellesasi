// CombatGameEditorModule.cpp
// Editor module that auto-creates all Blueprint assets on first load.
// Once assets exist, it does nothing (safe to leave enabled).

#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/WorldFactory.h"
#include "Factories/DataTableFactory.h"
#include "EditorAssetLibrary.h"
#include "FileHelpers.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

// Game classes
#include "CombatCharacter.h"
#include "CombatGameMode.h"
#include "CombatHUD.h"
#include "FightingArena.h"
#include "FightingCameraActor.h"
#include "FighterAIController.h"
#include "FighterAnimInstance.h"
#include "MainMenuWidget.h"
#include "CharacterSelectWidget.h"
#include "FightHUDWidget.h"
#include "PauseMenuWidget.h"
#include "CombatTypes.h"

#include "InputAction.h"
#include "InputMappingContext.h"

#define LOCTEXT_NAMESPACE "CombatGameEditor"

DEFINE_LOG_CATEGORY_STATIC(LogCombatSetup, Log, All);

class FCombatGameEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// Delay until the editor is fully loaded
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &FCombatGameEditorModule::OnPostEngineInit);
	}

	virtual void ShutdownModule() override
	{
	}

private:
	void OnPostEngineInit()
	{
		// Further delay to ensure all systems are ready
		if (GEditor)
		{
			GEditor->GetTimerManager()->SetTimerForNextTick([this]()
			{
				RunSetup();
			});
		}
	}

	void RunSetup()
	{
		// Check if we already ran (test for one known asset)
		if (UEditorAssetLibrary::DoesAssetExist(TEXT("/Game/Blueprints/BP_CombatGameMode")))
		{
			UE_LOG(LogCombatSetup, Log, TEXT("Assets already exist - skipping auto-setup."));
			return;
		}

		UE_LOG(LogCombatSetup, Warning, TEXT("============================================"));
		UE_LOG(LogCombatSetup, Warning, TEXT("CombatGame: Auto-creating all assets..."));
		UE_LOG(LogCombatSetup, Warning, TEXT("============================================"));

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		// ------------------------------------------------------------------
		// 1. MAPS
		// ------------------------------------------------------------------
		CreateMap(TEXT("/Game/Maps/MainMenuMap"));
		CreateMap(TEXT("/Game/Maps/CharacterSelectMap"));
		CreateMap(TEXT("/Game/Maps/FightingArenaMap"));

		// ------------------------------------------------------------------
		// 2. GAMEPLAY BLUEPRINTS
		// ------------------------------------------------------------------
		CreateBlueprint(AssetTools, TEXT("/Game/Blueprints"), TEXT("BP_CombatGameMode"), ACombatGameMode::StaticClass());
		CreateBlueprint(AssetTools, TEXT("/Game/Blueprints"), TEXT("BP_CombatHUD"), ACombatHUD::StaticClass());
		CreateBlueprint(AssetTools, TEXT("/Game/Characters"), TEXT("BP_TestFighter"), ACombatCharacter::StaticClass());
		CreateBlueprint(AssetTools, TEXT("/Game/Blueprints"), TEXT("BP_FightingArena"), AFightingArena::StaticClass());
		CreateBlueprint(AssetTools, TEXT("/Game/Blueprints"), TEXT("BP_FightingCamera"), AFightingCameraActor::StaticClass());
		CreateBlueprint(AssetTools, TEXT("/Game/Blueprints"), TEXT("BP_FighterAI"), AFighterAIController::StaticClass());

		// ------------------------------------------------------------------
		// 3. WIDGET BLUEPRINTS
		// ------------------------------------------------------------------
		CreateWidgetBlueprint(AssetTools, TEXT("/Game/UI"), TEXT("WBP_MainMenu"), UMainMenuWidget::StaticClass());
		CreateWidgetBlueprint(AssetTools, TEXT("/Game/UI"), TEXT("WBP_CharacterSelect"), UCharacterSelectWidget::StaticClass());
		CreateWidgetBlueprint(AssetTools, TEXT("/Game/UI"), TEXT("WBP_FightHUD"), UFightHUDWidget::StaticClass());
		CreateWidgetBlueprint(AssetTools, TEXT("/Game/UI"), TEXT("WBP_PauseMenu"), UPauseMenuWidget::StaticClass());

		// ------------------------------------------------------------------
		// 4. INPUT ACTIONS
		// ------------------------------------------------------------------
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_Move"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_LeftPunch"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_RightPunch"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_LeftKick"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_RightKick"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_Block"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_Jump"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_Crouch"));
		CreateInputAction(AssetTools, TEXT("/Game/Input"), TEXT("IA_Pause"));

		// Input Mapping Context
		CreateInputMappingContext(AssetTools, TEXT("/Game/Input"), TEXT("IMC_Fighter"));

		// ------------------------------------------------------------------
		// 5. SAVE ALL
		// ------------------------------------------------------------------
		FEditorFileUtils::SaveDirtyPackages(/*bPromptUserToSave=*/false, /*bSaveMapPackages=*/true, /*bSaveContentPackages=*/true);

		UE_LOG(LogCombatSetup, Warning, TEXT("============================================"));
		UE_LOG(LogCombatSetup, Warning, TEXT("CombatGame: Auto-setup COMPLETE!"));
		UE_LOG(LogCombatSetup, Warning, TEXT("Created: 3 Maps, 6 Blueprints, 4 Widgets, 10 Input assets"));
		UE_LOG(LogCombatSetup, Warning, TEXT("============================================"));
		UE_LOG(LogCombatSetup, Warning, TEXT(""));
		UE_LOG(LogCombatSetup, Warning, TEXT("NEXT STEPS:"));
		UE_LOG(LogCombatSetup, Warning, TEXT("  1. Open BP_CombatGameMode > set DefaultPawnClass = BP_TestFighter"));
		UE_LOG(LogCombatSetup, Warning, TEXT("  2. Open BP_CombatGameMode > set HUDClass = BP_CombatHUD"));
		UE_LOG(LogCombatSetup, Warning, TEXT("  3. Open BP_CombatHUD > set FightHUDWidgetClass = WBP_FightHUD"));
		UE_LOG(LogCombatSetup, Warning, TEXT("  4. Design UI in each WBP_* widget blueprint"));
		UE_LOG(LogCombatSetup, Warning, TEXT("  5. Open FightingArenaMap > place BP_FightingArena + BP_FightingCamera"));
	}

	// ------------------------------------------------------------------
	// HELPER: Create Blueprint
	// ------------------------------------------------------------------
	void CreateBlueprint(IAssetTools& AssetTools, const FString& Path, const FString& Name, UClass* ParentClass)
	{
		FString FullPath = Path / Name;
		if (UEditorAssetLibrary::DoesAssetExist(FullPath))
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Already exists: %s"), *FullPath);
			return;
		}

		UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
		Factory->ParentClass = ParentClass;

		UObject* Asset = AssetTools.CreateAsset(Name, Path, UBlueprint::StaticClass(), Factory);
		if (Asset)
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Created Blueprint: %s"), *FullPath);
		}
		else
		{
			UE_LOG(LogCombatSetup, Error, TEXT("  FAILED to create: %s"), *FullPath);
		}
	}

	// ------------------------------------------------------------------
	// HELPER: Create Widget Blueprint
	// ------------------------------------------------------------------
	void CreateWidgetBlueprint(IAssetTools& AssetTools, const FString& Path, const FString& Name, UClass* ParentClass)
	{
		FString FullPath = Path / Name;
		if (UEditorAssetLibrary::DoesAssetExist(FullPath))
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Already exists: %s"), *FullPath);
			return;
		}

		UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
		Factory->ParentClass = ParentClass;

		UObject* Asset = AssetTools.CreateAsset(Name, Path, UWidgetBlueprint::StaticClass(), Factory);
		if (Asset)
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Created Widget BP: %s"), *FullPath);
		}
		else
		{
			UE_LOG(LogCombatSetup, Error, TEXT("  FAILED to create: %s"), *FullPath);
		}
	}

	// ------------------------------------------------------------------
	// HELPER: Create Map
	// ------------------------------------------------------------------
	void CreateMap(const FString& FullPath)
	{
		if (UEditorAssetLibrary::DoesAssetExist(FullPath))
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Map already exists: %s"), *FullPath);
			return;
		}

		UWorldFactory* Factory = NewObject<UWorldFactory>();
		Factory->WorldType = EWorldType::None;

		// Extract path and name
		FString Path, Name;
		FullPath.Split(TEXT("/"), &Path, &Name, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UObject* Asset = AssetTools.CreateAsset(Name, Path, UWorld::StaticClass(), Factory);
		if (Asset)
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Created Map: %s"), *FullPath);
		}
		else
		{
			UE_LOG(LogCombatSetup, Error, TEXT("  FAILED to create map: %s"), *FullPath);
		}
	}

	// ------------------------------------------------------------------
	// HELPER: Create Input Action
	// ------------------------------------------------------------------
	void CreateInputAction(IAssetTools& AssetTools, const FString& Path, const FString& Name)
	{
		FString FullPath = Path / Name;
		if (UEditorAssetLibrary::DoesAssetExist(FullPath))
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Already exists: %s"), *FullPath);
			return;
		}

		UObject* Asset = AssetTools.CreateAsset(Name, Path, UInputAction::StaticClass(), nullptr);
		if (Asset)
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Created Input Action: %s"), *FullPath);
		}
		else
		{
			UE_LOG(LogCombatSetup, Error, TEXT("  FAILED to create: %s"), *FullPath);
		}
	}

	// ------------------------------------------------------------------
	// HELPER: Create Input Mapping Context
	// ------------------------------------------------------------------
	void CreateInputMappingContext(IAssetTools& AssetTools, const FString& Path, const FString& Name)
	{
		FString FullPath = Path / Name;
		if (UEditorAssetLibrary::DoesAssetExist(FullPath))
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Already exists: %s"), *FullPath);
			return;
		}

		UObject* Asset = AssetTools.CreateAsset(Name, Path, UInputMappingContext::StaticClass(), nullptr);
		if (Asset)
		{
			UE_LOG(LogCombatSetup, Log, TEXT("  Created IMC: %s"), *FullPath);
		}
		else
		{
			UE_LOG(LogCombatSetup, Error, TEXT("  FAILED to create: %s"), *FullPath);
		}
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCombatGameEditorModule, CombatGameEditor)
