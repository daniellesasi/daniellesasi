// FightingCameraActor.h - Tekken-style 3D fighting camera
// Automatically tracks both fighters, zooms based on distance, stays at a fixed height
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "FightingCameraActor.generated.h"

UCLASS()
class COMBATGAME_API AFightingCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AFightingCameraActor();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

	// Fighter references (set by GameMode)
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	class ACombatCharacter* Fighter1;

	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	class ACombatCharacter* Fighter2;

	// Camera settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float CameraHeight = 120.0f;

	// Distance from the midpoint along Y axis (perpendicular to fight axis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float BaseDistance = 400.0f;

	// How much the camera zooms out when fighters are far apart
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float DistanceMultiplier = 0.8f;

	// Min/max camera distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MinDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MaxDistance = 800.0f;

	// Camera smoothing speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float CameraLerpSpeed = 5.0f;

	// Field of view
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float CameraFOV = 60.0f;

	// Camera shake on big hits
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyHitShake(float Intensity);

private:
	FVector DesiredLocation;
	FRotator DesiredRotation;
};
