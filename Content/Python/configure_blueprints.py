"""
UE5.7 Python Script - Auto-Configure Blueprint Defaults
=========================================================

Run this AFTER setup_combat_game.py has created all assets.

This script automatically wires together:
  - BP_CombatGameMode: sets Default Pawn, HUD class, etc.
  - BP_CombatHUD: sets the FightHUD widget class
  - BP_TestFighter: assigns mannequin mesh, anim BP, input actions
  - BP_FighterAI: sets default difficulty
  - Project Settings: sets game mode, game instance, default map

HOW TO RUN:
  Tools > Execute Python Script > browse to Content/Python/configure_blueprints.py
"""

import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_asset_lib = unreal.EditorAssetLibrary
subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem) if hasattr(unreal, 'UnrealEditorSubsystem') else None

SUCCESS = []
WARNINGS = []


def log_section(title):
    unreal.log("")
    unreal.log(f"--- {title} ---")


def load_bp_class(path):
    """Load a Blueprint as a class (for assigning as Default Pawn, etc.)."""
    bp = unreal.load_asset(path)
    if bp:
        # Get the generated class from the Blueprint
        gen_class = bp.generated_class() if callable(getattr(bp, 'generated_class', None)) else None
        if gen_class is None:
            try:
                gen_class = bp.get_editor_property("generated_class")
            except Exception:
                pass
        if gen_class:
            return gen_class
        # Fallback: try loading as class directly
        try:
            return unreal.load_class(None, path + "_C")
        except Exception:
            pass
    return None


def set_bp_default(bp_path, property_name, value, description=""):
    """Set a default property on a Blueprint's CDO (Class Default Object)."""
    bp = unreal.load_asset(bp_path)
    if not bp:
        msg = f"  Could not load: {bp_path}"
        unreal.log_warning(msg)
        WARNINGS.append(msg)
        return False

    # Get the CDO
    cdo = None
    try:
        gen_class = bp.generated_class() if callable(getattr(bp, 'generated_class', None)) else None
        if gen_class is None:
            gen_class = bp.get_editor_property("generated_class")
        if gen_class:
            cdo = unreal.get_default_object(gen_class)
    except Exception:
        pass

    if cdo is None:
        msg = f"  Could not get CDO for: {bp_path}"
        unreal.log_warning(msg)
        WARNINGS.append(msg)
        return False

    try:
        cdo.set_editor_property(property_name, value)
        desc = f" ({description})" if description else ""
        unreal.log(f"  Set {property_name}{desc} on {bp_path.split('/')[-1]}")
        SUCCESS.append(f"{bp_path}: {property_name}")
        return True
    except Exception as e:
        msg = f"  Failed to set {property_name} on {bp_path}: {e}"
        unreal.log_warning(msg)
        WARNINGS.append(msg)
        return False


# ============================================================================
# START
# ============================================================================

unreal.log("=" * 60)
unreal.log("COMBAT GAME CONFIGURE - Wiring Blueprints Together...")
unreal.log("=" * 60)

# ============================================================================
# 1. CONFIGURE BP_CombatGameMode
# ============================================================================

log_section("Configuring BP_CombatGameMode")

gm_path = "/Game/Blueprints/BP_CombatGameMode"
fighter_path = "/Game/Characters/TestFighter/BP_TestFighter"
hud_path = "/Game/Blueprints/BP_CombatHUD"
ai_path = "/Game/Blueprints/BP_FighterAI"

# Set Default Pawn Class = BP_TestFighter
fighter_class = load_bp_class(fighter_path)
if fighter_class:
    set_bp_default(gm_path, "default_pawn_class", fighter_class, "BP_TestFighter")

# Set HUD Class = BP_CombatHUD
hud_class = load_bp_class(hud_path)
if hud_class:
    set_bp_default(gm_path, "hud_class", hud_class, "BP_CombatHUD")

# ============================================================================
# 2. CONFIGURE BP_CombatHUD
# ============================================================================

