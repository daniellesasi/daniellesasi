#include "FightingArena.h"
#include "CombatCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AFightingArena::AFightingArena()
{
	PrimaryActorTick.bCanEverTick = true;

	// Floor
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	RootComponent = FloorMesh;

	// Walls
	LeftWall = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(RootComponent);
	LeftWall->SetCollisionProfileName(TEXT("BlockAll"));
	LeftWall->SetHiddenInGame(true);

	RightWall = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(RootComponent);
	RightWall->SetCollisionProfileName(TEXT("BlockAll"));
	RightWall->SetHiddenInGame(true);

	BackWall = CreateDefaultSubobject<UBoxComponent>(TEXT("BackWall"));
	BackWall->SetupAttachment(RootComponent);
	BackWall->SetCollisionProfileName(TEXT("BlockAll"));
	BackWall->SetHiddenInGame(true);

	FrontWall = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontWall"));
	FrontWall->SetupAttachment(RootComponent);
	FrontWall->SetCollisionProfileName(TEXT("BlockAll"));
	FrontWall->SetHiddenInGame(true);

	SetupWalls();
}

void AFightingArena::SetupWalls()
{
	float HalfWidth = ArenaWidth * 0.5f;
	float HalfDepth = ArenaDepth * 0.5f;
	float HalfHeight = WallHeight * 0.5f;

	LeftWall->SetBoxExtent(FVector(10.0f, HalfDepth, HalfHeight));
	LeftWall->SetRelativeLocation(FVector(-HalfWidth, 0.0f, HalfHeight));

	RightWall->SetBoxExtent(FVector(10.0f, HalfDepth, HalfHeight));
	RightWall->SetRelativeLocation(FVector(HalfWidth, 0.0f, HalfHeight));

	BackWall->SetBoxExtent(FVector(HalfWidth, 10.0f, HalfHeight));
	BackWall->SetRelativeLocation(FVector(0.0f, HalfDepth, HalfHeight));

	FrontWall->SetBoxExtent(FVector(HalfWidth, 10.0f, HalfHeight));
	FrontWall->SetRelativeLocation(FVector(0.0f, -HalfDepth, HalfHeight));
}

void AFightingArena::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFightingArena::ConstrainFighterToArena(ACombatCharacter* Fighter)
{
	if (!Fighter) return;

	FVector Loc = Fighter->GetActorLocation();
	float HalfWidth = ArenaWidth * 0.5f;
	float HalfDepth = ArenaDepth * 0.5f;

	Loc.X = FMath::Clamp(Loc.X, -HalfWidth + 50.0f, HalfWidth - 50.0f);
	Loc.Y = FMath::Clamp(Loc.Y, -HalfDepth + 50.0f, HalfDepth - 50.0f);

	Fighter->SetActorLocation(Loc);
}
