#include "CombatCharacter.h"
#include "CombatGame.h"
#include "CombatGameMode.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

ACombatCharacter::ACombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure capsule
	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	GetCapsuleComponent()->SetCapsuleRadius(34.0f);

	// Configure movement
	GetCharacterMovement()->GravityScale = 2.5f;
	GetCharacterMovement()->JumpZVelocity = JumpHeight;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkForwardSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Create hitbox (inactive by default)
	HitboxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
	HitboxComponent->SetupAttachment(GetMesh());
	HitboxComponent->SetBoxExtent(FVector(30.0f, 30.0f, 30.0f));
	HitboxComponent->SetRelativeLocation(FVector(60.0f, 0.0f, 60.0f));
	HitboxComponent->SetCollisionProfileName(TEXT("FighterHitbox"));
	HitboxComponent->SetGenerateOverlapEvents(true);
	HitboxComponent->SetActive(false);
	HitboxComponent->SetHiddenInGame(true);
	HitboxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACombatCharacter::OnHitboxOverlap);

	// Create hurtbox
	HurtboxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Hurtbox"));
	HurtboxComponent->SetupAttachment(GetCapsuleComponent());
	HurtboxComponent->SetBoxExtent(FVector(30.0f, 30.0f, 80.0f));
	HurtboxComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	HurtboxComponent->SetCollisionProfileName(TEXT("FighterHurtbox"));
	HurtboxComponent->SetGenerateOverlapEvents(true);

	// Defaults
	CurrentHealth = MaxHealth;
	CurrentState = EFighterState::Idle;
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// Auto-load input assets if not set via Blueprint defaults
	LoadInputActionsIfNeeded();

	// Populate a default move list if none was set in Blueprint.
	// Uses a single generic "Attack" for all buttons (one animation covers all).
	if (MoveList.Num() == 0)
	{
		FAttackData Attack;
		Attack.MoveName = FName("Attack");
		Attack.Damage = 12.f;
		Attack.StartupFrames = 10;
		Attack.ActiveFrames = 4;
		Attack.RecoveryFrames = 14;
		Attack.HitStunFrames = 20;
		Attack.BlockStunFrames = 9;
		Attack.KnockbackDistance = 100.f;

		// All attack types point to the same move data so any button works
		EAttackType AllTypes[] = {
			EAttackType::HighPunch, EAttackType::MidPunch, EAttackType::LowPunch,
			EAttackType::HighKick,  EAttackType::MidKick,  EAttackType::LowKick,
			EAttackType::Launcher,  EAttackType::Sweep
		};
		for (EAttackType Type : AllTypes)
		{
			FAttackData Entry = Attack;
			Entry.AttackType = Type;
			// Set hit height to match type for blocking logic
			if (Type == EAttackType::HighPunch || Type == EAttackType::HighKick)
				Entry.HitHeight = EHitHeight::High;
			else if (Type == EAttackType::LowPunch || Type == EAttackType::LowKick || Type == EAttackType::Sweep)
				Entry.HitHeight = EHitHeight::Low;
			else
				Entry.HitHeight = EHitHeight::Mid;
			// Launcher gets extra launch height
			if (Type == EAttackType::Launcher)
				Entry.LaunchHeight = 500.f;
			if (Type == EAttackType::Sweep)
				Entry.bKnocksDown = true;
			MoveList.Add(Entry);
		}

		UE_LOG(LogCombatGame, Warning, TEXT("CombatCharacter: Populated default attack (single animation) for all %d attack types"), MoveList.Num());
	}

	// Add input mapping context (fallback if controller didn't set it)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UE_LOG(LogCombatGame, Warning, TEXT("CombatCharacter::BeginPlay - Controller=%s, LocalPlayer=%s"),
			*PC->GetName(), PC->GetLocalPlayer() ? TEXT("Valid") : TEXT("NULL"));
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (FighterMappingContext && !Subsystem->HasMappingContext(FighterMappingContext))
			{
				Subsystem->AddMappingContext(FighterMappingContext, 0);
				UE_LOG(LogCombatGame, Warning, TEXT("CombatCharacter::BeginPlay - Added mapping context!"));
			}
		}
		else
		{
			UE_LOG(LogCombatGame, Warning, TEXT("CombatCharacter::BeginPlay - No EnhancedInput subsystem!"));
		}
	}
	else
	{
		UE_LOG(LogCombatGame, Warning, TEXT("CombatCharacter::BeginPlay - No PlayerController!"));
	}
}

