#define _CRT_SECURE_NO_WARNINGS

#include "SQLiteCpp/Database.h"
#include "LootDatabase.h"
#include "LootBoxes.h"
#include "API/ARK/Ark.h"
#include <iostream>
#include <sstream>



SQLite::Database& LootDatabase::getDB() {

	const std::string path = LootBoxes::config["DBFilePath"];
	static SQLite::Database db((path + "/lootboxes.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
	return db;
}

void LootDatabase::InitDatabase()
{
	SQLite::Statement query(getDB(), "CREATE TABLE IF NOT EXISTS lootboxes(SteamId integer not null, Boxes text);");
	query.exec();
}


std::map<std::string, int> fromString(std::string const& s)
{
	std::map<std::string, int> m;

	std::string::size_type key_pos = 0;
	std::string::size_type key_end;
	std::string::size_type val_pos;
	std::string::size_type val_end;

	while ((key_end = s.find(':', key_pos)) != std::string::npos)
	{
		if ((val_pos = s.find_first_not_of(": ", key_end)) == std::string::npos)
			break;

		val_end = s.find(';', val_pos);
		m.emplace(s.substr(key_pos, key_end - key_pos), (int)std::stoull(s.substr(val_pos, val_end - val_pos), 0, 10));

		key_pos = val_end;
		if (key_pos != std::string::npos)
			++key_pos;
	}

	return m;
}

bool LootDatabase::hasLootBox(uint64 steamId, FString lootboxname)
{
	return getRemainingLootBoxes(steamId, lootboxname) > 0;
}


std::map<std::string, int> LootDatabase::getRemainingLootBoxesList(uint64 steamId) {

	std::map<std::string, int> boxes;
	SQLite::Statement query(getDB(), "SELECT * FROM lootboxes WHERE SteamId = ? LIMIT 1;");
	query.bind(1, static_cast<int64>(steamId));
	if (!query.executeStep()) return boxes;
	if (query.getColumn("Boxes").getString().empty()) return boxes;
	std::string available = query.getColumn("Boxes").getString();

	return fromString(available);
}

int LootDatabase::getRemainingLootBoxes(uint64 steamId, FString lootboxname) {

	std::map<std::string, int> lootboxes = getRemainingLootBoxesList(steamId);

	if (lootboxes.size() == 0) return 0;

	for (auto const& current : lootboxes) {
		if (current.first._Equal(lootboxname.ToString())) {
			return current.second;
		}
	}
	return 0;
}

void LootDatabase::addLootBox(uint64 steamId, FString lootboxname, int amount) {
	auto lootboxes = getRemainingLootBoxesList(steamId);
	if (lootboxes.size() == 0) {
		// Create a new database entry
		SQLite::Statement query(getDB(), "INSERT INTO lootboxes (SteamId, Boxes) VALUES (?, ?)");

		std::stringstream text;
		text << lootboxname.ToString() << ":" << amount;

		query.bind(1, static_cast<int64>(steamId));
		query.bind(2, text.str());
		query.exec();
	}
	else {
		// Just update the current one [check if this lootbox already exists]
		std::stringstream text;

		for (auto const& current : lootboxes) {

			if (text.str().length() > 0) {
				if (current.first._Equal(lootboxname.ToString())) {
					text << ";" << current.first << ":" << amount + current.second;
				}
				else {
					text << ";" << current.first << ":" << current.second;
				}
			} else
			if (current.first._Equal(lootboxname.ToString())) {
				text << current.first << ":" << amount+current.second;
			}
			else {
				text << current.first << ":" << current.second;
			}
		}
		if (!hasLootBox(steamId, lootboxname)) {
			if (text.str().length() > 0) {
				text << ";" << lootboxname.ToString() << ":" << amount;
			}
			else {
				text << lootboxname.ToString() << ":" << amount;
			}
		}

		SQLite::Statement query(getDB(), "UPDATE lootboxes SET Boxes = ? WHERE SteamId = ?");
		query.bind(1, text.str());
		query.bind(2, static_cast<int64>(steamId));
		query.exec();
	}
}
void LootDatabase::decreaseLootBox(uint64 steamId, FString lootboxname) {
	addLootBox(steamId, lootboxname, -1);
}