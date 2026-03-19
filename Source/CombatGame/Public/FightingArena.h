// FightingArena.h - Base arena/stage actor with boundaries
// Create arena Blueprints: BP_Arena_[StageName] (e.g., BP_Arena_Temple)
// Place one in each stage map
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FightingArena.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class COMBATGAME_API AFightingArena : public AActor
{
	GENERATED_BODY()

public:
	AFightingArena();

	virtual void Tick(float DeltaTime) override;

	// Visible floor mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UStaticMeshComponent* FloorMesh;

	// Arena boundaries - invisible walls
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UBoxComponent* LeftWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UBoxComponent* RightWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UBoxComponent* BackWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	UBoxComponent* FrontWall;

	// Arena size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
	float ArenaWidth = 1200.0f;   // X axis (fight axis)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
	float ArenaDepth = 600.0f;    // Y axis (sidestep axis)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
	float WallHeight = 500.0f;

	// Wall pushback (prevents fighters from going through walls)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
	float WallPushForce = 500.0f;

	// Spawn points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
	FVector Player1SpawnPoint = FVector(-300.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena")
	FVector Player2SpawnPoint = FVector(300.0f, 0.0f, 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Arena")
	void ConstrainFighterToArena(class ACombatCharacter* Fighter);

private:
	void SetupWalls();
};