void ACombatCharacter::LoadInputActionsIfNeeded()
{
	// Try loading from multiple possible paths — the user may have created
	// assets in /Game/Input/ or /Game/Input/Actions/
	auto TryLoad = [](UInputAction*& Action, const TCHAR* Name)
	{
		if (Action) return;

		// Try /Game/Input/Actions/ first (subfolder), then /Game/Input/ (root)
		FString Paths[] = {
			FString::Printf(TEXT("/Game/Input/Actions/%s.%s"), Name, Name),
			FString::Printf(TEXT("/Game/Input/%s.%s"), Name, Name),
		};

		for (const FString& Path : Paths)
		{
			Action = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr, *Path));
			if (Action)
			{
				UE_LOG(LogCombatGame, Log, TEXT("Loaded input action: %s from %s"), Name, *Path);
				return;
			}
		}
		UE_LOG(LogCombatGame, Warning, TEXT("Failed to load input action: %s"), Name);
	};

	if (!FighterMappingContext)
	{
		// Try multiple paths for the mapping context too
		const TCHAR* IMCPaths[] = {
			TEXT("/Game/Input/IMC_Fighter.IMC_Fighter"),
			TEXT("/Game/Input/Actions/IMC_Fighter.IMC_Fighter"),
		};
		for (const TCHAR* Path : IMCPaths)
		{
			FighterMappingContext = Cast<UInputMappingContext>(
				StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, Path));
			if (FighterMappingContext) break;
		}
	}

	TryLoad(MoveAction, TEXT("IA_Move"));
	TryLoad(JumpAction, TEXT("IA_Jump"));
	TryLoad(CrouchAction, TEXT("IA_Crouch"));
	TryLoad(BlockAction, TEXT("IA_Block"));
	TryLoad(LeftPunchAction, TEXT("IA_LeftPunch"));
	TryLoad(RightPunchAction, TEXT("IA_RightPunch"));
	TryLoad(LeftKickAction, TEXT("IA_LeftKick"));
	TryLoad(RightKickAction, TEXT("IA_RightKick"));
	TryLoad(SidestepAction, TEXT("IA_Sidestep"));
	TryLoad(SidestepLeftAction, TEXT("IA_SidestepLeft"));
	// Create SidestepLeftAction at runtime if asset doesn't exist
	if (!SidestepLeftAction)
	{
		SidestepLeftAction = NewObject<UInputAction>(this, TEXT("IA_SidestepLeft_Runtime"));
		SidestepLeftAction->ValueType = EInputActionValueType::Boolean;
	}

	UE_LOG(LogCombatGame, Warning, TEXT("CombatCharacter Input Load: Move=%s Jump=%s Crouch=%s Block=%s LP=%s RP=%s LK=%s RK=%s SS=%s IMC=%s"),
		MoveAction ? TEXT("OK") : TEXT("NULL"),
		JumpAction ? TEXT("OK") : TEXT("NULL"),
		CrouchAction ? TEXT("OK") : TEXT("NULL"),
		BlockAction ? TEXT("OK") : TEXT("NULL"),
		LeftPunchAction ? TEXT("OK") : TEXT("NULL"),
		RightPunchAction ? TEXT("OK") : TEXT("NULL"),
		LeftKickAction ? TEXT("OK") : TEXT("NULL"),
		RightKickAction ? TEXT("OK") : TEXT("NULL"),
		SidestepAction ? TEXT("OK") : TEXT("NULL"),
		FighterMappingContext ? TEXT("OK") : TEXT("NULL"));
}

void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInputEnabled && CurrentState != EFighterState::Launched) return;

	UpdateFacing();
	CleanInputBuffer();
	HandleStunTick(DeltaTime);
	HandleAttackFrames(DeltaTime);

	// Dash movement
	if (CurrentState == EFighterState::Dashing)
	{
		DashTimer -= DeltaTime;
		if (DashTimer <= 0.0f)
		{
			SetFighterState(EFighterState::Idle);
		}
	}

	// Juggle gravity
	if (CurrentState == EFighterState::Launched)
	{
		ApplyGravityToJuggle(DeltaTime);
	}
}

