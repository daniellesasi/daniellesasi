# Combat Game - Setup Guide for UE5.7 Third Person Template

## Overview

This turns a fresh UE5.7 **Third Person** template into a Tekken 3-style fighting game.
The C++ code handles ALL the game logic. You just need to set up assets in the editor.

**Two Python scripts automate most of the setup for you.**

---

## Step 1: Create the UE5 Project

1. Open **Unreal Engine 5.7**
2. Create a new project: **Games > Third Person** template
3. Name it **CombatGame** (must match exactly — the module name in all C++ source files is `CombatGame`)
4. Choose **C++** (not Blueprint) as the project type
5. Click **Create**

## Step 2: Copy Files from This Repo

Copy these folders from this GitHub repo into your UE5 project folder (next to the .uproject file):

| Copy This | Into Your Project |
|-----------|------------------|
| `Source/` | Replace the existing `Source/` folder |
| `Config/` | Replace the existing `Config/` folder |
| `Content/Python/` | Copy into `Content/Python/` |

Your project folder should look like:
```
Combat/
  Combat.uproject          (your UE5 project file)
  Config/                  (from this repo)
  Content/
    Python/                (from this repo - setup scripts)
    ThirdPerson/           (from the template - keep this!)
    Characters/            (from the template - keep this!)
  Source/                  (from this repo)
```

## Step 3: Compile C++ Code

1. Open the project in UE5.7
2. If prompted to rebuild modules, click **Yes**
3. Wait for compilation (check the bottom-right corner of the editor)
4. If it fails: **Tools > Refresh Visual Studio Project**, then rebuild

> **Tip:** If compilation fails, make sure the Source/ folder was fully copied and matches the repo.

## Step 4: Run the Setup Script (creates all assets)

1. Go to **Tools > Execute Python Script** (or **Edit > Execute Python Script**)
2. Browse to `Content/Python/setup_combat_game.py`
3. Click **Open** -- watch the Output Log for progress
4. This creates: Maps, Widget Blueprints, Gameplay Blueprints, Input Actions, etc.

## Step 5: Run the Configure Script (wires everything together)

1. Go to **Tools > Execute Python Script** again
2. Browse to `Content/Python/configure_blueprints.py`
3. This auto-sets GameMode defaults, input actions on the fighter, HUD class, etc.

---

## Step 6: Manual Setup (what the scripts CAN'T do)

After running both scripts, you only need to do these things by hand.

> **Note:** Key bindings (IMC_Fighter) and UI widget layouts are now **fully automated** by the scripts. The widgets are positioned in a Tekken-style layout with placeholder colors and text. You can tweak the layout later in the Widget Designer.

### 6a. Set Up BP_TestFighter's Mesh

1. Open `Content/Characters/TestFighter/BP_TestFighter`
2. Select the **Mesh** component in the Components panel
3. In Details, set **Skeletal Mesh Asset** using your `TestFighter_Skelaton` skeleton
4. Set **Anim Class** = `ABP_TestFighter`

### 6b. Set Up ABP_TestFighter (Animation Blueprint)

1. Open `Content/Characters/TestFighter/ABP_TestFighter` (created with `TestFighter_Skelaton`)
2. In the **AnimGraph**, add a **State Machine** named "Main"
3. Add these 10 states:

| State | Animation | Looping | Transition In |
|-------|-----------|---------|--------------|
| Idle | Anim_Idle | Yes | Default entry |
| Walking | Anim_Walk | Yes | bIsMoving = true |
| Jumping | Anim_Jump | No | bIsJumping = true |
| Crouching | Anim_Crouch | Yes | bIsCrouching = true |
| Blocking | Anim_Block | Yes | bIsBlocking = true |
| Attacking | Anim_Attack | No | bIsAttacking = true |
| HitStun | Anim_HitStun | No | bIsInHitStun = true |
| KnockedDown | Anim_KnockDown | No | bIsKnockedDown = true |
| Launched | Anim_Launched | No | bIsLaunched = true |
| Dead | Anim_Death | No | bIsDead = true |

4. Wire transitions back to **Idle** when each bool becomes false (except Dead)
5. Add cross-state transitions (e.g. any state -> HitStun, any state -> Dead)
6. Add a **Default Slot** node after the state machine (for attack montages)
7. Connect: State Machine > Default Slot > Output Pose

### 6c. Set Up the Arena Map

1. Open `Content/Maps/FightingArenaMap`
2. Drag `BP_FightingArena` from Content Browser into the level
3. Drag `BP_FightingCamera` into the level
4. Add a floor plane and any background you want

### 6d. Add a Character to DT_Characters

1. Open `Content/Data/DT_Characters`
2. Click **Add** to add a new row
3. Name the row `TestFighter`
4. Fill in:
   - DisplayName = `Test Fighter`
   - FighterClass = `BP_TestFighter`
   - MaxHealth = `170`
   - WalkSpeed = `300`

---

## Quick Test

1. Open `FightingArenaMap`
2. Make sure the **World Settings** (Window > World Settings) has:
   - GameMode Override = `BP_CombatGameMode`
3. Click **Play**
4. You should see two mannequins facing each other with the HUD

---

## Tekken Controls Reference

```
            Forward (toward opponent)
               ^
    Back < -- N -- > Forward
               v
            Backward (away from opponent)

   +--------+--------+
   |   LP   |   RP   |    LP = Left Punch (U / Square)
   |  (1)   |  (2)   |    RP = Right Punch (I / Triangle)
   +--------+--------+    LK = Left Kick (J / Cross)
   |   LK   |   RK   |    RK = Right Kick (K / Circle)
   |  (3)   |  (4)   |
   +--------+--------+

   Block = V / Right Bumper
   Sidestep = Q/E / Right Stick
```

---

## What the C++ Code Handles (No Setup Needed)

- Round system (best of 3, intro/fight/KO flow)
- Health system with smooth drain + chip damage on block
- 60-second round timer with timeout winner
- Combo system with input buffering and damage scaling
- Juggle system (launchers, air hits, max juggle limit)
- Block system (standing blocks high/mid, crouch blocks low)
- AI opponent with difficulty scaling (0.0 easy to 1.0 hard)
- Camera auto-tracking with zoom and hit shake
- HUD with health bars, timer, round indicators, combo counter
- Menu navigation (main menu > character select > fight > pause)
- Arena boundaries with invisible walls
- Animation integration with state machine variables

---

## Adding More Characters Later

1. Get a skeletal mesh (import FBX or use Marketplace assets) with its own Skeleton
2. Create a folder: `Content/Characters/[Name]/`
3. Create Animation Blueprint: `ABP_[Name]` (parent: FighterAnimInstance, using character's Skeleton)
4. Create Character Blueprint: `BP_[Name]` (parent: CombatCharacter)
5. Set up the mesh, animations (following the Anim_* naming convention), and move list on the BP
6. Add a row to `DT_Characters`

See the animation naming conventions at the top of `Source/CombatGame/Public/FighterAnimInstance.h`

---

## Troubleshooting

**"C++ classes not found" when running Python scripts:**
- Compile the C++ code first (bottom-right corner shows progress)
- If it fails: Tools > Refresh Visual Studio Project, then Build

**"No mannequin skeleton found":**
- The Third Person template mannequin path may differ in your UE5 version
- Open BP_TestFighter manually and assign the mesh

**Widgets crash on play:**
- Make sure the required widget names match EXACTLY (case-sensitive)
- Check the header file comments for each widget class

**Input not working:**
- Make sure IMC_Fighter has key bindings set up
- Check that BP_TestFighter has all Input Action properties assigned
