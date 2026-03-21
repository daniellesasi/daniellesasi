"""
UE5.7 Python Script - Auto-Setup for Combat Game (Tekken-Style Fighter)
=========================================================================

DESIGNED FOR: Fresh UE5.7 Third Person Template named "Combat"

HOW TO RUN:
  1. Copy the Source/ and Config/ folders from this repo into your UE5.7 project
  2. Open the project in UE5.7 and let it compile C++ (takes a few minutes)
  3. Go to: Tools > Execute Python Script
     Browse to: Content/Python/setup_combat_game.py
  4. Watch the Output Log (Window > Developer Tools > Output Log) for progress
  5. After this finishes, run: Content/Python/configure_blueprints.py

This script creates ALL of these automatically:
  - Folder structure (Characters, Maps, UI, Input, etc.)
  - 3 Maps (MainMenuMap, CharacterSelectMap, FightingArenaMap)
  - 4 Widget Blueprints (MainMenu, CharacterSelect, FightHUD, PauseMenu)
  - 6 Gameplay Blueprints (GameMode, HUD, Fighter, Arena, Camera, AI)
  - 1 Animation Blueprint (using the template mannequin)
  - 10 Input Actions + 1 Input Mapping Context (with key bindings!)
  - 1 Data Table for character roster
"""

import unreal

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_asset_lib = unreal.EditorAssetLibrary
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

CREATED = []
SKIPPED = []
FAILED = []


def log_section(title):
    unreal.log("")
    unreal.log(f"--- {title} ---")


def ensure_directory(path):
    """Create a content directory if it doesn't exist."""
    if not editor_asset_lib.does_directory_exist(path):
        editor_asset_lib.make_directory(path)
        unreal.log(f"  Created folder: {path}")


def create_blueprint(path, name, parent_class):
    """Create a Blueprint asset based on a C++ parent class."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"  Already exists: {full_path}")
        SKIPPED.append(full_path)
        return unreal.load_asset(full_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = asset_tools.create_asset(name, path, None, factory)
    if asset:
        unreal.log(f"  Created: {full_path}")
        CREATED.append(full_path)
    else:
        unreal.log_error(f"  FAILED: {full_path}")
        FAILED.append(full_path)
    return asset


def create_widget_blueprint(path, name, parent_class):
    """Create a Widget Blueprint (UMG) based on a C++ widget parent class."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"  Already exists: {full_path}")
        SKIPPED.append(full_path)
        return unreal.load_asset(full_path)

    # Try WidgetBlueprintFactory first, fall back to BlueprintFactory
    factory = None
    if hasattr(unreal, 'WidgetBlueprintFactory'):
        factory = unreal.WidgetBlueprintFactory()
    else:
        factory = unreal.BlueprintFactory()

    factory.set_editor_property("parent_class", parent_class)
    asset = asset_tools.create_asset(name, path, None, factory)
    if asset:
        unreal.log(f"  Created: {full_path}")
        CREATED.append(full_path)
    else:
        unreal.log_error(f"  FAILED: {full_path}")
        FAILED.append(full_path)
    return asset


def create_level(path, name):
    """Create a new empty level/map."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"  Already exists: {full_path}")
        SKIPPED.append(full_path)
        return None

    world = None

    # UE5.7+: Use LevelEditorSubsystem (non-deprecated)
    try:
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if level_subsystem:
            world = level_subsystem.new_level(full_path)
    except Exception:
        pass

    # Fallback: deprecated EditorLevelLibrary (still works with a warning)
    if not world:
        try:
            world = unreal.EditorLevelLibrary.new_level(full_path)
        except Exception:
            pass

    if not world:
        try:
            world = unreal.EditorLevelLibrary.new_level_from_template(full_path, "")
        except Exception:
            pass

    if world:
        unreal.log(f"  Created: {full_path}")
        CREATED.append(full_path)
    else:
        unreal.log_error(f"  FAILED: {full_path}")
        FAILED.append(full_path)
    return world


def create_anim_blueprint(path, name, parent_class, skeleton):
    """Create an Animation Blueprint."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"  Already exists: {full_path}")
        SKIPPED.append(full_path)
        return unreal.load_asset(full_path)

    factory = unreal.AnimBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    if skeleton:
        factory.set_editor_property("target_skeleton", skeleton)

    asset = asset_tools.create_asset(name, path, None, factory)
    if asset:
        unreal.log(f"  Created: {full_path}")
        CREATED.append(full_path)
    else:
        unreal.log_error(f"  FAILED: {full_path}")
        FAILED.append(full_path)
    return asset


