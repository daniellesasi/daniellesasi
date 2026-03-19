#include "FightingCameraActor.h"
#include "CombatCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

AFightingCameraActor::AFightingCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RootComponent = CameraComponent;
	CameraComponent->FieldOfView = CameraFOV;
}

void AFightingCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Fighter1 || !Fighter2) return;

	FVector P1Loc = Fighter1->GetActorLocation();
	FVector P2Loc = Fighter2->GetActorLocation();

	// Midpoint between fighters
	FVector Midpoint = (P1Loc + P2Loc) * 0.5f;
	Midpoint.Z = CameraHeight;

	// Distance between fighters determines camera zoom
	float FighterDistance = FVector::Dist2D(P1Loc, P2Loc);
	float CamDistance = BaseDistance + (FighterDistance * DistanceMultiplier);
	CamDistance = FMath::Clamp(CamDistance, MinDistance, MaxDistance);

	// Camera position: offset along negative Y (perpendicular to the X fight axis)
	// This gives the classic Tekken side-view perspective
	DesiredLocation = Midpoint + FVector(0.0f, -CamDistance, 0.0f);

	// Look at the midpoint
	DesiredRotation = (Midpoint - DesiredLocation).Rotation();

	// Smooth interpolation
	FVector NewLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaTime, CameraLerpSpeed);
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, CameraLerpSpeed);

	SetActorLocation(NewLocation);
	SetActorRotation(NewRotation);
}

void AFightingCameraActor::ApplyHitShake(float Intensity)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->ClientStartCameraShake(nullptr, Intensity);
		// For proper camera shake, create a CameraShakeBase Blueprint:
		// Name: CS_HitShake
		// Set it in a derived BP or call PC->ClientStartCameraShake with the class
	}
}
