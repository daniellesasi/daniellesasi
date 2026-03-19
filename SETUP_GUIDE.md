# Combat Game - Tekken-Style Fighter for UE5

## Quick Start

1. Open `CombatGame.uproject` in Unreal Engine 5.3+
2. Let it compile the C++ code
3. Follow the steps below to add your characters

---

## Project Structure

```
CombatGame/
├── Config/                          # Engine, game, input configs
├── Content/
│   ├── Characters/                  # Character Blueprints & meshes
│   │   ├── Fighter01/
│   │   │   ├── BP_Fighter01         # Character Blueprint
│   │   │   ├── ABP_Fighter01        # Animation Blueprint
│   │   │   ├── SK_Fighter01         # Skeletal Mesh
│   │   │   └── Animations/          # All anims for this character
│   │   └── Fighter02/
│   ├── Animations/                  # Shared animations
│   ├── Maps/
│   │   ├── MainMenuMap
│   │   ├── CharacterSelectMap
│   │   ├── ArenaMap
│   │   ├── TempleMap
│   │   └── StreetsMap
│   ├── UI/
│   │   ├── WBP_FightHUD            # In-fight health bars, timer
│   │   ├── WBP_MainMenu            # Main menu
│   │   ├── WBP_CharacterSelect     # Character select screen
│   │   ├── WBP_PauseMenu           # Pause menu
│   │   └── WBP_CharPortraitButton  # Portrait button for char select
│   ├── Sounds/                      # Hit sounds, music, announcer
│   ├── Effects/                     # Niagara hit effects
│   └── Data/
│       └── DT_Characters            # Character data table
└── Source/CombatGame/               # All C++ source code
```

---

## Adding a New Character (Step by Step)

### 1. Import Your Skeletal Mesh
- Import your character model as `SK_[CharacterName]`
- Example: `SK_Fighter01`
- Place in: `Content/Characters/Fighter01/`

### 2. Create the Animation Blueprint
- Right-click SK_Fighter01 > Create > Anim Blueprint
- Name: `ABP_Fighter01`
- **Parent class: FighterAnimInstance**
- Set up a State Machine with these states:
  - **Idle** → plays `A_Fighter01_Idle`
  - **Locomotion** → uses `BS_Fighter01_Locomotion` (blend space)
  - **Crouch** → plays `A_Fighter01_Crouch`
  - **Jump** → plays `A_Fighter01_Jump`
  - **Block** → plays `A_Fighter01_BlockStand`
  - **CrouchBlock** → plays `A_Fighter01_BlockCrouch`
- Transitions use the variables from FighterAnimInstance (bIsMoving, bIsCrouching, etc.)
- **Attack montages play on top** via a Default Slot

### 3. Create the Character Blueprint
- Create Blueprint child of `CombatCharacter`
- Name: `BP_Fighter01`
- Place in: `Content/Characters/Fighter01/`
- In the Blueprint:
  - Set Mesh component → `SK_Fighter01`
  - Set Anim Blueprint → `ABP_Fighter01`
  - Set the **Input Actions** and **Mapping Context** (see Input Setup below)
  - Fill in the **MoveList** array with attacks
  - Fill in the **ComboRoutes** array with combo strings
  - Set hit reaction montages

### 4. Name Your Animations

| Animation | Naming Convention | Example |
|-----------|------------------|---------|
| Idle | `A_[Name]_Idle` | `A_Fighter01_Idle` |
| Walk Forward | `A_[Name]_WalkForward` | `A_Fighter01_WalkForward` |
| Walk Backward | `A_[Name]_WalkBackward` | `A_Fighter01_WalkBackward` |
| Crouch | `A_[Name]_Crouch` | `A_Fighter01_Crouch` |
| Jump | `A_[Name]_Jump` | `A_Fighter01_Jump` |
| Block (Standing) | `A_[Name]_BlockStand` | `A_Fighter01_BlockStand` |
| Block (Crouching) | `A_[Name]_BlockCrouch` | `A_Fighter01_BlockCrouch` |

#### Attack Montages

