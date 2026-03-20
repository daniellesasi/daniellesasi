"""
UE5 Python Script - Auto-creates all Blueprints, Maps, and Widgets for CombatGame
==================================================================================

HOW TO RUN:
  1. Open the project in UE5
  2. Go to: Edit > Editor Preferences > Plugins > Python
     - Make sure Python is enabled (should be from .uproject)
  3. Open the Output Log: Window > Developer Tools > Output Log
  4. At the bottom of the Output Log, switch the dropdown from "Cmd" to "Python"
  5. Type:  exec(open('/Game/../Content/Python/setup_combat_game.py').read())
     OR easier: In UE5 menu, go to File > Execute Python Script
     and browse to this file in your project's Content/Python folder.

This script creates:
  - 3 Maps (MainMenuMap, CharacterSelectMap, FightingArenaMap)
  - 4 Widget Blueprints (WBP_MainMenu, WBP_CharacterSelect, WBP_FightHUD, WBP_PauseMenu)
  - 1 Fighter Blueprint (BP_TestFighter)
  - 1 Game Mode Blueprint (BP_CombatGameMode)
  - 1 HUD Blueprint (BP_CombatHUD)
  - 1 Arena Blueprint (BP_FightingArena)
  - 1 Camera Blueprint (BP_FightingCamera)
  - 1 AI Controller Blueprint (BP_FighterAI)
  - 1 Animation Blueprint (ABP_TestFighter)
  - 1 Input Mapping Context + Input Actions for Enhanced Input
"""

import unreal

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_asset_lib = unreal.EditorAssetLibrary
subsystem = unreal.get_editor_subsystem(unreal.SubobjectDataSubsystem) if hasattr(unreal, 'SubobjectDataSubsystem') else None

def create_blueprint(path, name, parent_class):
    """Create a Blueprint asset based on a C++ parent class."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"Asset already exists: {full_path}, skipping.")
        return unreal.load_asset(full_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset = asset_tools.create_asset(name, path, None, factory)
    if asset:
        unreal.log(f"Created Blueprint: {full_path}")
    else:
        unreal.log_error(f"Failed to create Blueprint: {full_path}")
    return asset


def create_widget_blueprint(path, name, parent_class):
    """Create a Widget Blueprint (UMG) based on a C++ widget parent class."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"Asset already exists: {full_path}, skipping.")
        return unreal.load_asset(full_path)

    # For widget BPs we need the WidgetBlueprintFactory
    # Note: parent_class should be a UUserWidget subclass
    factory = unreal.WidgetBlueprintFactory() if hasattr(unreal, 'WidgetBlueprintFactory') else None

    if factory is None:
        # Fallback: create as regular blueprint
        unreal.log_warning(f"WidgetBlueprintFactory not available, using standard factory for {name}")
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)
        asset = asset_tools.create_asset(name, path, None, factory)
    else:
        factory.set_editor_property("parent_class", parent_class)
        asset = asset_tools.create_asset(name, path, None, factory)

    if asset:
        unreal.log(f"Created Widget Blueprint: {full_path}")
    else:
        unreal.log_error(f"Failed to create Widget Blueprint: {full_path}")
    return asset


def create_level(path, name):
    """Create a new empty level/map."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"Map already exists: {full_path}, skipping.")
        return

    editor_level_lib = unreal.EditorLevelLibrary
    world = unreal.EditorLevelLibrary.new_level(full_path)
    if world:
        unreal.log(f"Created Level: {full_path}")
    else:
        # Alternative approach
        try:
            unreal.EditorLevelLibrary.new_level_from_template(full_path, "")
            unreal.log(f"Created Level (template): {full_path}")
        except:
            unreal.log_error(f"Failed to create Level: {full_path}")
    return world


def create_anim_blueprint(path, name, parent_class, skeleton):
    """Create an Animation Blueprint."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"Asset already exists: {full_path}, skipping.")
        return unreal.load_asset(full_path)

    factory = unreal.AnimBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    if skeleton:
        factory.set_editor_property("target_skeleton", skeleton)

    asset = asset_tools.create_asset(name, path, None, factory)
    if asset:
        unreal.log(f"Created Anim Blueprint: {full_path}")
    else:
        unreal.log_error(f"Failed to create Anim Blueprint: {full_path}")
    return asset