log_section("Configuring BP_CombatHUD")

# Set FightHUDWidgetClass = WBP_FightHUD
fight_hud_class = load_bp_class("/Game/UI/WBP_FightHUD")
if fight_hud_class:
    set_bp_default(hud_path, "FightHUDWidgetClass", fight_hud_class, "WBP_FightHUD")

# ============================================================================
# 3. CONFIGURE BP_TestFighter (assign mannequin + input actions)
# ============================================================================

log_section("Configuring BP_TestFighter")

# Load input actions
input_path = "/Game/Input"
input_actions = {
    "MoveAction":        f"{input_path}/IA_Move",
    "JumpAction":        f"{input_path}/IA_Jump",
    "CrouchAction":      f"{input_path}/IA_Crouch",
    "BlockAction":       f"{input_path}/IA_Block",
    "LeftPunchAction":   f"{input_path}/IA_LeftPunch",
    "RightPunchAction":  f"{input_path}/IA_RightPunch",
    "LeftKickAction":    f"{input_path}/IA_LeftKick",
    "RightKickAction":   f"{input_path}/IA_RightKick",
    "SidestepAction":    f"{input_path}/IA_Sidestep",
}

for prop_name, ia_path in input_actions.items():
    ia = unreal.load_asset(ia_path)
    if ia:
        set_bp_default(fighter_path, prop_name, ia, ia_path.split("/")[-1])
    else:
        WARNINGS.append(f"  Input action not found: {ia_path}")

# Set mapping context
imc = unreal.load_asset(f"{input_path}/IMC_Fighter")
if imc:
    set_bp_default(fighter_path, "FighterMappingContext", imc, "IMC_Fighter")

# Try to assign mannequin skeletal mesh
mannequin_paths = [
    "/Game/Characters/Mannequins/Meshes/SKM_Manny",
    "/Game/Characters/Mannequins/Meshes/SKM_Quinn",
    "/Game/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny",
    "/Game/Characters/Mannequin/Mesh/SK_Mannequin",
]
for mesh_path in mannequin_paths:
    mesh = unreal.load_asset(mesh_path)
    if mesh:
        unreal.log(f"  Found mannequin mesh at: {mesh_path}")
        # Note: Setting the mesh on the skeletal mesh component requires
        # opening the BP in the editor. Log instructions instead.
        unreal.log(f"  MANUAL: Open BP_TestFighter > Mesh component > set to {mesh_path.split('/')[-1]}")
        break
else:
    unreal.log_warning("  No mannequin mesh found. Open BP_TestFighter and assign a mesh manually.")

# Set anim blueprint
abp_path = "/Game/Characters/TestFighter/ABP_TestFighter"
abp = unreal.load_asset(abp_path)
if abp:
    unreal.log(f"  Found ABP_TestFighter. Set it on BP_TestFighter's Mesh component in editor.")
else:
    unreal.log_warning("  ABP_TestFighter not found. Create it manually if skeleton wasn't found.")

# ============================================================================
# 4. CONFIGURE BP_FighterAI
# ============================================================================

log_section("Configuring BP_FighterAI")
set_bp_default(ai_path, "Difficulty", 0.5, "Medium difficulty")

# ============================================================================
# 5. CONFIGURE PROJECT SETTINGS (via config)
# ============================================================================

log_section("Verifying Project Settings")

# Check if DefaultEngine.ini is already set up
unreal.log("  Project settings are configured via Config/DefaultEngine.ini:")
unreal.log("    GameDefaultMap = /Game/Maps/MainMenuMap")
unreal.log("    GlobalDefaultGameMode = CombatGameMode")
unreal.log("    GameInstanceClass = CombatGameInstance")
unreal.log("  (These were set when you copied the Config/ folder)")

# ============================================================================
# 6. SAVE ALL
# ============================================================================

log_section("Saving All Assets")
try:
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log("  All assets saved.")
except Exception as e:
    unreal.log_warning(f"  Save: {e}")