void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Must load input actions here, not just in BeginPlay, because
	// SetupPlayerInputComponent runs during Possess() which can happen
	// before BeginPlay when the actor is spawned inside another BeginPlay
	LoadInputActionsIfNeeded();

	// Build a runtime IMC with correct key→action mappings.
	// This guarantees correct bindings regardless of the asset's state.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Create a transient IMC at runtime
			UInputMappingContext* RuntimeIMC = NewObject<UInputMappingContext>(this, TEXT("RuntimeIMC_Fighter"));

			// Helper: map a key to an action with optional modifiers
			auto MapKey = [&](UInputAction* Action, FKey Key,
				bool bNegate = false, bool bSwizzle = false)
			{
				if (!Action) return;
				FEnhancedActionKeyMapping& Mapping = RuntimeIMC->MapKey(Action, Key);
				if (bSwizzle)
				{
					UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(RuntimeIMC);
					Swizzle->Order = EInputAxisSwizzle::YXZ;
					Mapping.Modifiers.Add(Swizzle);
				}
				if (bNegate)
				{
					UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(RuntimeIMC);
					Mapping.Modifiers.Add(Negate);
				}
			};

			// Ensure IA_Move is Axis2D
			if (MoveAction)
			{
				MoveAction->ValueType = EInputActionValueType::Axis2D;
			}

			// Arrow keys → IA_Move (Vector2D)
			MapKey(MoveAction, EKeys::Right);                        // X+
			MapKey(MoveAction, EKeys::Left, true, false);            // X- (Negate)
			MapKey(MoveAction, EKeys::Up, false, true);              // Y+ (Swizzle)
			MapKey(MoveAction, EKeys::Down, true, true);             // Y- (Swizzle+Negate)

			// Also support WASD
			MapKey(MoveAction, EKeys::D);                            // X+
			MapKey(MoveAction, EKeys::A, true, false);               // X-
			MapKey(MoveAction, EKeys::W, false, true);               // Y+
			MapKey(MoveAction, EKeys::S, true, true);                // Y-

			// Jump
			if (JumpAction)
			{
				JumpAction->ValueType = EInputActionValueType::Boolean;
				MapKey(JumpAction, EKeys::SpaceBar);
			}

			// Crouch
			if (CrouchAction)
			{
				CrouchAction->ValueType = EInputActionValueType::Boolean;
				MapKey(CrouchAction, EKeys::LeftControl);
			}

			// Block
			if (BlockAction)
			{
				BlockAction->ValueType = EInputActionValueType::Boolean;
				MapKey(BlockAction, EKeys::LeftShift);
			}

			// Attacks
			if (LeftPunchAction)
			{
				LeftPunchAction->ValueType = EInputActionValueType::Boolean;
				MapKey(LeftPunchAction, EKeys::U);
			}
			if (RightPunchAction)
			{
				RightPunchAction->ValueType = EInputActionValueType::Boolean;
				MapKey(RightPunchAction, EKeys::I);
			}
			if (LeftKickAction)
			{
				LeftKickAction->ValueType = EInputActionValueType::Boolean;
				MapKey(LeftKickAction, EKeys::J);
			}
			if (RightKickAction)
			{
				RightKickAction->ValueType = EInputActionValueType::Boolean;
				MapKey(RightKickAction, EKeys::K);
			}

			// Sidestep (Q = into background, E = back out)
			if (SidestepAction)
			{
				SidestepAction->ValueType = EInputActionValueType::Boolean;
				MapKey(SidestepAction, EKeys::Q);
			}
			if (SidestepLeftAction)
			{
				SidestepLeftAction->ValueType = EInputActionValueType::Boolean;
				MapKey(SidestepLeftAction, EKeys::E);
			}

			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(RuntimeIMC, 0);
			UE_LOG(LogCombatGame, Warning, TEXT("SetupPlayerInputComponent: Built runtime IMC with all key mappings"));
		}
		else
		{
			UE_LOG(LogCombatGame, Warning, TEXT("SetupPlayerInputComponent: No EnhancedInput subsystem! LocalPlayer=%s"),
				PC->GetLocalPlayer() ? TEXT("Valid") : TEXT("NULL"));
		}
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogCombatGame, Error, TEXT("SetupPlayerInputComponent: NOT an EnhancedInputComponent!"));
		return;
	}

	UE_LOG(LogCombatGame, Warning, TEXT("SetupPlayerInputComponent: MoveAction=%s, JumpAction=%s, CrouchAction=%s, SidestepAction=%s"),
		MoveAction ? *MoveAction->GetName() : TEXT("NULL"),
		JumpAction ? *JumpAction->GetName() : TEXT("NULL"),
		CrouchAction ? *CrouchAction->GetName() : TEXT("NULL"),
		SidestepAction ? *SidestepAction->GetName() : TEXT("NULL"));

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleMove);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACombatCharacter::HandleMoveCompleted);
	}
	if (JumpAction)
		EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleJump);
	if (CrouchAction)
	{
		EIC->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleCrouch);
		EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ACombatCharacter::HandleCrouchReleased);
	}
	if (BlockAction)
	{
		EIC->BindAction(BlockAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleBlock);
		EIC->BindAction(BlockAction, ETriggerEvent::Completed, this, &ACombatCharacter::HandleBlockReleased);
	}
	if (LeftPunchAction)
		EIC->BindAction(LeftPunchAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleLeftPunch);
	if (RightPunchAction)
		EIC->BindAction(RightPunchAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleRightPunch);
	if (LeftKickAction)
		EIC->BindAction(LeftKickAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleLeftKick);
	if (RightKickAction)
		EIC->BindAction(RightKickAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleRightKick);
	if (SidestepAction)
		EIC->BindAction(SidestepAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleSidestep);
	if (SidestepLeftAction)
		EIC->BindAction(SidestepLeftAction, ETriggerEvent::Triggered, this, &ACombatCharacter::HandleSidestepLeft);
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

void ACombatCharacter::SetFighterState(EFighterState NewState)
{
	if (CurrentState == NewState) return;

	EFighterState OldState = CurrentState;
	CurrentState = NewState;
	OnFighterStateChanged.Broadcast(NewState);

	// State enter logic
	switch (NewState)
	{
	case EFighterState::Idle:
		GetCharacterMovement()->MaxWalkSpeed = WalkForwardSpeed;
		break;
	case EFighterState::Crouching:
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		break;
	case EFighterState::Blocking:
		CurrentBlockType = EBlockType::Standing;
		break;
	case EFighterState::CrouchBlocking:
		CurrentBlockType = EBlockType::Crouching;
		break;
	case EFighterState::KnockedDown:
		if (KnockdownMontage) PlayAnimMontage(KnockdownMontage);
		GetWorldTimerManager().SetTimer(GetUpTimerHandle, [this]()
		{
			SetFighterState(EFighterState::GettingUp);
			if (GetUpMontage)
			{
				float Duration = PlayAnimMontage(GetUpMontage);
				FTimerHandle TempHandle;
				GetWorldTimerManager().SetTimer(TempHandle, [this]()
				{
					SetFighterState(EFighterState::Idle);
				}, Duration, false);
			}
			else
			{
				SetFighterState(EFighterState::Idle);
			}
		}, GetUpTime, false);
		break;
	case EFighterState::Victory:
		if (VictoryMontage) PlayAnimMontage(VictoryMontage);
		break;
	case EFighterState::Intro:
		if (IntroMontage) PlayAnimMontage(IntroMontage);
		break;
	default:
		break;
	}

	// State exit logic
	if (OldState == EFighterState::Blocking || OldState == EFighterState::CrouchBlocking)
	{
		CurrentBlockType = EBlockType::None;
	}
}