def create_input_action(path, name, value_type=unreal.InputActionValueType.BOOLEAN):
    """Create an Enhanced Input Action."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"Asset already exists: {full_path}, skipping.")
        return unreal.load_asset(full_path)

    factory = unreal.InputActionFactory() if hasattr(unreal, 'InputActionFactory') else None
    if factory:
        asset = asset_tools.create_asset(name, path, None, factory)
        if asset:
            asset.set_editor_property("value_type", value_type)
            unreal.log(f"Created Input Action: {full_path}")
        return asset
    else:
        # Create via asset tools directly
        asset = asset_tools.create_asset(name, path, unreal.InputAction, None)
        if asset:
            asset.set_editor_property("value_type", value_type)
            unreal.log(f"Created Input Action: {full_path}")
        return asset


def create_input_mapping_context(path, name):
    """Create an Enhanced Input Mapping Context."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"Asset already exists: {full_path}, skipping.")
        return unreal.load_asset(full_path)

    factory = unreal.InputMappingContextFactory() if hasattr(unreal, 'InputMappingContextFactory') else None
    if factory:
        asset = asset_tools.create_asset(name, path, None, factory)
    else:
        asset = asset_tools.create_asset(name, path, unreal.InputMappingContext, None)

    if asset:
        unreal.log(f"Created Input Mapping Context: {full_path}")
    return asset


# ============================================================================
# LOAD C++ PARENT CLASSES
# ============================================================================

unreal.log("=" * 60)
unreal.log("CombatGame Setup Script - Starting...")
unreal.log("=" * 60)

# Load all C++ parent classes
CombatCharacter = unreal.load_class(None, "/Script/CombatGame.CombatCharacter")
CombatGameMode = unreal.load_class(None, "/Script/CombatGame.CombatGameMode")
CombatHUD = unreal.load_class(None, "/Script/CombatGame.CombatHUD")
FightingArena = unreal.load_class(None, "/Script/CombatGame.FightingArena")
FightingCameraActor = unreal.load_class(None, "/Script/CombatGame.FightingCameraActor")
FighterAIController = unreal.load_class(None, "/Script/CombatGame.FighterAIController")
FighterAnimInstance = unreal.load_class(None, "/Script/CombatGame.FighterAnimInstance")
MainMenuWidget = unreal.load_class(None, "/Script/CombatGame.MainMenuWidget")
CharacterSelectWidget = unreal.load_class(None, "/Script/CombatGame.CharacterSelectWidget")
FightHUDWidget = unreal.load_class(None, "/Script/CombatGame.FightHUDWidget")
PauseMenuWidget = unreal.load_class(None, "/Script/CombatGame.PauseMenuWidget")

# Verify all classes loaded
classes = {
    "CombatCharacter": CombatCharacter,
    "CombatGameMode": CombatGameMode,
    "CombatHUD": CombatHUD,
    "FightingArena": FightingArena,
    "FightingCameraActor": FightingCameraActor,
    "FighterAIController": FighterAIController,
    "FighterAnimInstance": FighterAnimInstance,
    "MainMenuWidget": MainMenuWidget,
    "CharacterSelectWidget": CharacterSelectWidget,
    "FightHUDWidget": FightHUDWidget,
    "PauseMenuWidget": PauseMenuWidget,
}

all_loaded = True
for name, cls in classes.items():
    if cls is None:
        unreal.log_error(f"FAILED to load class: {name}")
        all_loaded = False
    else:
        unreal.log(f"Loaded class: {name}")

