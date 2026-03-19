// CombatTypes.h - Core enums and structs for the combat system
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CombatTypes.generated.h"

// ============================================================================
// ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EFighterState : uint8
{
	Idle,
	Walking,
	Crouching,
	Jumping,
	Attacking,
	Blocking,
	CrouchBlocking,
	HitStun,
	BlockStun,
	KnockedDown,
	GettingUp,
	Launched,       // Juggle state (airborne from hit)
	Dashing,
	BackDashing,
	Sidestepping,   // Tekken-style sidestep
	Dead,
	Intro,
	Victory
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	HighPunch,
	MidPunch,
	LowPunch,
	HighKick,
	MidKick,
	LowKick,
	Throw,
	Special,
	Launcher,       // Launches opponent for juggle
	Sweep,          // Knocks down
	Unblockable
};

UENUM(BlueprintType)
enum class EBlockType : uint8
{
	None,
	Standing,       // Blocks high and mid
	Crouching       // Blocks low, ducks high
};

UENUM(BlueprintType)
enum class EHitHeight : uint8
{
	High,           // Can be ducked under
	Mid,            // Must stand-block
	Low,            // Must crouch-block
	Unblockable
};

UENUM(BlueprintType)
enum class ERoundResult : uint8
{
	None,
	Player1Win,
	Player2Win,
	Draw,
	TimeOut
};

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	WaitingToStart,
	RoundIntro,
	Fighting,
	RoundEnd,
	MatchEnd,
	CharacterSelect,
	MainMenu
};

UENUM(BlueprintType)
enum class EInputDirection : uint8
{
	Neutral,
	Forward,
	Back,
	Up,
	Down,
	UpForward,
	UpBack,
	DownForward,
	DownBack
};

UENUM(BlueprintType)
enum class EAttackButton : uint8
{
	None,
	LeftPunch,      // Tekken 1 (LP)
	RightPunch,     // Tekken 2 (RP)
	LeftKick,       // Tekken 3 (LK)
	RightKick       // Tekken 4 (RK)
};

// ============================================================================
// STRUCTS
// ============================================================================

USTRUCT(BlueprintType)
struct FAttackData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name for the move
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName MoveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	EAttackType AttackType = EAttackType::HighPunch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	EHitHeight HitHeight = EHitHeight::Mid;

	// Damage dealt on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float Damage = 10.0f;

	// Chip damage when blocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float ChipDamage = 1.0f;

	// Frames before the hitbox becomes active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 StartupFrames = 10;

	// Frames the hitbox is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 ActiveFrames = 3;

	// Frames of recovery after active frames end
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 RecoveryFrames = 15;

	// Hitstun inflicted on the opponent (in frames)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 HitStunFrames = 20;

	// Blockstun inflicted on the opponent (in frames)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 BlockStunFrames = 10;

	// Frame advantage on hit (positive = you recover first)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 FrameAdvantageOnHit = 5;

	// Frame advantage on block
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frames")
	int32 FrameAdvantageOnBlock = -5;

	// Knockback distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float KnockbackDistance = 100.0f;

	// Launch height (for launchers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float LaunchHeight = 0.0f;

	// Does this move knock down?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bKnocksDown = false;

	// Can this move be used in a juggle?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bJuggleMove = false;

	// Is this a counter-hit launcher?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bCounterHitLauncher = false;

	// Animation montage to play
	// Name your montage: AM_[CharacterName]_[MoveName]
	// Example: AM_Fighter01_LeftJab
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<class UAnimMontage> AttackMontage;

	// Hit effect (Niagara system)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSoftObjectPtr<class UNiagaraSystem> HitEffect;

	// Sound to play on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<class USoundBase> HitSound;

	// Sound to play on block
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<class USoundBase> BlockSound;

	// Sound to play on whiff
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<class USoundBase> WhiffSound;
};

USTRUCT(BlueprintType)
struct FComboInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInputDirection Direction = EInputDirection::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAttackButton Button = EAttackButton::None;

	// Max time allowed between this input and the next (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDelay = 0.3f;
};

USTRUCT(BlueprintType)
struct FComboRoute
{
	GENERATED_BODY()

	// Display name of the combo/string
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FName ComboName;

	// Sequence of inputs to trigger this combo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<FComboInput> InputSequence;

	// Attacks to execute in order when the combo fires
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<FAttackData> Attacks;
};

USTRUCT(BlueprintType)
struct FInputBufferEntry
{
	GENERATED_BODY()

	UPROPERTY()
	EInputDirection Direction = EInputDirection::Neutral;

	UPROPERTY()
	EAttackButton Button = EAttackButton::None;

	UPROPERTY()
	float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct FCharacterData : public FTableRowBase
{
	GENERATED_BODY()

	// Character display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FText DisplayName;

	// Character select portrait
	// Name: T_[CharacterName]_Portrait (e.g., T_Fighter01_Portrait)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	TSoftObjectPtr<class UTexture2D> Portrait;

	// Full body render for character select
	// Name: T_[CharacterName]_FullBody
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	TSoftObjectPtr<class UTexture2D> FullBodyRender;

	// Blueprint class of the fighter
	// Name: BP_[CharacterName] (e.g., BP_Fighter01)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TSoftClassPtr<class ACombatCharacter> FighterClass;

	// Base health for this character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 170.0f;

	// Walk speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float WalkSpeed = 300.0f;

	// Backdash distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BackDashDistance = 250.0f;

	// This character's move list (attacks mapped by name)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FAttackData> MoveList;

	// This character's combo routes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FComboRoute> ComboRoutes;
};

USTRUCT(BlueprintType)
struct FRoundData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 RoundNumber = 1;

	UPROPERTY(BlueprintReadOnly)
	ERoundResult Result = ERoundResult::None;

	UPROPERTY(BlueprintReadOnly)
	float Player1HealthRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float Player2HealthRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RoundTime = 0.0f;
};