void ACombatCharacter::SetFighterInputEnabled(bool bEnabled)
{
	bInputEnabled = bEnabled;
	UE_LOG(LogCombatGame, Warning, TEXT("Fighter P%d: Input %s"), PlayerIndex, bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void ACombatCharacter::ResetForRound()
{
	CurrentHealth = MaxHealth;
	CurrentState = EFighterState::Idle;
	CurrentBlockType = EBlockType::None;
	bIsAttacking = false;
	bHitboxActive = false;
	JuggleHits = 0;
	ComboScaling = 1.0f;
	CurrentComboCount = 0;
	RemainingStunFrames = 0;
	RemainingAttackFrames = 0;
	FrameAccumulator = 0.0f;
	StunAccumulator = 0.0f;
	InputBuffer.Empty();
	bInCombo = false;
	ComboRouteIndex = -1;
	ComboAttackIndex = 0;
	DeactivateHitbox();
	StopAnimMontage(nullptr);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

// ============================================================================
// COMBAT
// ============================================================================

void ACombatCharacter::ExecuteAttack(const FAttackData& Attack)
{
	if (CurrentState == EFighterState::HitStun ||
		CurrentState == EFighterState::BlockStun ||
		CurrentState == EFighterState::KnockedDown ||
		CurrentState == EFighterState::GettingUp ||
		CurrentState == EFighterState::Dead)
	{
		return;
	}

	CurrentAttack = Attack;
	bIsAttacking = true;
	bHasHitThisAttack = false;
	CurrentAttackFrame = 0;
	RemainingAttackFrames = Attack.StartupFrames + Attack.ActiveFrames + Attack.RecoveryFrames;

	SetFighterState(EFighterState::Attacking);
	PlayAttackMontage(Attack);
}

void ACombatCharacter::HandleAttackFrames(float DeltaTime)
{
	if (!bIsAttacking) return;

	// Frame-based timing
	FrameAccumulator += DeltaTime;

	while (FrameAccumulator >= FrameTime)
	{
		FrameAccumulator -= FrameTime;
		CurrentAttackFrame++;
		RemainingAttackFrames--;

		// Activate hitbox during active frames
		int32 ActiveStart = CurrentAttack.StartupFrames;
		int32 ActiveEnd = ActiveStart + CurrentAttack.ActiveFrames;

		if (CurrentAttackFrame == ActiveStart)
		{
			// Position hitbox based on attack type
			FVector Offset;
			FVector Extent;
			switch (CurrentAttack.AttackType)
			{
			case EAttackType::HighPunch:
			case EAttackType::MidPunch:
				Offset = FVector(70.0f, 0.0f, 70.0f);
				Extent = FVector(35.0f, 25.0f, 20.0f);
				break;
			case EAttackType::LowPunch:
				Offset = FVector(60.0f, 0.0f, 30.0f);
				Extent = FVector(30.0f, 25.0f, 20.0f);
				break;
			case EAttackType::HighKick:
			case EAttackType::MidKick:
				Offset = FVector(80.0f, 0.0f, 65.0f);
				Extent = FVector(40.0f, 25.0f, 25.0f);
				break;
			case EAttackType::LowKick:
			case EAttackType::Sweep:
				Offset = FVector(70.0f, 0.0f, 15.0f);
				Extent = FVector(45.0f, 25.0f, 20.0f);
				break;
			case EAttackType::Launcher:
				Offset = FVector(65.0f, 0.0f, 50.0f);
				Extent = FVector(35.0f, 30.0f, 40.0f);
				break;
			default:
				Offset = FVector(65.0f, 0.0f, 55.0f);
				Extent = FVector(35.0f, 25.0f, 25.0f);
				break;
			}
			ActivateHitbox(Offset, Extent);
		}
		else if (CurrentAttackFrame == ActiveEnd)
		{
			DeactivateHitbox();
		}

		// Attack finished
		if (RemainingAttackFrames <= 0)
		{
			EndAttack();
			FrameAccumulator = 0.0f;
			break;
		}
	}
}

void ACombatCharacter::EndAttack()
{
	bIsAttacking = false;
	bHitboxActive = false;
	DeactivateHitbox();
	CurrentAttackFrame = 0;
	RemainingAttackFrames = 0;

	// Check if we're in a combo route and there's a window for next input
	if (bInCombo)
	{
		ComboWindowTimer = ComboWindowDuration;
	}
	else
	{
		SetFighterState(EFighterState::Idle);
	}
}

void ACombatCharacter::TakeFighterDamage(float Damage, const FAttackData& Attack, ACombatCharacter* Attacker)
{
	if (CurrentState == EFighterState::Dead) return;

	bool bBlocked = CanBlock(Attack);
	float ActualDamage;

	if (bBlocked)
	{
		ActualDamage = Attack.ChipDamage;
		RemainingStunFrames = Attack.BlockStunFrames;
		SetFighterState(bIsCrouching ? EFighterState::CrouchBlocking : EFighterState::BlockStun);
		PlayBlockReaction();

		// Play block sound
		if (USoundBase* BlockSnd = Attack.BlockSound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, BlockSnd, GetActorLocation());
		}
	}
	else
	{
		// Apply combo scaling
		ActualDamage = Damage * ComboScaling;
		RemainingStunFrames = Attack.HitStunFrames;

		// Play hit sound
		if (USoundBase* HitSnd = Attack.HitSound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSnd, GetActorLocation());
		}

		// Spawn hit effect
		if (UNiagaraSystem* HitFX = Attack.HitEffect.LoadSynchronous())
		{
			FVector HitLocation = GetActorLocation() + FVector(0, 0, 50);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HitFX, HitLocation);
		}

		// Launcher
		if (Attack.LaunchHeight > 0.0f && CurrentState != EFighterState::Launched)
		{
			LaunchOpponent(Attack.LaunchHeight);
		}
		else if (Attack.bKnocksDown)
		{
			SetFighterState(EFighterState::KnockedDown);
		}
		else
		{
			SetFighterState(EFighterState::HitStun);
		}

		PlayHitReaction(Attack);

		// Update combo tracking on the attacker
		if (Attacker)
		{
			Attacker->CurrentComboCount++;
			if (Attacker->CurrentComboCount == 1)
			{
				Attacker->StartCombo();
			}
			Attacker->ComboScaling = FMath::Max(0.2f, 1.0f - (Attacker->CurrentComboCount - 1) * 0.1f);
			Attacker->OnAttackLanded.Broadcast(Attack);
		}
	}

	// Apply damage
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	// Apply knockback
	if (!bBlocked || Attack.KnockbackDistance > 50.0f)
	{
		FVector KnockDir = (GetActorLocation() - (Attacker ? Attacker->GetActorLocation() : GetActorLocation())).GetSafeNormal();
		KnockDir.Z = 0.0f;
		ApplyKnockback(KnockDir, bBlocked ? Attack.KnockbackDistance * 0.3f : Attack.KnockbackDistance);
	}

	// Report to game mode
	if (ACombatGameMode* GM = Cast<ACombatGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->ReportDamage(this, ActualDamage);
	}

	// Check KO
	if (CurrentHealth <= 0.0f)
	{
		SetFighterState(EFighterState::Dead);
	}
}