if not all_loaded:
    unreal.log_error("Some classes failed to load! Make sure the CombatGame module compiled successfully.")
    unreal.log_error("Continuing with available classes...")

# ============================================================================
# 1. CREATE MAPS
# ============================================================================

unreal.log("\n--- Creating Maps ---")
create_level("/Game/Maps", "MainMenuMap")
create_level("/Game/Maps", "CharacterSelectMap")
create_level("/Game/Maps", "FightingArenaMap")

# ============================================================================
# 2. CREATE WIDGET BLUEPRINTS
# ============================================================================

unreal.log("\n--- Creating Widget Blueprints ---")
if MainMenuWidget:
    create_widget_blueprint("/Game/UI", "WBP_MainMenu", MainMenuWidget)
if CharacterSelectWidget:
    create_widget_blueprint("/Game/UI", "WBP_CharacterSelect", CharacterSelectWidget)
if FightHUDWidget:
    create_widget_blueprint("/Game/UI", "WBP_FightHUD", FightHUDWidget)
if PauseMenuWidget:
    create_widget_blueprint("/Game/UI", "WBP_PauseMenu", PauseMenuWidget)

# ============================================================================
# 3. CREATE GAMEPLAY BLUEPRINTS
# ============================================================================

unreal.log("\n--- Creating Gameplay Blueprints ---")

# Game Mode BP
bp_game_mode = None
if CombatGameMode:
    bp_game_mode = create_blueprint("/Game/Blueprints", "BP_CombatGameMode", CombatGameMode)

# HUD BP
bp_hud = None
if CombatHUD:
    bp_hud = create_blueprint("/Game/Blueprints", "BP_CombatHUD", CombatHUD)

# Fighter BP (test character using UE5 mannequin)
bp_fighter = None
if CombatCharacter:
    bp_fighter = create_blueprint("/Game/Characters", "BP_TestFighter", CombatCharacter)

# Arena BP
bp_arena = None
if FightingArena:
    bp_arena = create_blueprint("/Game/Blueprints", "BP_FightingArena", FightingArena)

# Camera BP
bp_camera = None
if FightingCameraActor:
    bp_camera = create_blueprint("/Game/Blueprints", "BP_FightingCamera", FightingCameraActor)

# AI Controller BP
bp_ai = None
if FighterAIController:
    bp_ai = create_blueprint("/Game/Blueprints", "BP_FighterAI", FighterAIController)

# ============================================================================
# 4. CREATE ANIMATION BLUEPRINT (using UE5 Mannequin skeleton if available)
# ============================================================================

unreal.log("\n--- Creating Animation Blueprint ---")

# Try to find UE5 mannequin skeleton
skeleton = None
mannequin_paths = [
    "/Game/Characters/Mannequins/Meshes/SKM_Manny",
    "/Game/Characters/Mannequins/Meshes/SKM_Quinn",
    "/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP",
]
for path in mannequin_paths:
    skel = unreal.load_asset(path)
    if skel and hasattr(skel, 'skeleton'):
        skeleton = skel.get_editor_property("skeleton")
        unreal.log(f"Found skeleton from: {path}")
        break

if FighterAnimInstance:
    if skeleton:
        create_anim_blueprint("/Game/Animations", "ABP_TestFighter", FighterAnimInstance, skeleton)
    else:
        unreal.log_warning("No skeleton found - you'll need to create ABP_TestFighter manually after importing a skeletal mesh.")
        unreal.log_warning("Steps: Right-click > Animation > Animation Blueprint > pick FighterAnimInstance as parent + your skeleton")

# ============================================================================
# 5. CREATE INPUT ACTIONS FOR ENHANCED INPUT
# ============================================================================

unreal.log("\n--- Creating Input Actions ---")

input_path = "/Game/Input"

# Movement (Axis2D for stick/WASD)
create_input_action(input_path, "IA_Move", unreal.InputActionValueType.AXIS2D if hasattr(unreal.InputActionValueType, 'AXIS2D') else unreal.InputActionValueType.BOOLEAN)

