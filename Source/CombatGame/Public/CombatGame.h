#pragma once

#include "CoreMinimal.h"

// Log categories
DECLARE_LOG_CATEGORY_EXTERN(LogCombatGame, Log, All);

// Custom collision channels
#define ECC_FighterHitbox ECollisionChannel::ECC_GameTraceChannel1
#define ECC_FighterHurtbox ECollisionChannel::ECC_GameTraceChannel2
