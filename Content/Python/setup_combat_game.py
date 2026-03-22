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


def find_testfighter_skeleton():
    """Find the TestFighter_Skelaton skeleton asset."""
    # Search common locations for the user's custom skeleton
    search_paths = [
        "/Game/Characters/TestFighter/TestFighter_Skelaton",
        "/Game/Characters/TestFighter_Skelaton",
        "/Game/Animations/TestFighter_Skelaton",
        "/Game/TestFighter_Skelaton",
    ]

    for skel_path in search_paths:
        skeleton = unreal.load_asset(skel_path)
        if skeleton:
            unreal.log(f"  Found TestFighter_Skelaton at: {skel_path}")
            return skeleton, skel_path

    # Broader search via Asset Registry
    unreal.log_warning("  TestFighter_Skelaton not found at known paths, searching...")
    try:
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        all_assets = None

        if hasattr(unreal, 'TopLevelAssetPath'):
            try:
                class_path = unreal.TopLevelAssetPath("/Script/Engine", "Skeleton")
                all_assets = ar.get_assets_by_class(class_path, True)
            except Exception:
                pass

        if not all_assets:
            try:
                ar_filter = unreal.ARFilter()
                ar_filter.class_names = ["Skeleton"]
                all_assets = ar.get_assets(ar_filter)
            except Exception:
                pass

        if all_assets:
            for asset_data in all_assets:
                asset_name = str(asset_data.asset_name)
                if "testfighter" in asset_name.lower() or "skelaton" in asset_name.lower():
                    skeleton = asset_data.get_asset()
                    if skeleton:
                        unreal.log(f"  Found TestFighter_Skelaton via search: {asset_data.package_name}")
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
    "CombatPlayerController": "/Script/CombatGame.CombatPlayerController",
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

bp_player_controller = None
if loaded_classes["CombatPlayerController"]:
    bp_player_controller = create_blueprint("/Game/Blueprints", "BP_CombatPlayerController", loaded_classes["CombatPlayerController"])

# Fighter BP (test character - will use mannequin as placeholder)
bp_fighter = None
if loaded_classes["CombatCharacter"]:
    bp_fighter = create_blueprint("/Game/Characters/TestFighter", "BP_TestFighter", loaded_classes["CombatCharacter"])

# ============================================================================
# STEP 5: CREATE ANIMATION BLUEPRINT (using mannequin skeleton)
# ============================================================================

log_section("Creating Animation Blueprint")

skeleton, skeleton_path = find_testfighter_skeleton()

abp_fighter = None
if loaded_classes["FighterAnimInstance"] and skeleton:
    abp_fighter = create_anim_blueprint("/Game/Characters/TestFighter", "ABP_TestFighter",
                                        loaded_classes["FighterAnimInstance"], skeleton)
elif loaded_classes["FighterAnimInstance"]:
    unreal.log_warning("  TestFighter_Skelaton not found. You'll create ABP_TestFighter manually:")
    unreal.log_warning("  Right-click > Animation > Anim Blueprint > parent=FighterAnimInstance > pick TestFighter_Skelaton")
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
# STEP 6b: SET UP KEY BINDINGS IN IMC_Fighter
# ============================================================================

log_section("Setting Up Key Bindings in IMC_Fighter")

