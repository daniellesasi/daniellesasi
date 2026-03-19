#include "FighterAnimInstance.h"
#include "CombatCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UFighterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerFighter = Cast<ACombatCharacter>(TryGetPawnOwner());
}

void UFighterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!OwnerFighter)
	{
		OwnerFighter = Cast<ACombatCharacter>(TryGetPawnOwner());
		if (!OwnerFighter) return;
	}

	FighterState = OwnerFighter->CurrentState;

	// Movement
	FVector Velocity = OwnerFighter->GetVelocity();
	bIsMoving = Velocity.Size2D() > 10.0f;
	bIsOnGround = OwnerFighter->GetCharacterMovement()->IsMovingOnGround();
	VerticalVelocity = Velocity.Z;

	// Calculate movement direction relative to facing
	if (bIsMoving)
	{
		FVector Forward = OwnerFighter->GetActorForwardVector();
		float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal2D(), Forward);
		MovementSpeed = DotProduct; // -1 = walking back, 1 = walking forward
	}
	else
	{
		MovementSpeed = 0.0f;
	}

	// State booleans
	bIsJumping = FighterState == EFighterState::Jumping;
	bIsCrouching = FighterState == EFighterState::Crouching || FighterState == EFighterState::CrouchBlocking;
	bIsAttacking = FighterState == EFighterState::Attacking;
	bIsBlocking = FighterState == EFighterState::Blocking || FighterState == EFighterState::CrouchBlocking;
	bIsInHitStun = FighterState == EFighterState::HitStun || FighterState == EFighterState::BlockStun;
	bIsKnockedDown = FighterState == EFighterState::KnockedDown || FighterState == EFighterState::GettingUp;
	bIsLaunched = FighterState == EFighterState::Launched;
	bIsDead = FighterState == EFighterState::Dead;
}