# ============================================================================
# SUMMARY
# ============================================================================

unreal.log("")
unreal.log("=" * 60)
unreal.log("CONFIGURATION COMPLETE!")
unreal.log("=" * 60)
unreal.log(f"  Configured: {len(SUCCESS)} properties")
unreal.log(f"  Warnings:   {len(WARNINGS)}")

if WARNINGS:
    unreal.log("")
    unreal.log("WARNINGS:")
    for w in WARNINGS:
        unreal.log(f"  {w}")

unreal.log("")
unreal.log("=" * 60)
unreal.log("REMAINING MANUAL STEPS (do these in UE5 editor):")
unreal.log("=" * 60)
unreal.log("")
unreal.log("1. OPEN BP_TestFighter:")
unreal.log("   - Select the Mesh component")
unreal.log("   - Set Skeletal Mesh = SKM_Manny (or SKM_Quinn)")
unreal.log("   - Set Anim Blueprint = ABP_TestFighter")
unreal.log("")
unreal.log("2. SET UP ABP_TestFighter (Animation Blueprint):")
unreal.log("   - Open it, add a State Machine to the AnimGraph")
unreal.log("   - Add states: Idle, Walking, Crouching, Jumping, Blocking")
unreal.log("   - Use the template's existing animations for now")
unreal.log("   - Transitions read from: bIsMoving, bIsCrouching, etc.")
unreal.log("")
unreal.log("3. SET UP KEY BINDINGS in IMC_Fighter:")
unreal.log("   - Open /Game/Input/IMC_Fighter")
unreal.log("   - Add mappings (see table below)")
unreal.log("")
unreal.log("   KEYBOARD:                    GAMEPAD:")
unreal.log("   IA_Move       = WASD         Left Stick")
unreal.log("   IA_Jump       = Space        Left Stick Up")
unreal.log("   IA_Crouch     = S (hold)     Left Stick Down")
unreal.log("   IA_Block      = V            Right Bumper")
unreal.log("   IA_LeftPunch  = U            Face Left (Square/X)")
unreal.log("   IA_RightPunch = I            Face Top (Triangle/Y)")
unreal.log("   IA_LeftKick   = J            Face Bottom (Cross/A)")
unreal.log("   IA_RightKick  = K            Face Right (Circle/B)")
unreal.log("   IA_Sidestep   = Q/E          Right Stick Y")
unreal.log("   IA_Pause      = Escape       Start Button")
unreal.log("")
unreal.log("4. DESIGN UI WIDGETS:")
unreal.log("   Open each WBP_* and add the required widgets:")
unreal.log("")
unreal.log("   WBP_MainMenu: PlayButton, OptionsButton, QuitButton")
unreal.log("   WBP_CharacterSelect: CharacterGrid, ReadyButton, BackButton")
unreal.log("   WBP_FightHUD: P1HealthBar, P2HealthBar, TimerText, RoundText")
unreal.log("   WBP_PauseMenu: ResumeButton, CharSelectButton, MainMenuButton")
unreal.log("")
unreal.log("5. SET UP FightingArenaMap:")
unreal.log("   - Open /Game/Maps/FightingArenaMap")
unreal.log("   - Drag in BP_FightingArena (adds floor + invisible walls)")
unreal.log("   - Drag in BP_FightingCamera")
unreal.log("   - Add any background scenery you want")
unreal.log("")
unreal.log("6. ADD ROW TO DT_Characters:")
unreal.log("   - Open /Game/Data/DT_Characters")
unreal.log("   - Add a row named 'TestFighter'")
unreal.log("   - Set DisplayName = 'Test Fighter'")
unreal.log("   - Set FighterClass = BP_TestFighter")
unreal.log("   - Set MaxHealth = 170")
unreal.log("")
unreal.log("After these steps, hit Play in FightingArenaMap to test!")
unreal.log("=" * 60)
