#pragma once

#include "stdafx.h"
#include "Exam_HelperStructs.h"

struct AgentMemoryInventory
{
	UINT slotID{ 0 };
	eItemType itemType{};
	int itemValue{};
};

struct AgentMemoryHouse
{
	Elite::Vector2 Center{};
	Elite::Vector2 Size{};
	bool recentlyVisited{ false };
};