bool ACombatCharacter::CanBlock(const FAttackData& IncomingAttack) const
{
	if (!bIsBlocking) return false;
	if (CurrentState == EFighterState::Attacking) return false;
	if (CurrentState == EFighterState::HitStun) return false;
	if (CurrentState == EFighterState::Launched) return false;
	if (CurrentState == EFighterState::KnockedDown) return false;

	switch (IncomingAttack.HitHeight)
	{
	case EHitHeight::High:
		// Standing block works, crouching ducks under it
		return CurrentBlockType == EBlockType::Standing || CurrentBlockType == EBlockType::Crouching;
	case EHitHeight::Mid:
		// Only standing block
		return CurrentBlockType == EBlockType::Standing;
	case EHitHeight::Low:
		// Only crouching block
		return CurrentBlockType == EBlockType::Crouching;
	case EHitHeight::Unblockable:
		return false;
	}
	return false;
}

void ACombatCharacter::ActivateHitbox(FVector RelativeOffset, FVector BoxExtent)
{
	if (!HitboxComponent) return;

	// Flip offset based on facing
	if (!bIsFacingRight)
	{
		RelativeOffset.X *= -1.0f;
	}

	HitboxComponent->SetRelativeLocation(RelativeOffset);
	HitboxComponent->SetBoxExtent(BoxExtent);
	HitboxComponent->SetActive(true);
	HitboxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	bHitboxActive = true;
}

void ACombatCharacter::DeactivateHitbox()
{
	if (!HitboxComponent) return;

	HitboxComponent->SetActive(false);
	HitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bHitboxActive = false;
}

void ACombatCharacter::OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bHitboxActive || bHasHitThisAttack) return;

	ACombatCharacter* OtherFighter = Cast<ACombatCharacter>(OtherActor);
	if (!OtherFighter || OtherFighter == this) return;

	// Check it's the hurtbox
	if (OtherComp != OtherFighter->HurtboxComponent) return;

	bHasHitThisAttack = true;
	OtherFighter->TakeFighterDamage(CurrentAttack.Damage, CurrentAttack, this);
}

void ACombatCharacter::ApplyKnockback(FVector Direction, float Force)
{
	LaunchCharacter(Direction * Force, true, false);
}

void ACombatCharacter::LaunchOpponent(float Height)
{
	SetFighterState(EFighterState::Launched);
	JuggleHits = 0;
	LaunchCharacter(FVector(0, 0, Height), false, true);
	if (LaunchMontage) PlayAnimMontage(LaunchMontage);
}

void ACombatCharacter::ApplyGravityToJuggle(float DeltaTime)
{
	// When landing, exit juggle state
	if (GetCharacterMovement()->IsMovingOnGround() && GetVelocity().Z <= 0.0f)
	{
		SetFighterState(EFighterState::KnockedDown);
	}
}

// ============================================================================
// MOVEMENT
// ============================================================================

void ACombatCharacter::StartBlock()
{
	bIsBlocking = true;
	if (bIsCrouching)
	{
		SetFighterState(EFighterState::CrouchBlocking);
	}
	else
	{
		SetFighterState(EFighterState::Blocking);
	}
}

void ACombatCharacter::StopBlock()
{
	bIsBlocking = false;
	CurrentBlockType = EBlockType::None;
	if (bIsCrouching)
	{
		SetFighterState(EFighterState::Crouching);
	}
	else
	{
		SetFighterState(EFighterState::Idle);
	}
}

