// CombatCharacter.h - Base fighter class with full Tekken-style combat
// To create a character:
//   1. Create a Blueprint child: BP_[CharacterName] (e.g., BP_Fighter01)
//   2. Set the Skeletal Mesh in the Mesh component (e.g., using TestFighter_Skelaton)
//   3. Set the Animation Blueprint: ABP_[CharacterName] (parent: FighterAnimInstance)
//   4. Fill in the MoveList and ComboRoutes arrays
//   5. Set hit/hurt box sizes as needed
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatTypes.h"
#include "InputActionValue.h"
#include "CombatCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UBoxComponent;
class UAnimMontage;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFighterStateChanged, EFighterState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackLanded, FAttackData, Attack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboEnded, int32, HitCount);

UCLASS()
class COMBATGAME_API ACombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACombatCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========================================================================
	// IDENTITY
	// ========================================================================

	UPROPERTY(BlueprintReadWrite, Category = "Fighter")
	int32 PlayerIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Fighter")
	ACombatCharacter* OpponentFighter = nullptr;

	// ========================================================================
	// HEALTH
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 170.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	// ========================================================================
	// STATE
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EFighterState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EBlockType CurrentBlockType;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsOnGround = true;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsFacingRight = true;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bInputEnabled = false;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFighterStateChanged OnFighterStateChanged;

	// ========================================================================
	// COMBAT STATS
	// ========================================================================

	// Move list for this fighter (set in Blueprint)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FAttackData> MoveList;

	// Combo routes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FComboRoute> ComboRoutes;

	// Currently executing attack
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FAttackData CurrentAttack;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	// Juggle state
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 JuggleHits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxJuggleHits = 5;

	// Damage scaling in combos
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float ComboScaling = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 CurrentComboCount = 0;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttackLanded OnAttackLanded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnComboStarted OnComboStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnComboEnded OnComboEnded;

	// ========================================================================
	// MOVEMENT TUNING
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkForwardSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkBackwardSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BackDashDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SidestepDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpHeight = 400.0f;

	// ========================================================================
	// HITBOX / HURTBOX
	// ========================================================================

	// Active hitbox (spawned during attacks)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* HitboxComponent;

	// Body hurtbox
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* HurtboxComponent;

	// ========================================================================
	// INPUT BUFFER
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	TArray<FInputBufferEntry> InputBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	int32 MaxInputBufferSize = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float InputBufferLifetime = 0.5f;

	// ========================================================================
	// STUN TRACKING
	// ========================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 RemainingStunFrames = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 RemainingAttackFrames = 0;

	// ========================================================================
	// FUNCTIONS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Fighter")
	void SetFighterState(EFighterState NewState);

	UFUNCTION(BlueprintCallable, Category = "Fighter")
	void SetFighterInputEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Fighter")
	void ResetForRound();

	// Combat
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecuteAttack(const FAttackData& Attack);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeFighterDamage(float Damage, const FAttackData& Attack, ACombatCharacter* Attacker);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanBlock(const FAttackData& IncomingAttack) const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ActivateHitbox(FVector RelativeOffset, FVector BoxExtent);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DeactivateHitbox();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyKnockback(FVector Direction, float Force);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LaunchOpponent(float Height);

	// Movement
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartBlock();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopBlock();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void PerformDash(bool bForward);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void PerformSidestep(bool bRight);

	// Facing
	UFUNCTION(BlueprintCallable, Category = "Fighter")
	void UpdateFacing();

	// Get direction relative to opponent (for input parsing)
	UFUNCTION(BlueprintCallable, Category = "Input")
	EInputDirection GetDirectionFromStick(FVector2D StickInput) const;

	// Animation helpers
	UFUNCTION(BlueprintCallable, Category = "Animation")
	float PlayAttackMontage(const FAttackData& Attack);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayHitReaction(const FAttackData& Attack);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayBlockReaction();

protected:
	// Enhanced Input
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* FighterMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* BlockAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LeftPunchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RightPunchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LeftKickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* RightKickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SidestepAction;

	// Input handlers
	void HandleMove(const FInputActionValue& Value);
	void HandleMoveCompleted(const FInputActionValue& Value);
	void HandleJump(const FInputActionValue& Value);
	void HandleCrouch(const FInputActionValue& Value);
	void HandleCrouchReleased(const FInputActionValue& Value);
	void HandleBlock(const FInputActionValue& Value);
	void HandleBlockReleased(const FInputActionValue& Value);
	void HandleLeftPunch(const FInputActionValue& Value);
	void HandleRightPunch(const FInputActionValue& Value);
	void HandleLeftKick(const FInputActionValue& Value);
	void HandleRightKick(const FInputActionValue& Value);
	void HandleSidestep(const FInputActionValue& Value);

	// Internal helpers
	void ProcessAttackButton(EAttackButton Button);
	void BufferInput(EInputDirection Dir, EAttackButton Button);
	void CleanInputBuffer();
	FAttackData* FindMoveForInput(EInputDirection Dir, EAttackButton Button);
	FComboRoute* CheckComboRoutes();
	void HandleStunTick(float DeltaTime);
	void HandleAttackFrames(float DeltaTime);
	void EndAttack();
	void StartCombo();
	void EndCombo();
	void ApplyGravityToJuggle(float DeltaTime);

	UFUNCTION()
	void OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// State tracking
	EInputDirection CurrentStickDirection = EInputDirection::Neutral;
	bool bIsCrouching = false;
	bool bIsBlocking = false;
	bool bDashForwardPending = false;
	float DashTimer = 0.0f;
	float LastForwardTapTime = 0.0f;
	float LastBackTapTime = 0.0f;
	float DoubleTapThreshold = 0.25f;

	// Frame counting (60fps fixed)
	static constexpr float FrameTime = 1.0f / 60.0f;
	float FrameAccumulator = 0.0f;
	float StunAccumulator = 0.0f;
	int32 CurrentAttackFrame = 0;
	bool bHitboxActive = false;
	bool bHasHitThisAttack = false;

	// Combo
	bool bInCombo = false;
	int32 ComboRouteIndex = -1;
	int32 ComboAttackIndex = 0;
	float ComboWindowTimer = 0.0f;
	float ComboWindowDuration = 0.4f;

	// Knockdown
	FTimerHandle GetUpTimerHandle;
	float GetUpTime = 1.5f;

	// Generic hit reaction montages (override in BP)
	// Name: AM_[CharacterName]_HitHigh, AM_[CharacterName]_HitMid, etc.
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitReactionHigh;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitReactionMid;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitReactionLow;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* BlockReactionMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* GetUpMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LaunchMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* VictoryMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* IntroMontage;
};