| Animation Montage | Example |
|-------------------|---------|
| `AM_[Name]_LeftJab` | `AM_Fighter01_LeftJab` |
| `AM_[Name]_RightStraight` | `AM_Fighter01_RightStraight` |
| `AM_[Name]_LeftLowKick` | `AM_Fighter01_LeftLowKick` |
| `AM_[Name]_RightHighKick` | `AM_Fighter01_RightHighKick` |
| `AM_[Name]_Launcher` | `AM_Fighter01_Launcher` |
| `AM_[Name]_Sweep` | `AM_Fighter01_Sweep` |

#### Reaction Montages

| Animation Montage | Example |
|-------------------|---------|
| `AM_[Name]_HitHigh` | `AM_Fighter01_HitHigh` |
| `AM_[Name]_HitMid` | `AM_Fighter01_HitMid` |
| `AM_[Name]_HitLow` | `AM_Fighter01_HitLow` |
| `AM_[Name]_BlockReaction` | `AM_Fighter01_BlockReaction` |
| `AM_[Name]_Knockdown` | `AM_Fighter01_Knockdown` |
| `AM_[Name]_GetUp` | `AM_Fighter01_GetUp` |
| `AM_[Name]_Launched` | `AM_Fighter01_Launched` |
| `AM_[Name]_Victory` | `AM_Fighter01_Victory` |
| `AM_[Name]_Intro` | `AM_Fighter01_Intro` |

### 5. Create Character Portraits
- `T_[Name]_Portrait` — square portrait for character select grid (256x256 recommended)
- `T_[Name]_FullBody` — full body render for selection screen (512x1024 recommended)

### 6. Register in Data Table
- Create Data Table: `DT_Characters` (Row Structure: `FCharacterData`)
- Add a row per character with:
  - DisplayName
  - Portrait texture reference
  - FullBody texture reference
  - FighterClass → `BP_Fighter01`
  - MaxHealth (default: 170)
  - WalkSpeed
  - MoveList and ComboRoutes

---

## Input Setup (Enhanced Input)

### Create these Input Actions in Content/Input/:

| Input Action | Type | Description |
|-------------|------|-------------|
| `IA_Move` | Axis2D (Vector2D) | Left stick / WASD |
| `IA_Jump` | Bool | Up / Spacebar |
| `IA_Crouch` | Bool | Down / Ctrl |
| `IA_Block` | Bool | Back + button / RB |
| `IA_LeftPunch` | Bool | Square / X |
| `IA_RightPunch` | Bool | Triangle / Y |
| `IA_LeftKick` | Bool | X / A |
| `IA_RightKick` | Bool | Circle / B |
| `IA_Sidestep` | Float | Up/Down on right stick |

### Create Input Mapping Context: `IMC_Fighter`

#### Gamepad (Recommended):
| Action | Binding |
|--------|---------|
| IA_Move | Left Thumbstick |
| IA_Jump | Left Thumbstick Up |
| IA_Crouch | Left Thumbstick Down |
| IA_Block | Right Bumper |
| IA_LeftPunch | Face Button Left (Square/X) |
| IA_RightPunch | Face Button Top (Triangle/Y) |
| IA_LeftKick | Face Button Bottom (Cross/A) |
| IA_RightKick | Face Button Right (Circle/B) |
| IA_Sidestep | Right Thumbstick Y |

#### Keyboard (Player 1):
| Action | Binding |
|--------|---------|
| IA_Move | WASD |
| IA_Jump | W / Space |
| IA_Crouch | S |
| IA_Block | V |
| IA_LeftPunch | U |
| IA_RightPunch | I |
| IA_LeftKick | J |
| IA_RightKick | K |
| IA_Sidestep | Q/E |

---

## Setting Up Moves

Each move in the **MoveList** has these key properties:

```
MoveName:           "LeftJab"
AttackType:         HighPunch
HitHeight:          High
Damage:             10
ChipDamage:         1
StartupFrames:      10      (how fast the move comes out)
ActiveFrames:       3       (how long the hitbox is active)
RecoveryFrames:     12      (vulnerability after attack)
HitStunFrames:      18      (how long opponent is stunned on hit)
BlockStunFrames:    8       (how long opponent is stunned on block)
KnockbackDistance:  100     (pushback on hit)
LaunchHeight:       0       (>0 makes it a launcher for juggles)
bKnocksDown:        false   (does it knock down?)
AttackMontage:      AM_Fighter01_LeftJab
HitEffect:          NS_HitSpark  (Niagara system)
HitSound:           S_PunchHit
```