void ACombatCharacter::PerformDash(bool bForward)
{
	if (bIsAttacking || CurrentState == EFighterState::HitStun ||
		CurrentState == EFighterState::BlockStun) return;

	if (bForward)
	{
		SetFighterState(EFighterState::Dashing);
		FVector DashDir = bIsFacingRight ? FVector(1, 0, 0) : FVector(-1, 0, 0);
		GetCharacterMovement()->Velocity = DashDir * DashSpeed;
		DashTimer = DashDuration;
	}
	else
	{
		SetFighterState(EFighterState::BackDashing);
		FVector DashDir = bIsFacingRight ? FVector(-1, 0, 0) : FVector(1, 0, 0);
		LaunchCharacter(DashDir * BackDashDistance + FVector(0, 0, 100), true, true);

		FTimerHandle BackDashTimer;
		GetWorldTimerManager().SetTimer(BackDashTimer, [this]()
		{
			if (CurrentState == EFighterState::BackDashing)
				SetFighterState(EFighterState::Idle);
		}, 0.4f, false);
	}
}

void ACombatCharacter::PerformSidestep(bool bRight)
{
	if (bIsAttacking || CurrentState == EFighterState::HitStun) return;

	SetFighterState(EFighterState::Sidestepping);
	FVector StepDir = bRight ? FVector(0, 1, 0) : FVector(0, -1, 0);
	LaunchCharacter(StepDir * SidestepDistance, true, false);

	FTimerHandle StepTimer;
	GetWorldTimerManager().SetTimer(StepTimer, [this]()
	{
		if (CurrentState == EFighterState::Sidestepping)
			SetFighterState(EFighterState::Idle);
	}, 0.3f, false);
}

void ACombatCharacter::UpdateFacing()
{
	if (!OpponentFighter) return;

	FVector ToOpponent = OpponentFighter->GetActorLocation() - GetActorLocation();
	bIsFacingRight = ToOpponent.X > 0.0f;

	// Only auto-face when not attacking
	if (!bIsAttacking && CurrentState != EFighterState::KnockedDown)
	{
		FRotator NewRot = FRotator(0.0f, bIsFacingRight ? 0.0f : 180.0f, 0.0f);
		SetActorRotation(NewRot);
	}
}

// ============================================================================
// INPUT
// ============================================================================

void ACombatCharacter::HandleMove(const FInputActionValue& Value)
{
	FVector2D DebugInput = Value.Get<FVector2D>();
	UE_LOG(LogCombatGame, Warning, TEXT("HandleMove called! Input=(%f,%f) bInputEnabled=%d State=%d"),
		DebugInput.X, DebugInput.Y, bInputEnabled, (int32)CurrentState);
	if (!bInputEnabled) return;
	if (CurrentState == EFighterState::HitStun || CurrentState == EFighterState::BlockStun ||
		CurrentState == EFighterState::KnockedDown || CurrentState == EFighterState::Launched) return;

	FVector2D MoveInput = Value.Get<FVector2D>();
	CurrentStickDirection = GetDirectionFromStick(MoveInput);

	// Tekken-style: Up on stick = Jump, Down on stick = Crouch
	if (MoveInput.Y > 0.5f)
	{
		// Up pressed → jump (same as dedicated jump button)
		if (!bIsAttacking && CurrentState != EFighterState::HitStun &&
			CurrentState != EFighterState::BlockStun && CurrentState != EFighterState::KnockedDown)
		{
			if (GetCharacterMovement()->IsMovingOnGround())
			{
				SetFighterState(EFighterState::Jumping);
				Jump();
			}
		}
	}
	else if (MoveInput.Y < -0.5f)
	{
		// Down pressed → crouch
		if (!bIsCrouching)
		{
			bIsCrouching = true;
			if (bIsBlocking)
				SetFighterState(EFighterState::CrouchBlocking);
			else if (!bIsAttacking)
				SetFighterState(EFighterState::Crouching);
		}
	}
	else
	{
		// Y near zero → release crouch if it was from stick
		if (bIsCrouching && bCrouchFromStick)
		{
			bIsCrouching = false;
			if (CurrentState == EFighterState::Crouching || CurrentState == EFighterState::CrouchBlocking)
			{
				if (bIsBlocking)
					SetFighterState(EFighterState::Blocking);
				else
					SetFighterState(EFighterState::Idle);
			}
		}
	}

	// Track whether crouch came from stick (Down arrow) so we don't
	// accidentally release crouch triggered by the dedicated Crouch key
	bCrouchFromStick = (MoveInput.Y < -0.5f);

	if (!bIsAttacking)
	{
		// Horizontal movement
		float MoveSpeed = 0.0f;
		FVector MoveDir = FVector::ZeroVector;

		bool bMovingForward = (bIsFacingRight && MoveInput.X > 0) || (!bIsFacingRight && MoveInput.X < 0);
		bool bMovingBackward = (bIsFacingRight && MoveInput.X < 0) || (!bIsFacingRight && MoveInput.X > 0);

		if (bMovingForward)
		{
			MoveSpeed = WalkForwardSpeed;
			MoveDir = FVector(bIsFacingRight ? 1 : -1, 0, 0);

			// Double tap dash detection
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if (CurrentTime - LastForwardTapTime < DoubleTapThreshold)
			{
				PerformDash(true);
				return;
			}
			LastForwardTapTime = CurrentTime;
		}
		else if (bMovingBackward)
		{
			MoveSpeed = WalkBackwardSpeed;
			MoveDir = FVector(bIsFacingRight ? -1 : 1, 0, 0);

			// Double tap backdash
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if (CurrentTime - LastBackTapTime < DoubleTapThreshold)
			{
				PerformDash(false);
				return;
			}
			LastBackTapTime = CurrentTime;
		}

		if (CurrentState != EFighterState::Dashing && CurrentState != EFighterState::BackDashing)
		{
			GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
			AddMovementInput(MoveDir, 1.0f);
			if (!bIsCrouching && CurrentState != EFighterState::Jumping)
			{
				SetFighterState(EFighterState::Walking);
			}
		}
	}
}