if imc:
    try:
        # Reload to make sure we have the latest
        imc = unreal.load_asset(f"{input_path}/IMC_Fighter")

        # Helper to add a key mapping
        def add_key_mapping(mapping_context, action, key_name, modifiers=None):
            """Add a key mapping to the input mapping context."""
            if not action or not mapping_context:
                return
            try:
                key = unreal.Key(key_name) if isinstance(key_name, str) else key_name
                mapping = mapping_context.map_key(action, key)
                if modifiers and mapping:
                    for mod in modifiers:
                        mapping.modifiers.append(mod)
                unreal.log(f"    Mapped {action.get_name()} -> {key_name}")
            except Exception as e:
                unreal.log_warning(f"    Could not map {key_name}: {e}")

        # Reload input actions
        ia_move = unreal.load_asset(f"{input_path}/IA_Move")
        ia_lp = unreal.load_asset(f"{input_path}/IA_LeftPunch")
        ia_rp = unreal.load_asset(f"{input_path}/IA_RightPunch")
        ia_lk = unreal.load_asset(f"{input_path}/IA_LeftKick")
        ia_rk = unreal.load_asset(f"{input_path}/IA_RightKick")
        ia_block = unreal.load_asset(f"{input_path}/IA_Block")
        ia_jump = unreal.load_asset(f"{input_path}/IA_Jump")
        ia_crouch = unreal.load_asset(f"{input_path}/IA_Crouch")
        ia_sidestep = unreal.load_asset(f"{input_path}/IA_Sidestep")
        ia_pause = unreal.load_asset(f"{input_path}/IA_Pause")

        # --- KEYBOARD MAPPINGS ---
        unreal.log("  Keyboard mappings:")

        # Movement WASD (Axis2D needs modifiers for direction)
        # W = Forward (Swizzle YXZ so Y axis becomes primary)
        if ia_move:
            try:
                swizzle_mod = unreal.InputModifierSwizzleAxis()
                negate_mod = unreal.InputModifierNegate()

                # W key - forward (+Y) - needs Swizzle
                mapping_w = imc.map_key(ia_move, unreal.Key("W"))
                if mapping_w:
                    mapping_w.modifiers.append(swizzle_mod)
                    unreal.log("    Mapped IA_Move -> W (forward)")

                # S key - backward (-Y) - needs Swizzle + Negate
                mapping_s = imc.map_key(ia_move, unreal.Key("S"))
                if mapping_s:
                    mapping_s.modifiers.append(swizzle_mod)
                    mapping_s.modifiers.append(negate_mod)
                    unreal.log("    Mapped IA_Move -> S (backward)")

                # A key - left (-X) - needs Negate
                mapping_a = imc.map_key(ia_move, unreal.Key("A"))
                if mapping_a:
                    mapping_a.modifiers.append(negate_mod)
                    unreal.log("    Mapped IA_Move -> A (left)")

                # D key - right (+X) - no modifier needed
                mapping_d = imc.map_key(ia_move, unreal.Key("D"))
                if mapping_d:
                    unreal.log("    Mapped IA_Move -> D (right)")
            except Exception as e:
                unreal.log_warning(f"    WASD mapping error: {e}")
                # Fallback: map without modifiers
                add_key_mapping(imc, ia_move, "W")
                add_key_mapping(imc, ia_move, "A")
                add_key_mapping(imc, ia_move, "S")
                add_key_mapping(imc, ia_move, "D")

        # Gamepad left stick for movement
        try:
            if ia_move:
                imc.map_key(ia_move, unreal.Key("Gamepad_LeftStick2D"))
                unreal.log("    Mapped IA_Move -> Gamepad Left Stick")
        except Exception:
            pass

        # Attack keys - keyboard
        add_key_mapping(imc, ia_lp, "U")
        add_key_mapping(imc, ia_rp, "I")
        add_key_mapping(imc, ia_lk, "J")
        add_key_mapping(imc, ia_rk, "K")

        # Attack keys - gamepad
        add_key_mapping(imc, ia_lp, "Gamepad_FaceButton_Left")
        add_key_mapping(imc, ia_rp, "Gamepad_FaceButton_Top")
        add_key_mapping(imc, ia_lk, "Gamepad_FaceButton_Bottom")
        add_key_mapping(imc, ia_rk, "Gamepad_FaceButton_Right")

        # Defense & movement - keyboard
        add_key_mapping(imc, ia_block, "V")
        add_key_mapping(imc, ia_jump, "SpaceBar")
        add_key_mapping(imc, ia_crouch, "LeftControl")
        add_key_mapping(imc, ia_pause, "Escape")

        # Defense & movement - gamepad
        add_key_mapping(imc, ia_block, "Gamepad_RightShoulder")
        add_key_mapping(imc, ia_jump, "Gamepad_FaceButton_Bottom")
        add_key_mapping(imc, ia_pause, "Gamepad_Special_Right")

        # Sidestep - keyboard Q/E
        if ia_sidestep:
            try:
                negate_mod = unreal.InputModifierNegate()
                mapping_q = imc.map_key(ia_sidestep, unreal.Key("Q"))
                if mapping_q:
                    mapping_q.modifiers.append(negate_mod)
                    unreal.log("    Mapped IA_Sidestep -> Q (left)")
                add_key_mapping(imc, ia_sidestep, "E")
            except Exception as e:
                add_key_mapping(imc, ia_sidestep, "Q")
                add_key_mapping(imc, ia_sidestep, "E")

        unreal.log("  Key bindings configured!")
    except Exception as e:
        unreal.log_warning(f"  Key binding setup error: {e}")
        unreal.log_warning("  You can set these up manually in IMC_Fighter.")
else:
    unreal.log_warning("  IMC_Fighter not available, skipping key bindings.")

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
unreal.log("That second script will auto-wire the Blueprints together")
unreal.log("and populate the Widget Blueprints with UI elements.")
unreal.log("")
unreal.log("After both scripts, the only MANUAL work left is:")
unreal.log("  1. Assign your skeletal mesh to BP_TestFighter (using TestFighter_Skelaton)")
unreal.log("  2. Set up ABP_TestFighter anim state machine")
unreal.log("  3. Style/reposition widgets in each WBP_* (optional)")
unreal.log("  4. Place BP_FightingArena + BP_FightingCamera in FightingArenaMap")
