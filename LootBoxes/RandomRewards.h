#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "API/ARK/Ark.h"
#include "Lootboxes.h"

namespace RandomRewards {
	
	void generateAndGiveRewards(AShooterPlayerController *sender, FString *lootbox);
}