void ACombatCharacter::HandleMoveCompleted(const FInputActionValue& Value)
{
	CurrentStickDirection = EInputDirection::Neutral;

	// Release stick-crouch when all movement keys released
	if (bCrouchFromStick && bIsCrouching)
	{
		bIsCrouching = false;
		bCrouchFromStick = false;
		if (CurrentState == EFighterState::Crouching || CurrentState == EFighterState::CrouchBlocking)
		{
			if (bIsBlocking)
				SetFighterState(EFighterState::Blocking);
			else
				SetFighterState(EFighterState::Idle);
		}
	}

	if (CurrentState == EFighterState::Walking)
	{
		SetFighterState(EFighterState::Idle);
	}
}

void ACombatCharacter::HandleJump(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	if (bIsAttacking || CurrentState == EFighterState::HitStun ||
		CurrentState == EFighterState::BlockStun || CurrentState == EFighterState::KnockedDown) return;

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		SetFighterState(EFighterState::Jumping);
		Jump();
	}
}

void ACombatCharacter::HandleCrouch(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	bIsCrouching = true;
	if (bIsBlocking)
	{
		SetFighterState(EFighterState::CrouchBlocking);
	}
	else if (!bIsAttacking)
	{
		SetFighterState(EFighterState::Crouching);
	}
}

void ACombatCharacter::HandleCrouchReleased(const FInputActionValue& Value)
{
	bIsCrouching = false;
	if (CurrentState == EFighterState::Crouching || CurrentState == EFighterState::CrouchBlocking)
	{
		if (bIsBlocking)
			SetFighterState(EFighterState::Blocking);
		else
			SetFighterState(EFighterState::Idle);
	}
}

void ACombatCharacter::HandleBlock(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	StartBlock();
}

void ACombatCharacter::HandleBlockReleased(const FInputActionValue& Value)
{
	StopBlock();
}

void ACombatCharacter::HandleLeftPunch(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	ProcessAttackButton(EAttackButton::LeftPunch);
}

void ACombatCharacter::HandleRightPunch(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	ProcessAttackButton(EAttackButton::RightPunch);
}

void ACombatCharacter::HandleLeftKick(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	ProcessAttackButton(EAttackButton::LeftKick);
}

void ACombatCharacter::HandleRightKick(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	ProcessAttackButton(EAttackButton::RightKick);
}

void ACombatCharacter::HandleSidestep(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	PerformSidestep(true); // Q = sidestep right (into background)
}

void ACombatCharacter::HandleSidestepLeft(const FInputActionValue& Value)
{
	if (!bInputEnabled) return;
	PerformSidestep(false); // E = sidestep left (back to foreground)
}

void ACombatCharacter::ProcessAttackButton(EAttackButton Button)
{
	EInputDirection Dir = CurrentStickDirection;
	BufferInput(Dir, Button);

	// Check combo routes first
	FComboRoute* Combo = CheckComboRoutes();
	if (Combo && !bIsAttacking)
	{
		bInCombo = true;
		ComboAttackIndex = 0;
		if (Combo->Attacks.IsValidIndex(0))
		{
			ExecuteAttack(Combo->Attacks[0]);
		}
		return;
	}

	// Continue existing combo
	if (bInCombo && !bIsAttacking && ComboRouteIndex >= 0)
	{
		if (ComboRoutes.IsValidIndex(ComboRouteIndex))
		{
			FComboRoute& ActiveCombo = ComboRoutes[ComboRouteIndex];
			ComboAttackIndex++;
			if (ActiveCombo.Attacks.IsValidIndex(ComboAttackIndex))
			{
				ExecuteAttack(ActiveCombo.Attacks[ComboAttackIndex]);
				return;
			}
			else
			{
				EndCombo();
			}
		}
	}

	// Single move lookup
	if (!bIsAttacking)
	{
		FAttackData* Move = FindMoveForInput(Dir, Button);
		if (Move)
		{
			ExecuteAttack(*Move);
		}
	}
}

void ACombatCharacter::BufferInput(EInputDirection Dir, EAttackButton Button)
{
	FInputBufferEntry Entry;
	Entry.Direction = Dir;
	Entry.Button = Button;
	Entry.Timestamp = GetWorld()->GetTimeSeconds();
	InputBuffer.Add(Entry);

	if (InputBuffer.Num() > MaxInputBufferSize)
	{
		InputBuffer.RemoveAt(0);
	}
}

void ACombatCharacter::CleanInputBuffer()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	InputBuffer.RemoveAll([this, CurrentTime](const FInputBufferEntry& Entry)
	{
		return (CurrentTime - Entry.Timestamp) > InputBufferLifetime;
	});
}

FAttackData* ACombatCharacter::FindMoveForInput(EInputDirection Dir, EAttackButton Button)
{
	// Map direction + button to attack type for generic lookup
	EAttackType TargetType;
	bool bIsCrouchAttack = (Dir == EInputDirection::Down || Dir == EInputDirection::DownForward || Dir == EInputDirection::DownBack);

	switch (Button)
	{
	case EAttackButton::LeftPunch:
		TargetType = bIsCrouchAttack ? EAttackType::LowPunch : EAttackType::HighPunch;
		break;
	case EAttackButton::RightPunch:
		TargetType = bIsCrouchAttack ? EAttackType::LowPunch : EAttackType::MidPunch;
		break;
	case EAttackButton::LeftKick:
		TargetType = bIsCrouchAttack ? EAttackType::LowKick : EAttackType::HighKick;
		break;
	case EAttackButton::RightKick:
		TargetType = bIsCrouchAttack ? EAttackType::LowKick : EAttackType::MidKick;
		break;
	default:
		return nullptr;
	}

	// Special direction-based moves (e.g., Forward+RP = launcher)
	if (Dir == EInputDirection::DownForward && Button == EAttackButton::RightPunch)
	{
		TargetType = EAttackType::Launcher;
	}

	// Search move list
	for (FAttackData& Move : MoveList)
	{
		if (Move.AttackType == TargetType)
		{
			return &Move;
		}
	}

	return nullptr;
}

