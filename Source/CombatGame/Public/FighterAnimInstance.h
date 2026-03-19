// FighterAnimInstance.h - Base animation blueprint class for fighters
// Create Animation Blueprints per character: ABP_[CharacterName] (e.g., ABP_Fighter01)
// Parent this to FighterAnimInstance
//
// ANIMATION NAMING CONVENTION:
// ============================
// Idle:        A_[CharacterName]_Idle           (e.g., A_Fighter01_Idle)
// Walk Fwd:    A_[CharacterName]_WalkForward
// Walk Back:   A_[CharacterName]_WalkBackward
// Crouch:      A_[CharacterName]_Crouch
// Jump:        A_[CharacterName]_Jump
// Block Stand: A_[CharacterName]_BlockStand
// Block Crouch:A_[CharacterName]_BlockCrouch
//
// ATTACK MONTAGES:
// AM_[CharacterName]_[MoveName]        (e.g., AM_Fighter01_LeftJab)
// AM_[CharacterName]_[MoveName]        (e.g., AM_Fighter01_RightUppercut)
//
// HIT REACTIONS:
// AM_[CharacterName]_HitHigh
// AM_[CharacterName]_HitMid
// AM_[CharacterName]_HitLow
// AM_[CharacterName]_BlockReaction
// AM_[CharacterName]_Knockdown
// AM_[CharacterName]_GetUp
// AM_[CharacterName]_Launched
//
// SPECIAL:
// AM_[CharacterName]_Victory
// AM_[CharacterName]_Intro
// AM_[CharacterName]_Defeat
//
// BLEND SPACE (optional):
// BS_[CharacterName]_Locomotion       (1D blend: -1=back, 0=idle, 1=forward)

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CombatTypes.h"
#include "FighterAnimInstance.generated.h"

UCLASS()
class COMBATGAME_API UFighterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	// State variables for the Anim Blueprint state machine
	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	EFighterState FighterState;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsJumping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsCrouching = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsBlocking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsInHitStun = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsKnockedDown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsLaunched = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsDead = false;

	// Movement speed for blend space (-1 = backward, 0 = idle, 1 = forward)
	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	float MovementSpeed = 0.0f;

	// Vertical velocity for jump blending
	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	float VerticalVelocity = 0.0f;

	// Is on ground?
	UPROPERTY(BlueprintReadOnly, Category = "Fighter")
	bool bIsOnGround = true;

private:
	UPROPERTY()
	class ACombatCharacter* OwnerFighter;
};