# Attack buttons (Tekken-style: LP, RP, LK, RK)
create_input_action(input_path, "IA_LeftPunch")
create_input_action(input_path, "IA_RightPunch")
create_input_action(input_path, "IA_LeftKick")
create_input_action(input_path, "IA_RightKick")

# Other actions
create_input_action(input_path, "IA_Block")
create_input_action(input_path, "IA_Jump")
create_input_action(input_path, "IA_Crouch")
create_input_action(input_path, "IA_Pause")

# Input Mapping Context
create_input_mapping_context(input_path, "IMC_Fighter")

# ============================================================================
# 6. CREATE DATA TABLE FOR CHARACTER ROSTER
# ============================================================================

unreal.log("\n--- Creating Data Table ---")

dt_path = "/Game/Data/DT_Characters"
if not editor_asset_lib.does_asset_exist(dt_path):
    try:
        # Try to create data table with FCharacterData row struct
        factory = unreal.DataTableFactory() if hasattr(unreal, 'DataTableFactory') else None
        if factory:
            # Set the row struct to FCharacterData
            row_struct = unreal.load_object(None, "/Script/CombatGame.CharacterData")
            if row_struct:
                factory.set_editor_property("struct", row_struct)
                asset = asset_tools.create_asset("DT_Characters", "/Game/Data", None, factory)
                if asset:
                    unreal.log(f"Created Data Table: {dt_path}")
            else:
                unreal.log_warning("Could not load FCharacterData struct for data table. You can create it manually: Right-click > Miscellaneous > Data Table > pick FCharacterData")
        else:
            unreal.log_warning("DataTableFactory not available. Create DT_Characters manually.")
    except Exception as e:
        unreal.log_warning(f"Data table creation: {e}")
        unreal.log_warning("Create DT_Characters manually: Right-click > Miscellaneous > Data Table > FCharacterData")
else:
    unreal.log_warning(f"Asset already exists: {dt_path}, skipping.")

# ============================================================================
# 7. SAVE ALL
# ============================================================================

unreal.log("\n--- Saving All Assets ---")
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

# ============================================================================
# SUMMARY
# ============================================================================

unreal.log("\n" + "=" * 60)
unreal.log("CombatGame Setup COMPLETE!")
unreal.log("=" * 60)
unreal.log("")
unreal.log("Created assets:")
unreal.log("  Maps:       MainMenuMap, CharacterSelectMap, FightingArenaMap")
unreal.log("  Widgets:    WBP_MainMenu, WBP_CharacterSelect, WBP_FightHUD, WBP_PauseMenu")
unreal.log("  Blueprints: BP_CombatGameMode, BP_CombatHUD, BP_TestFighter,")
unreal.log("              BP_FightingArena, BP_FightingCamera, BP_FighterAI")
unreal.log("  Input:      IA_Move, IA_LeftPunch, IA_RightPunch, IA_LeftKick,")
unreal.log("              IA_RightKick, IA_Block, IA_Jump, IA_Crouch, IA_Pause, IMC_Fighter")
unreal.log("")
unreal.log("NEXT STEPS:")
unreal.log("  1. Open BP_CombatGameMode > set Default Pawn Class = BP_TestFighter")
unreal.log("     and HUD Class = BP_CombatHUD")
unreal.log("  2. Open BP_CombatHUD > set FightHUDWidgetClass = WBP_FightHUD")
unreal.log("  3. Open each WBP_* widget and design the UI layout")
unreal.log("     (see header file comments for required widget names)")
unreal.log("  4. Open BP_TestFighter > assign a Skeletal Mesh")
unreal.log("  5. Open FightingArenaMap > place BP_FightingArena and BP_FightingCamera")
unreal.log("  6. Set up IMC_Fighter input mappings (keyboard/gamepad)")
unreal.log("=" * 60)