FComboRoute* ACombatCharacter::CheckComboRoutes()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (int32 i = 0; i < ComboRoutes.Num(); i++)
	{
		FComboRoute& Route = ComboRoutes[i];
		if (Route.InputSequence.Num() == 0) continue;

		// Check if recent input buffer matches the combo input sequence
		int32 SeqIdx = Route.InputSequence.Num() - 1;
		int32 BufIdx = InputBuffer.Num() - 1;
		bool bMatch = true;

		while (SeqIdx >= 0 && BufIdx >= 0)
		{
			const FComboInput& Required = Route.InputSequence[SeqIdx];
			const FInputBufferEntry& Buffered = InputBuffer[BufIdx];

			if (Buffered.Direction != Required.Direction || Buffered.Button != Required.Button)
			{
				bMatch = false;
				break;
			}

			// Check timing
			if (SeqIdx < Route.InputSequence.Num() - 1)
			{
				const FInputBufferEntry& NextInput = InputBuffer[BufIdx + 1];
				if (NextInput.Timestamp - Buffered.Timestamp > Required.MaxDelay)
				{
					bMatch = false;
					break;
				}
			}

			SeqIdx--;
			BufIdx--;
		}

		if (bMatch && SeqIdx < 0)
		{
			ComboRouteIndex = i;
			return &Route;
		}
	}

	return nullptr;
}

// ============================================================================
// STUN
// ============================================================================

void ACombatCharacter::HandleStunTick(float DeltaTime)
{
	if (RemainingStunFrames <= 0) return;

	StunAccumulator += DeltaTime;

	while (StunAccumulator >= FrameTime && RemainingStunFrames > 0)
	{
		StunAccumulator -= FrameTime;
		RemainingStunFrames--;
	}

	if (RemainingStunFrames <= 0)
	{
		if (CurrentState == EFighterState::HitStun || CurrentState == EFighterState::BlockStun)
		{
			EndCombo();
			SetFighterState(EFighterState::Idle);
		}
	}
}

// ============================================================================
// COMBO
// ============================================================================

void ACombatCharacter::StartCombo()
{
	OnComboStarted.Broadcast();
}

void ACombatCharacter::EndCombo()
{
	if (CurrentComboCount > 0)
	{
		OnComboEnded.Broadcast(CurrentComboCount);
	}
	CurrentComboCount = 0;
	ComboScaling = 1.0f;
	bInCombo = false;
	ComboRouteIndex = -1;
	ComboAttackIndex = 0;
}

// ============================================================================
// INPUT DIRECTION
// ============================================================================

EInputDirection ACombatCharacter::GetDirectionFromStick(FVector2D StickInput) const
{
	if (StickInput.IsNearlyZero(0.2f))
		return EInputDirection::Neutral;

	// Flip X based on facing direction so "forward" is always toward opponent
	float X = bIsFacingRight ? StickInput.X : -StickInput.X;
	float Y = StickInput.Y;

	if (Y > 0.5f)
	{
		if (X > 0.5f) return EInputDirection::UpForward;
		if (X < -0.5f) return EInputDirection::UpBack;
		return EInputDirection::Up;
	}
	if (Y < -0.5f)
	{
		if (X > 0.5f) return EInputDirection::DownForward;
		if (X < -0.5f) return EInputDirection::DownBack;
		return EInputDirection::Down;
	}
	if (X > 0.5f) return EInputDirection::Forward;
	if (X < -0.5f) return EInputDirection::Back;

	return EInputDirection::Neutral;
}

// ============================================================================
// ANIMATION
// ============================================================================

float ACombatCharacter::PlayAttackMontage(const FAttackData& Attack)
{
	if (UAnimMontage* Montage = Attack.AttackMontage.LoadSynchronous())
	{
		return PlayAnimMontage(Montage);
	}

	// No montage set — the state machine handles the Attacking animation
	// via the bIsAttacking transition in the ABP.

	if (USoundBase* WhiffSnd = Attack.WhiffSound.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(this, WhiffSnd, GetActorLocation());
	}

	return 0.0f;
}

void ACombatCharacter::PlayHitReaction(const FAttackData& Attack)
{
	UAnimMontage* ReactionMontage = nullptr;

	switch (Attack.HitHeight)
	{
	case EHitHeight::High:
		ReactionMontage = HitReactionHigh;
		break;
	case EHitHeight::Mid:
		ReactionMontage = HitReactionMid;
		break;
	case EHitHeight::Low:
		ReactionMontage = HitReactionLow;
		break;
	default:
		ReactionMontage = HitReactionMid;
		break;
	}

	if (ReactionMontage)
	{
		PlayAnimMontage(ReactionMontage);
	}
}

void ACombatCharacter::PlayBlockReaction()
{
	if (BlockReactionMontage)
	{
		PlayAnimMontage(BlockReactionMontage);
	}
}