### Example Move List (Tekken-Style):

| Move Name | Type | Height | Damage | Startup | Notes |
|-----------|------|--------|--------|---------|-------|
| LeftJab | HighPunch | High | 10 | 10f | Fast poke |
| RightStraight | MidPunch | Mid | 15 | 13f | Mid check |
| LeftLowPunch | LowPunch | Low | 8 | 12f | Low poke |
| RightUppercut | Launcher | Mid | 20 | 15f | LaunchHeight=500 |
| LeftHighKick | HighKick | High | 18 | 14f | Good range |
| RightMidKick | MidKick | Mid | 20 | 16f | Solid mid |
| LowSweep | Sweep | Low | 15 | 18f | KnocksDown=true |
| SpinKick | Special | Mid | 25 | 20f | Big damage, slow |

---

## Setting Up Combos

ComboRoutes define input sequences that trigger combo strings:

```
ComboName: "1-2 Punch"
InputSequence:
  [0] Direction=Neutral, Button=LeftPunch, MaxDelay=0.3
  [1] Direction=Neutral, Button=RightPunch, MaxDelay=0.3
Attacks:
  [0] LeftJab (from MoveList)
  [1] RightStraight (from MoveList)
```

---

## Creating Maps

### Required Maps:
1. **MainMenuMap** — Place a widget-spawning Blueprint or use Level Blueprint to show WBP_MainMenu
2. **CharacterSelectMap** — Shows WBP_CharacterSelect
3. **ArenaMap** (and other stage maps) — Place a `BP_Arena_[Name]` actor with floor mesh and scenery

### Arena Setup:
- Drop a `FightingArena` Blueprint into the level
- Set ArenaWidth/Depth as needed
- Add visual scenery around it (background buildings, skybox, etc.)
- The invisible walls auto-constrain fighters

---

## Tekken Controls Reference

```
               Forward (toward opponent)
                  ↑
    Back ← ──── N ──── → Forward
                  ↓
               Backward (away from opponent)

   ┌────────┬────────┐
   │   LP   │   RP   │    LP = Left Punch (1)
   │  (1)   │  (2)   │    RP = Right Punch (2)
   ├────────┼────────┤    LK = Left Kick (3)
   │   LK   │   RK   │    RK = Right Kick (4)
   │  (3)   │  (4)   │
   └────────┴────────┘

   Double-tap Forward = Forward Dash
   Double-tap Back    = Backdash
   Up/Down (3D axis)  = Sidestep
   Back + Block       = Standing Block
   Down-Back + Block  = Crouch Block
```

---

## What's Included (No Manual Setup Needed)

- **Round system** — Best of 3 rounds, with intro/fight/KO flow
- **Health system** — Smooth health bar drain with chip damage on block
- **Timer** — 60-second rounds, timeout winner by health
- **Combo system** — Input buffering, combo routes, damage scaling
- **Juggle system** — Launchers, air hits, max juggle limit
- **Block system** — Standing blocks mid/high, crouch blocks low, chip damage
- **AI opponent** — Difficulty-scaled (0.0-1.0) with approach/attack/block/punish behaviors
- **Camera** — Auto-tracks both fighters, zooms based on distance
- **HUD** — Health bars, timer, round indicators, combo counter, announcer text
- **Menus** — Main menu, character select with portraits, pause menu
- **Arena** — Bounded fighting area with invisible walls
- **Animation integration** — AnimInstance with state variables, montage support

---

## Tips

- Start with a simple moveset (4-6 moves) per character, expand later
- Frame data is at 60fps: 10 startup frames = ~0.167 seconds
- Test hitbox sizes in-editor by temporarily unhiding HitboxComponent
- Use Niagara for hit effects: `NS_HitSpark_Light`, `NS_HitSpark_Heavy`
- The AI difficulty slider goes from 0.0 (barely fights back) to 1.0 (reads your inputs)