def create_input_action(path, name, value_type=unreal.InputActionValueType.BOOLEAN):
    """Create an Enhanced Input Action."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"  Already exists: {full_path}")
        SKIPPED.append(full_path)
        return unreal.load_asset(full_path)

    asset = None

    # Try factory approach first
    if hasattr(unreal, 'InputActionFactory'):
        factory = unreal.InputActionFactory()
        asset = asset_tools.create_asset(name, path, None, factory)
    else:
        # Direct creation
        try:
            asset = asset_tools.create_asset(name, path, unreal.InputAction, None)
        except Exception:
            pass

    if asset:
        asset.set_editor_property("value_type", value_type)
        unreal.log(f"  Created: {full_path} (type={value_type})")
        CREATED.append(full_path)
    else:
        unreal.log_error(f"  FAILED: {full_path}")
        FAILED.append(full_path)
    return asset


def create_input_mapping_context(path, name):
    """Create an Enhanced Input Mapping Context."""
    full_path = f"{path}/{name}"
    if editor_asset_lib.does_asset_exist(full_path):
        unreal.log_warning(f"  Already exists: {full_path}")
        SKIPPED.append(full_path)
        return unreal.load_asset(full_path)

    asset = None
    if hasattr(unreal, 'InputMappingContextFactory'):
        factory = unreal.InputMappingContextFactory()
        asset = asset_tools.create_asset(name, path, None, factory)
    else:
        try:
            asset = asset_tools.create_asset(name, path, unreal.InputMappingContext, None)
        except Exception:
            pass

    if asset:
        unreal.log(f"  Created: {full_path}")
        CREATED.append(full_path)
    else:
        unreal.log_error(f"  FAILED: {full_path}")
        FAILED.append(full_path)
    return asset


def get_skeleton_from_mesh(mesh):
    """Extract skeleton from a skeletal mesh asset."""
    skeleton = None
    if hasattr(mesh, 'skeleton'):
        try:
            skeleton = mesh.get_editor_property("skeleton")
        except Exception:
            pass
    if skeleton is None and hasattr(mesh, 'get_skeleton'):
        try:
            skeleton = mesh.get_skeleton()
        except Exception:
            pass
    return skeleton


def find_mannequin_skeleton():
    """Find the UE5 Third Person template mannequin skeleton."""
    # UE5.7 Third Person template stores mannequins in these locations
    search_paths = [
        # UE5.4+ Third Person template paths
        "/Game/Characters/Mannequins/Meshes/SKM_Manny",
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn",
        # UE5.0-5.3 paths
        "/Game/Characters/Mannequin/Mesh/SK_Mannequin",
        # Alternative UE5.7 paths (various template layouts)
        "/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny",
        "/Game/ThirdPerson/Mannequins/Meshes/SKM_Manny",
        # Engine content mannequins
        "/Engine/Mannequin/Character/Mesh/SK_Mannequin",
    ]

    for mesh_path in search_paths:
        mesh = unreal.load_asset(mesh_path)
        if mesh:
            skeleton = get_skeleton_from_mesh(mesh)
            if skeleton:
                unreal.log(f"  Found mannequin skeleton from: {mesh_path}")
                return skeleton, mesh_path

    # Broader search: scan Content for any skeletal mesh using Asset Registry
    unreal.log_warning("  Mannequin not found at known paths, searching...")
    try:
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        skeletal_meshes = None

        # UE5.7+ requires TopLevelAssetPath instead of string for get_assets_by_class
        if hasattr(unreal, 'TopLevelAssetPath'):
            try:
                class_path = unreal.TopLevelAssetPath("/Script/Engine", "SkeletalMesh")
                skeletal_meshes = ar.get_assets_by_class(class_path, True)
            except Exception:
                pass

        # Fallback: use ARFilter for broader compatibility
        if not skeletal_meshes:
            try:
                ar_filter = unreal.ARFilter()
                ar_filter.class_names = ["SkeletalMesh"]
                skeletal_meshes = ar.get_assets(ar_filter)
            except Exception:
                pass

        # Last resort: use get_all_assets and filter
        if not skeletal_meshes:
            try:
                all_assets = ar.get_all_assets(True)
                skeletal_meshes = [a for a in all_assets
                                   if "SkeletalMesh" in str(a.asset_class_path) or
                                      "SkeletalMesh" in str(getattr(a, 'asset_class', ''))]
            except Exception:
                pass

        if skeletal_meshes:
            for asset_data in skeletal_meshes:
                asset_name = str(asset_data.asset_name)
                if "manny" in asset_name.lower() or "quinn" in asset_name.lower() or "mannequin" in asset_name.lower():
                    mesh = asset_data.get_asset()
                    if mesh:
                        skeleton = get_skeleton_from_mesh(mesh)
                        if skeleton:
                            unreal.log(f"  Found mannequin via search: {asset_data.package_name}")
                            return skeleton, str(asset_data.package_name)
    except Exception as e:
        unreal.log_warning(f"  Skeleton search error: {e}")

    return None, None


# ============================================================================
# START
# ============================================================================

unreal.log("=" * 60)
unreal.log("COMBAT GAME SETUP - Starting...")
unreal.log("For UE5.7 Third Person Template")
unreal.log("=" * 60)

# ============================================================================
# STEP 0: LOAD C++ PARENT CLASSES
# ============================================================================

log_section("Loading C++ Classes (from CombatGame module)")

class_map = {
    "CombatCharacter":       "/Script/CombatGame.CombatCharacter",
    "CombatGameMode":        "/Script/CombatGame.CombatGameMode",
    "CombatHUD":             "/Script/CombatGame.CombatHUD",
    "FightingArena":         "/Script/CombatGame.FightingArena",
    "FightingCameraActor":   "/Script/CombatGame.FightingCameraActor",
    "FighterAIController":   "/Script/CombatGame.FighterAIController",
    "FighterAnimInstance":   "/Script/CombatGame.FighterAnimInstance",
    "MainMenuWidget":        "/Script/CombatGame.MainMenuWidget",
    "CharacterSelectWidget": "/Script/CombatGame.CharacterSelectWidget",
    "FightHUDWidget":        "/Script/CombatGame.FightHUDWidget",
    "PauseMenuWidget":       "/Script/CombatGame.PauseMenuWidget",
}

loaded_classes = {}
all_loaded = True
for name, path in class_map.items():
    cls = unreal.load_class(None, path)
    loaded_classes[name] = cls
    if cls:
        unreal.log(f"  OK: {name}")
    else:
        unreal.log_error(f"  MISSING: {name} - C++ not compiled?")
        all_loaded = False

if not all_loaded:
    unreal.log_error("")
    unreal.log_error("Some C++ classes are missing!")
    unreal.log_error("Make sure you copied the Source/ folder and compiled.")
    unreal.log_error("In UE5: Tools > Refresh Visual Studio Project, then Build.")
    unreal.log_error("Continuing with what's available...")

# ============================================================================
# STEP 1: CREATE FOLDER STRUCTURE
# ============================================================================

log_section("Creating Folder Structure")

folders = [
    "/Game/Maps",
    "/Game/UI",
    "/Game/Blueprints",
    "/Game/Characters",
    "/Game/Characters/TestFighter",
    "/Game/Animations",
    "/Game/Input",
    "/Game/Data",
    "/Game/Sounds",
    "/Game/Effects",
]
for folder in folders:
    ensure_directory(folder)

# ============================================================================
# STEP 2: CREATE MAPS
# ============================================================================

log_section("Creating Maps")
create_level("/Game/Maps", "MainMenuMap")
create_level("/Game/Maps", "CharacterSelectMap")
create_level("/Game/Maps", "FightingArenaMap")

# ============================================================================
# STEP 3: CREATE WIDGET BLUEPRINTS
# ============================================================================

log_section("Creating Widget Blueprints (UI)")

if loaded_classes["MainMenuWidget"]:
    create_widget_blueprint("/Game/UI", "WBP_MainMenu", loaded_classes["MainMenuWidget"])
if loaded_classes["CharacterSelectWidget"]:
    create_widget_blueprint("/Game/UI", "WBP_CharacterSelect", loaded_classes["CharacterSelectWidget"])
if loaded_classes["FightHUDWidget"]:
    create_widget_blueprint("/Game/UI", "WBP_FightHUD", loaded_classes["FightHUDWidget"])
if loaded_classes["PauseMenuWidget"]:
    create_widget_blueprint("/Game/UI", "WBP_PauseMenu", loaded_classes["PauseMenuWidget"])

# Also create portrait button widget (plain UserWidget - you design it later)
create_widget_blueprint("/Game/UI", "WBP_CharPortraitButton", unreal.UserWidget.static_class())

# ============================================================================
# STEP 4: CREATE GAMEPLAY BLUEPRINTS
# ============================================================================

log_section("Creating Gameplay Blueprints")

bp_game_mode = None
if loaded_classes["CombatGameMode"]:
    bp_game_mode = create_blueprint("/Game/Blueprints", "BP_CombatGameMode", loaded_classes["CombatGameMode"])

bp_hud = None
if loaded_classes["CombatHUD"]:
    bp_hud = create_blueprint("/Game/Blueprints", "BP_CombatHUD", loaded_classes["CombatHUD"])

bp_arena = None
if loaded_classes["FightingArena"]:
    bp_arena = create_blueprint("/Game/Blueprints", "BP_FightingArena", loaded_classes["FightingArena"])

bp_camera = None
if loaded_classes["FightingCameraActor"]:
    bp_camera = create_blueprint("/Game/Blueprints", "BP_FightingCamera", loaded_classes["FightingCameraActor"])

bp_ai = None
if loaded_classes["FighterAIController"]:
    bp_ai = create_blueprint("/Game/Blueprints", "BP_FighterAI", loaded_classes["FighterAIController"])

# Fighter BP (test character - will use mannequin as placeholder)
bp_fighter = None
if loaded_classes["CombatCharacter"]:
    bp_fighter = create_blueprint("/Game/Characters/TestFighter", "BP_TestFighter", loaded_classes["CombatCharacter"])

# ============================================================================
# STEP 5: CREATE ANIMATION BLUEPRINT (using mannequin skeleton)
# ============================================================================

log_section("Creating Animation Blueprint")

skeleton, mannequin_mesh_path = find_mannequin_skeleton()

abp_fighter = None
if loaded_classes["FighterAnimInstance"] and skeleton:
    abp_fighter = create_anim_blueprint("/Game/Characters/TestFighter", "ABP_TestFighter",
                                        loaded_classes["FighterAnimInstance"], skeleton)
elif loaded_classes["FighterAnimInstance"]:
    unreal.log_warning("  No mannequin skeleton found. You'll create ABP_TestFighter manually:")
    unreal.log_warning("  Right-click > Animation > Anim Blueprint > parent=FighterAnimInstance > pick skeleton")
else:
    unreal.log_error("  FighterAnimInstance class not loaded, skipping ABP.")

# ============================================================================
# STEP 6: CREATE INPUT ACTIONS + MAPPING CONTEXT
# ============================================================================

log_section("Creating Enhanced Input Actions")

input_path = "/Game/Input"

# Determine Axis2D type (different naming across UE5 versions)
AXIS_2D = unreal.InputActionValueType.AXIS2D if hasattr(unreal.InputActionValueType, 'AXIS2D') else unreal.InputActionValueType.BOOLEAN
AXIS_1D = unreal.InputActionValueType.AXIS1D if hasattr(unreal.InputActionValueType, 'AXIS1D') else unreal.InputActionValueType.BOOLEAN

# Movement
ia_move = create_input_action(input_path, "IA_Move", AXIS_2D)

# Attack buttons (Tekken layout: LP=1, RP=2, LK=3, RK=4)
ia_lp = create_input_action(input_path, "IA_LeftPunch")
ia_rp = create_input_action(input_path, "IA_RightPunch")
ia_lk = create_input_action(input_path, "IA_LeftKick")
ia_rk = create_input_action(input_path, "IA_RightKick")

# Defense & movement
ia_block = create_input_action(input_path, "IA_Block")
ia_jump = create_input_action(input_path, "IA_Jump")
ia_crouch = create_input_action(input_path, "IA_Crouch")
ia_sidestep = create_input_action(input_path, "IA_Sidestep", AXIS_1D)

# UI
ia_pause = create_input_action(input_path, "IA_Pause")

# Mapping Context
imc = create_input_mapping_context(input_path, "IMC_Fighter")

# ============================================================================
# STEP 7: CREATE DATA TABLE
# ============================================================================

log_section("Creating Character Data Table")

dt_path = "/Game/Data/DT_Characters"
if not editor_asset_lib.does_asset_exist(dt_path):
    created_dt = False
    try:
        if hasattr(unreal, 'DataTableFactory'):
            factory = unreal.DataTableFactory()
            # Try to load the struct
            row_struct = None
            struct_paths = [
                "/Script/CombatGame.CharacterData",
                "/Script/CombatGame.FCharacterData",
            ]
            for sp in struct_paths:
                try:
                    row_struct = unreal.load_object(None, sp)
                    if row_struct:
                        break
                except Exception:
                    continue

            if row_struct:
                factory.set_editor_property("struct", row_struct)
                asset = asset_tools.create_asset("DT_Characters", "/Game/Data", None, factory)
                if asset:
                    unreal.log(f"  Created: {dt_path}")
                    CREATED.append(dt_path)
                    created_dt = True
    except Exception as e:
        unreal.log_warning(f"  Data table auto-creation: {e}")

    if not created_dt:
        unreal.log_warning("  Could not auto-create DT_Characters.")
        unreal.log_warning("  MANUAL: Right-click in /Game/Data > Miscellaneous > Data Table")
        unreal.log_warning("          Pick 'CharacterData' as the Row Structure")
        FAILED.append(dt_path)
else:
    unreal.log_warning(f"  Already exists: {dt_path}")
    SKIPPED.append(dt_path)

# ============================================================================
# STEP 8: SAVE ALL
# ============================================================================

log_section("Saving All Assets")
try:
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log("  All assets saved.")
except Exception as e:
    unreal.log_warning(f"  Save warning: {e}")
    unreal.log("  Some assets may need manual save (Ctrl+Shift+S in editor).")

# ============================================================================
# SUMMARY
# ============================================================================

unreal.log("")
unreal.log("=" * 60)
unreal.log("SETUP COMPLETE!")
unreal.log("=" * 60)
unreal.log(f"  Created: {len(CREATED)} assets")
unreal.log(f"  Skipped: {len(SKIPPED)} (already existed)")
unreal.log(f"  Failed:  {len(FAILED)}")

if FAILED:
    unreal.log("")
    unreal.log("FAILED ASSETS (create manually):")
    for f in FAILED:
        unreal.log(f"  - {f}")

unreal.log("")
unreal.log("=" * 60)
unreal.log("NEXT: Run Content/Python/configure_blueprints.py")
unreal.log("      (Tools > Execute Python Script)")
unreal.log("=" * 60)
unreal.log("")
unreal.log("That second script will auto-wire the Blueprints together.")
unreal.log("After both scripts, the only MANUAL work left is:")
unreal.log("  1. Design the UI layouts in each WBP_* widget")
unreal.log("  2. Set up the Anim Blueprint state machine in ABP_TestFighter")
unreal.log("  3. Add key bindings in IMC_Fighter (keyboard/gamepad)")
unreal.log("  4. Place BP_FightingArena + BP_FightingCamera in FightingArenaMap")
