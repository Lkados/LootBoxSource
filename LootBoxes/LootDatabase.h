#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <SQLiteCpp/Database.h>

#include <API/ARK/Ark.h>
#include <API/UE/Math/ColorList.h>


namespace LootDatabase {
	SQLite::Database & getDB();
	void InitDatabase();
	bool hasLootBox(uint64 steamId, FString lootboxname);
	std::map<std::string, int> getRemainingLootBoxesList(uint64 steamId);
	int getRemainingLootBoxes(uint64 steamId, FString lootboxname);
	void addLootBox(uint64 steamId, FString lootboxname, int amount);
	void decreaseLootBox(uint64 steamId, FString lootboxname);
}