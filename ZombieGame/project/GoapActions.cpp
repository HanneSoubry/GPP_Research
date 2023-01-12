
#include "stdafx.h"
#include "GoapActions.h"
#include "Steering.h"
#include "IExamInterface.h"
#include "HelperStructs.h"

using namespace Elite;
using namespace GOAP_Actions;

#pragma region FindCheckPoint_ID_1

GOAPActionFindCheckpoint::GOAPActionFindCheckpoint()
{
	m_PositionRequired = true;
	m_ActionID = 1;
	
	// default initialize, value will be calculated later, needs blackboard
	m_Value = 0;

	//m_PreconditionsVec empty
	m_EffectsVec.push_back(GoapEffect::ExploreMap);
}

BehaviorState GOAPActionFindCheckpoint::ExecuteAction(Blackboard* pBlackBoard)
{
	if (m_FirstTimeCalled)
		CalculateCheckPoints(pBlackBoard);

	m_Checkpoints.pop_front();	// checkpoint reached

	std::vector<AgentMemoryHouse>* pMemoryHouses;
	if (pBlackBoard->GetData("MemoryHouses", pMemoryHouses) == false || pMemoryHouses == nullptr)
	{
		std::cout << "GoapActionFindCheckPoint: ExecuteAction: no memory of houses found.\n";
	}

	if (m_Checkpoints.empty() || pMemoryHouses->size() == 20)
	{	
		// no checkpoints left or found all houses
		// go to all houses again
		for (AgentMemoryHouse& currHouse : *pMemoryHouses)
		{
			currHouse.recentlyVisited = false;
			m_Checkpoints.push_back(currHouse.Center);
		}
	}
	
	return BehaviorState::Success;
}

void GOAPActionFindCheckpoint::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (m_FirstTimeCalled)
		CalculateCheckPoints(pBlackBoard);

	moveInfo.targetType = MoveTargetType::toPosition;

	if (m_Checkpoints.empty())
	{
		std::cout << "GoapActionFindCheckpoint: no checkpoint available.\n";
		moveInfo.targetPos = { 0,0 };
		return;
	}

	// check if in house that is already found
	Vector2 nextCheckpoint{};
	bool alreadyFound{ true };
	
	std::vector<AgentMemoryHouse>* pHouses;
	if (pBlackBoard->GetData("MemoryHouses", pHouses) == false || pHouses == nullptr)
	{
		std::cout << "GoapActionSearchHouse: memoryHouses not found.\n";
		nextCheckpoint = m_Checkpoints.front();
	}
	else if(!pHouses->empty())
	{
		while (alreadyFound)
		{ 
			nextCheckpoint = m_Checkpoints.front();
			if (m_Checkpoints.size() <= 1)
			{
				// accept last remaining checkpoint, even if in a found house
				alreadyFound = false;
			}
			else
			{
				for (const AgentMemoryHouse& currHouse : *pHouses)
				{
					float houseSizeX{ currHouse.Size.x / 2 };
					float houseSizeY{ currHouse.Size.y / 2 };
	
					if (nextCheckpoint.x >= currHouse.Center.x - houseSizeX &&
						nextCheckpoint.x <= currHouse.Center.x + houseSizeX &&
						nextCheckpoint.y >= currHouse.Center.y - houseSizeY &&
						nextCheckpoint.y <= currHouse.Center.y + houseSizeY)
					{
						// inside house
						m_Checkpoints.pop_front();
					}
					else
					{
						alreadyFound = false;
					}
				}
			}
		}
	}
	else  // no houses yet
	{
		nextCheckpoint = m_Checkpoints.front();
	}

	moveInfo.targetPos = nextCheckpoint;
	moveInfo.acceptedDistance = 7;
	moveInfo.targetVelocity = { 0,0 };
}

int GOAPActionFindCheckpoint::GetValue(Blackboard* pBlackBoard)
{
	if (m_FirstTimeCalled)
		CalculateCheckPoints(pBlackBoard);

	return m_Value;
}

void GOAPActionFindCheckpoint::CalculateCheckPoints(Blackboard* pBlackBoard)
{
	IExamInterface* pInterface;
	if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "GoapActionFindCheckPoint: interface not found.\n";
	}

	Vector2 worldCenter{ pInterface->World_GetInfo().Center };

	// calculate checkpoints
	m_Checkpoints.clear();

	float diameter{ 25 };
	int dotsPerCircle{ 4 };
	const float nrCircles{ 7 };

	float angle{ static_cast<float>(2 * M_PI / dotsPerCircle) };

	for (int cirlce{ 0 }; cirlce < nrCircles; ++cirlce)
	{
		for (int dot{ 0 }; dot < dotsPerCircle; ++dot)
		{
			Elite::Vector2 circleCenter{ worldCenter.x + diameter * cosf(dot * angle), worldCenter.y + diameter * sinf(dot * angle) };
			m_Checkpoints.push_back(circleCenter);
		}

		if (cirlce < 2)
		{
			diameter *= 2;
			dotsPerCircle *= 2;
			angle /= 2;
		}
		else
		{
			diameter += 50;
			dotsPerCircle += 4;
			angle = static_cast<float>(2 * M_PI / dotsPerCircle);
		}
	}

	// the more checkpoints left, the more value 
	m_Value = m_Checkpoints.size();
	m_FirstTimeCalled = false;
}

#pragma endregion

#pragma region SearchHouse_ID_2

GOAPActionSearchHouse::GOAPActionSearchHouse()
{
	m_PositionRequired = true;
	m_ActionID = 2;

	// just a value
	m_Value = 10;	

	m_PreconditionsVec.push_back(GoapEffect::HouseAvailable);
	m_EffectsVec.push_back(GoapEffect::SearchHouse);
}

BehaviorState GOAPActionSearchHouse::ExecuteAction(Blackboard* pBlackBoard)
{
	if (m_StartNewHouse)
		CalculateCheckPoints(pBlackBoard);
	else
	{
		m_Checkpoints.pop_front();	// checkpoint reached
		if (m_Checkpoints.empty())
		{
			// house finished
			std::vector<AgentMemoryHouse>* pHouses;
			if (pBlackBoard->GetData("MemoryHouses", pHouses) == false || pHouses == nullptr)
			{
				std::cout << "GoapActionSearchHouse: memoryHouses not found.\n";
			}
			int houseIndex;
			if (pBlackBoard->GetData("HouseIndex", houseIndex) == false)
			{
				std::cout << "GoapActionSearchHouse: houseIndex not found.\n";
			}

			(*pHouses)[houseIndex].recentlyVisited = true;
			m_PositionRequired = false;
			m_StartNewHouse = true;
		}
	}

	return BehaviorState::Success;
}

void GOAPActionSearchHouse::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (m_StartNewHouse)
		CalculateCheckPoints(pBlackBoard);

	moveInfo.targetType = MoveTargetType::toPosition;

	if (m_Checkpoints.empty())
	{
		std::cout << "GoapActionFindCheckpoint: no checkpoint for house available.\n";
		moveInfo.targetPos = { 0,0 };
		return;
	}

	moveInfo.targetPos = m_Checkpoints.front();
	moveInfo.acceptedDistance = 5;
	moveInfo.targetVelocity = { 0,0 };
}

void GOAPActionSearchHouse::CalculateCheckPoints(Blackboard* pBlackBoard)
{
	std::vector<AgentMemoryHouse>* pHouses;
	if (pBlackBoard->GetData("MemoryHouses", pHouses) == false || pHouses == nullptr)
	{
		std::cout << "GoapActionSearchHouse: memoryHouses not found.\n";
	}
	int houseIndex;
	if (pBlackBoard->GetData("HouseIndex", houseIndex) == false)
	{
		std::cout << "GoapActionSearchHouse: houseIndex not found.\n";
	}

	const AgentMemoryHouse& searchHouse{ (*pHouses)[houseIndex] };

	Vector2 houseCenter{ searchHouse.Center };
	Vector2 searchSize{ searchHouse.Size / 4 };

	// calculate checkpoints
	m_Checkpoints.clear();

	m_Checkpoints.push_back({ houseCenter.x - searchSize.x, houseCenter.y + searchSize.y });
	m_Checkpoints.push_back({ houseCenter.x + searchSize.x, houseCenter.y + searchSize.y });
	m_Checkpoints.push_back({ houseCenter.x - searchSize.x, houseCenter.y - searchSize.y });
	m_Checkpoints.push_back({ houseCenter.x + searchSize.x, houseCenter.y - searchSize.y });

	m_StartNewHouse = false;
	m_PositionRequired = true;
}

#pragma endregion

#pragma region LookAround_ID_3

GOAPActionLookAround::GOAPActionLookAround()
{
	m_PositionRequired = true;
	m_ActionID = 3;

	// first thing to be done
	m_Value = INT_MAX;

	//m_PreconditionsVec empty
	m_EffectsVec.push_back(GoapEffect::ExploreMap);
}

BehaviorState GOAPActionLookAround::ExecuteAction(Blackboard* pBlackBoard)
{
	if (m_FirstTimeCalled)
		CalculateCheckPoints(pBlackBoard);

	m_Checkpoints.pop_front();	// checkpoint reached

	if (m_Checkpoints.empty())
	{
		// done full circle
		m_Value = 0;
	}

	return BehaviorState::Success;
}

void GOAPActionLookAround::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (m_FirstTimeCalled)
		CalculateCheckPoints(pBlackBoard);

	moveInfo.targetType = MoveTargetType::faceTo;

	if (m_Checkpoints.empty())
	{
		std::cout << "GoapActionFindCheckpoint: no checkpoint available.\n";
		moveInfo.targetPos = { 0,0 };
		return;
	}

	moveInfo.targetPos = m_Checkpoints.front();
	moveInfo.acceptedDistance = 0.2;
	moveInfo.targetVelocity = { 0,0 };
}

void GOAPActionLookAround::CalculateCheckPoints(Blackboard* pBlackBoard)
{
	IExamInterface* pInterface;
	if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "GoapActionFindCheckPoint: interface not found.\n";
	}

	Vector2 worldCenter{ pInterface->World_GetInfo().Center };

	// calculate checkpoints
	m_Checkpoints.clear();

	int dotsPerCircle{ 3 };
	float angle{ static_cast<float>(2 * M_PI / dotsPerCircle) };

	for (int dot{ 1 }; dot <= dotsPerCircle; ++dot)
	{
		Elite::Vector2 circleCenter{ worldCenter.x + cosf(dot * angle), worldCenter.y + sinf(dot * angle) };
		m_Checkpoints.push_back(circleCenter);
	}

	m_FirstTimeCalled = false;
}

#pragma endregion

#pragma region DestroyGarbage_ID_4

GOAPActionDestroyGarbage::GOAPActionDestroyGarbage()
{
	m_PositionRequired = true;
	m_ActionID = 4;

	m_Value = 20;

	m_PreconditionsVec.push_back(GoapEffect::GarbageFound);
	m_EffectsVec.push_back(GoapEffect::HandleItem);
}

BehaviorState GOAPActionDestroyGarbage::ExecuteAction(Blackboard* pBlackBoard)
{
	IExamInterface* pInterface;
	if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "GoapActionDestroyGarbage: ExecuteAction: no interface found.\n";
		return BehaviorState::Failure;
	}
	std::vector<EntityInfo> items;
	if (pBlackBoard->GetData("FOVItemInfo", items) == false)
	{
		std::cout << "GoapActionDestroyGarbage: ExecuteAction: no items found.\n";
		return BehaviorState::Failure;
	}

	for (int i{ 0 }; i < items.size(); ++i)
	{
		if (items[i].Location == m_TargetPos)
		{
			pInterface->Item_Destroy(items[i]);
			return BehaviorState::Success;
		}
	}

	// item not found
	return BehaviorState::Failure;
}

void GOAPActionDestroyGarbage::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (!m_TargetSet)
	{
		std::vector<EntityInfo> items;
		if (pBlackBoard->GetData("FOVItemInfo", items) == false)
		{
			std::cout << "GoapActionDestroyGarbage: GetRequiredPosInfo: no items found.\n";
			return;
		}
		
		EntityInfo target{ items[0] };

		if (items.size() > 1)
		{
			AgentInfo agent;
			if (pBlackBoard->GetData("AgentInfo", agent) == false)
			{
				std::cout << "GoapActionDestroyGarbage: GetRequiredPosInfo: no agent found.\n";
				return;
			}
			IExamInterface* pInterface;
			if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
			{
				std::cout << "GoapActionDestroyGarbage: ExecuteAction: no interface found.\n";
				return;
			}

			float closestDistance{};
			int closestIndex{};
			for (int i{0}; i < items.size(); ++i)
			{
				ItemInfo itemInfo{};
				pInterface->Item_GetInfo(items[i], itemInfo);

				if (itemInfo.Type == eItemType::GARBAGE)
				{
					const float distance{ items[i].Location.DistanceSquared(agent.Position) };
					if (distance < closestDistance)
					{
						closestDistance = distance;
						closestIndex = i;
					}
				}
			}

			target = items[closestIndex];
		}

		m_TargetPos = target.Location;
	}
	
	moveInfo.targetType = MoveTargetType::seekAndFace;

	moveInfo.targetPos = m_TargetPos;
	moveInfo.acceptedDistance = 1;
	moveInfo.targetVelocity = { 0,0 };
}

#pragma endregion

#pragma region ChoseItem_ID_5

GOAPActionChoseItem::GOAPActionChoseItem()
{
	m_PositionRequired = true;
	m_ActionID = 5;

	m_Value = 30;

	m_PreconditionsVec.push_back(GoapEffect::ItemFound);
	m_EffectsVec.push_back(GoapEffect::HandleItem);
}

BehaviorState GOAPActionChoseItem::ExecuteAction(Blackboard* pBlackBoard)
{
	// get info
	std::vector<EntityInfo> items;
	if (pBlackBoard->GetData("FOVItemInfo", items) == false)
	{
		std::cout << "GoapActionChoseItem: ExecuteAction: no items found.\n";
		return BehaviorState::Failure;
	}
	IExamInterface* pInterface;
	if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "GoapActionChoseItem: ExecuteAction: no interface found.\n";
		return BehaviorState::Failure;
	}

	// find target item
	EntityInfo targetEntityInfo{};
	bool itemFound{ false };
	for (int i{ 0 }; i < items.size(); ++i)
	{
		if (items[i].Location == m_TargetPos)
		{
			targetEntityInfo = items[i];
			itemFound = true;
		}
	}
	if (!itemFound)
	{
		std::cout << "GoapActionChoseItem: ExecuteAction: target item not found.\n";
		return BehaviorState::Failure;
	}

	std::vector<AgentMemoryInventory>* memoryInventory;
	if (pBlackBoard->GetData("MemoryInventory", memoryInventory) == false || memoryInventory == nullptr)
	{
		std::cout << "GoapActionChoseItem: ExecuteAction: no memory inventory found.\n";
		return BehaviorState::Failure;
	}

	ItemInfo targetItemInfo{};
	pInterface->Item_GetInfo(targetEntityInfo, targetItemInfo);

	// open spaces in inventory
	if (memoryInventory->size() < pInterface->Inventory_GetCapacity())
	{
		// find empty space
		UINT slotID{ 0 };
		for(int nr{}; nr < pInterface->Inventory_GetCapacity(); ++nr)
		{
			bool slotTaken{ false };
			for (const AgentMemoryInventory& currMemItem : *memoryInventory)
			{
				if (currMemItem.slotID == nr)
				{
					slotTaken = true;
					break;
				}
			}

			if (!slotTaken)
			{
				slotID = nr;
				break;
			}
		}
		pInterface->Item_Grab(targetEntityInfo, targetItemInfo);
		pInterface->Inventory_AddItem(slotID, targetItemInfo);
		AgentMemoryInventory newItem{ slotID, targetItemInfo.Type };
		switch (targetItemInfo.Type)
		{
		case eItemType::FOOD:
			newItem.itemValue = pInterface->Food_GetEnergy(targetItemInfo);
			break;
		case eItemType::PISTOL:
		case eItemType::SHOTGUN:
			newItem.itemValue = pInterface->Weapon_GetAmmo(targetItemInfo);
			break;
		case eItemType::MEDKIT:
			newItem.itemValue = pInterface->Medkit_GetHealth(targetItemInfo);
			break;
		}

		memoryInventory->push_back(newItem);
		return BehaviorState::Success;
	}

	// if inventory full
	std::vector<eItemType> idealInventory;
	// ask a copy of ideal items
	if (pBlackBoard->GetData("IdealInventory", idealInventory) == false)
	{
		std::cout << "GoapActionChoseItem: ExecuteAction: no ideal inventory found.\n";
		return BehaviorState::Failure;
	}

	std::vector<int> indexWrongItems{};
	std::vector<int> indexCorrectItems{};

	// find wrong items compared to ideal
	for (int m{}; m < memoryInventory->size(); ++m)
	{
		bool idealFound{ false };
		for (int i{}; i < idealInventory.size(); ++i)
		{
			// check list of ideal items
			if ((*memoryInventory)[m].itemType == idealInventory[i])
			{
				idealFound = true;
				idealInventory[i] = idealInventory[idealInventory.size() - 1];
				indexCorrectItems.push_back(m);
			}
		}
		
		if (!idealFound)
		{
			// check list of correct items
			for (int i{}; i < indexCorrectItems.size(); ++i)
			{
				// if better value, swap them
				if ((*memoryInventory)[m].itemType == (*memoryInventory)[i].itemType && 
					(*memoryInventory)[m].itemValue > (*memoryInventory)[i].itemValue)
				{
					idealFound = true;
					indexWrongItems.push_back(indexCorrectItems[i]);
					indexCorrectItems[i] = m;
				}
			}
		}

		// else wrong item
		if (!idealFound)
			indexWrongItems.push_back(m);
	}

	// does new item fit in ideal
	bool fitsIn{ false };
	for (int i{}; i < idealInventory.size(); ++i)
	{
		if (idealInventory[i] == targetItemInfo.Type)
		{
			fitsIn = true;
			break;
		}
	}

	// if no wrong items or new item not needed
	if (indexWrongItems.empty() || !fitsIn)
	{
		pInterface->Item_Destroy(targetEntityInfo);
		return BehaviorState::Success;
	}

	// else
	AgentMemoryInventory newItem{ (*memoryInventory)[indexWrongItems[0]] };
	const UINT slotID{ newItem.slotID };
	pInterface->Inventory_RemoveItem(slotID);

	pInterface->Item_Grab(targetEntityInfo, targetItemInfo);
	pInterface->Inventory_AddItem(slotID, targetItemInfo);

	newItem.itemType = targetItemInfo.Type;
	switch (targetItemInfo.Type)
	{
	case eItemType::FOOD:
		newItem.itemValue = pInterface->Food_GetEnergy(targetItemInfo);
		break;
	case eItemType::PISTOL:
	case eItemType::SHOTGUN:
		newItem.itemValue = pInterface->Weapon_GetAmmo(targetItemInfo);
		break;
	case eItemType::MEDKIT:
		newItem.itemValue = pInterface->Medkit_GetHealth(targetItemInfo);
		break;
	}

	return BehaviorState::Success;
}

void GOAPActionChoseItem::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (!m_TargetSet)
	{
		std::vector<EntityInfo> items;
		if (pBlackBoard->GetData("FOVItemInfo", items) == false)
		{
			std::cout << "GoapActionChoseItem: GetRequiredPosInfo: no items found.\n";
			return;
		}

		EntityInfo target{ items[0] };

		if (items.size() > 1)
		{
			AgentInfo agent;
			if (pBlackBoard->GetData("AgentInfo", agent) == false)
			{
				std::cout << "GoapActionChoseItem: GetRequiredPosInfo: no agent found.\n";
				return;
			}
			IExamInterface* pInterface;
			if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
			{
				std::cout << "GoapActionChoseItem: ExecuteAction: no interface found.\n";
				return;
			}

			float closestDistance{};
			int closestIndex{};
			for (int i{ 0 }; i < items.size(); ++i)
			{
				ItemInfo itemInfo{};
				pInterface->Item_GetInfo(items[i], itemInfo);

				if (itemInfo.Type != eItemType::GARBAGE)
				{
					const float distance{ items[i].Location.DistanceSquared(agent.Position) };
					if (distance < closestDistance)
					{
						closestDistance = distance;
						closestIndex = i;
					}
				}
			}

			target = items[closestIndex];
		}

		m_TargetPos = target.Location;
	}

	moveInfo.targetType = MoveTargetType::seekAndFace;

	moveInfo.targetPos = m_TargetPos;
	moveInfo.acceptedDistance = 1;
	moveInfo.targetVelocity = { 0,0 };
}

#pragma endregion

#pragma region FindItem_ID_6

GOAPActionFindItem::GOAPActionFindItem()
{
	m_PositionRequired = true;
	m_ActionID = 6;

	m_Value = 10;

	m_PreconditionsVec.push_back(GoapEffect::UnknowItemFound);
	m_EffectsVec.push_back(GoapEffect::HandleItem);
}

BehaviorState GOAPActionFindItem::ExecuteAction(Blackboard* pBlackBoard)
{
	// reset state of action
	m_TargetSet = false;

	// nothing to do, just find the item
	return BehaviorState::Success;
}

void GOAPActionFindItem::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (!m_TargetSet)
	{
		std::vector<EntityInfo> memoryItems;
		if (pBlackBoard->GetData("ItemsToGrab", memoryItems) == false)
		{
			std::cout << "GoapActionFindItem: GetRequiredPosInfo: no items found.\n";
			return;
		}

		EntityInfo target{ memoryItems[0] };

		if (memoryItems.size() > 1)
		{
			AgentInfo agent;
			if (pBlackBoard->GetData("AgentInfo", agent) == false)
			{
				std::cout << "GoapActionFindItem: GetRequiredPosInfo: no agent found.\n";
				return;
			}

			float closestDistance{};
			int closestIndex{};
			for (int i{ 0 }; i < memoryItems.size(); ++i)
			{
				const float distance{ memoryItems[i].Location.DistanceSquared(agent.Position) };
				if (distance < closestDistance)
				{
					closestDistance = distance;
					closestIndex = i;
				}
			}

			target = memoryItems[closestIndex];
		}

		m_TargetPos = target.Location;
	}

	moveInfo.targetType = MoveTargetType::seekAndFace;

	moveInfo.targetPos = m_TargetPos;
	moveInfo.acceptedDistance = 1;
	moveInfo.targetVelocity = { 0,0 };
}

#pragma endregion

#pragma region Heal_ID_7

GOAPActionHeal::GOAPActionHeal()
{
	m_PositionRequired = false;
	m_ActionID = 7;

	m_Value = 10;

	//m_PreconditionsVec.push_back();		// transition to healing state will check for required item
	m_EffectsVec.push_back(GoapEffect::Heal);
}

BehaviorState GOAPActionHeal::ExecuteAction(Blackboard* pBlackBoard)
{
	// get info
	IExamInterface* pInterface;
	if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "GoapActionHeal: ExecuteAction: no interface found.\n";
		return BehaviorState::Failure;
	}
	std::vector<AgentMemoryInventory>* memoryInventory;
	if (pBlackBoard->GetData("MemoryInventory", memoryInventory) == false || memoryInventory == nullptr)
	{
		std::cout << "GoapActionHeal: ExecuteAction: no memory inventory found.\n";
		return BehaviorState::Failure;
	}
	AgentInfo agentInfo;
	if (pBlackBoard->GetData("AgentInfo", agentInfo) == false)
	{
		std::cout << "NeedHealingCondition: no agent found.\n";
		return BehaviorState::Failure;
	}

	if (agentInfo.Energy < 5)
	{
		for (int i{0}; i < memoryInventory->size(); ++i)
		{
			if ((*memoryInventory)[i].itemType == eItemType::FOOD)
			{
				pInterface->Inventory_UseItem((*memoryInventory)[i].slotID);
				(*memoryInventory)[i] = (*memoryInventory)[memoryInventory->size() - 1];
				(*memoryInventory).pop_back();
			}
		}
	}

	if (agentInfo.Health < 5)
	{
		for (int i{ 0 }; i < memoryInventory->size(); ++i)
		{
			if ((*memoryInventory)[i].itemType == eItemType::MEDKIT)
			{
				pInterface->Inventory_UseItem((*memoryInventory)[i].slotID);
				(*memoryInventory)[i] = (*memoryInventory)[memoryInventory->size() - 1];
				(*memoryInventory).pop_back();
			}
		}
	}

	return BehaviorState::Success;
}

#pragma endregion

#pragma region EscapePurge_ID_8

GOAPActionEscapePurgeZone::GOAPActionEscapePurgeZone()
{
	m_PositionRequired = true;
	m_ActionID = 8;

	m_Value = 10;

	//m_PreconditionsVec.push_back();		// transition to escape purge zone state will check for required item
	m_EffectsVec.push_back(GoapEffect::EscapePurge);
}

BehaviorState GOAPActionEscapePurgeZone::ExecuteAction(Blackboard* pBlackBoard)
{
	// only move needed

	return BehaviorState::Success;
}

void GOAPActionEscapePurgeZone::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (!m_TargetSet)
	{
		std::vector<EntityInfo>* purgeZones;
		if (pBlackBoard->GetData("FOVPurgeZoneInfo", purgeZones) == false)
		{
			std::cout << "GOAPActionEscapePurgeZone: GetRequiredPos: no purge zone vector found.\n";
			return;
		}
		IExamInterface* pInterface;
		if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			std::cout << "GOAPActionEscapePurgeZone: GetRequiredPos: no interface found.\n";
			return;
		}

		PurgeZoneInfo purgeZone{};
		pInterface->PurgeZone_GetInfo((*purgeZones)[0], purgeZone);

		m_TargetPos = purgeZone.Center;
		m_Distance = purgeZone.Radius / 2 + 10;
	}

	moveInfo.targetType = MoveTargetType::fromPosition;

	moveInfo.targetPos = m_TargetPos;
	moveInfo.acceptedDistance = m_Distance;
	moveInfo.targetVelocity = { 0,0 };
}

#pragma endregion

#pragma region RunFromEnemy_ID_9

GOAPActionRunFromEnemy::GOAPActionRunFromEnemy()
{
	m_PositionRequired = true;
	m_ActionID = 9;

	// last option
	m_Value = 0;

	m_PreconditionsVec.push_back(GoapEffect::EnemyVisible);		// transition to escape purge zone state will check for required item
	m_EffectsVec.push_back(GoapEffect::SurviveEnemey);
}

BehaviorState GOAPActionRunFromEnemy::ExecuteAction(Blackboard* pBlackBoard)
{
	// only move needed

	return BehaviorState::Success;
}

void GOAPActionRunFromEnemy::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (!m_TargetSet)
	{
		std::vector<EntityInfo>* enemies;
		if (pBlackBoard->GetData("FOVEnemyInfo", enemies) == false)
		{
			std::cout << "GOAPActionRunFromEnemy: GetRequiredPos: no enemy vector found.\n";
			return;
		}
		IExamInterface* pInterface;
		if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			std::cout << "GOAPActionRunFromEnemy: GetRequiredPos: no interface found.\n";
			return;
		}

		EnemyInfo enemy{};
		pInterface->Enemy_GetInfo((*enemies)[0], enemy);

		m_TargetPos = enemy.Location;
		m_TargetVel = enemy.LinearVelocity;
		
	}

	moveInfo.targetType = MoveTargetType::fromAgent;

	moveInfo.targetPos = m_TargetPos;
	moveInfo.acceptedDistance = 30;
	moveInfo.targetVelocity = m_TargetVel;
}

#pragma endregion

#pragma region ShootEnemy_ID_11

GOAPActionShootEnemy::GOAPActionShootEnemy()
{
	m_PositionRequired = true;
	m_ActionID = 11;

	m_Value = 10;

	m_PreconditionsVec.push_back(GoapEffect::LoadedWeaponAvailable);	
	m_PreconditionsVec.push_back(GoapEffect::EnemyVisible);	
	m_EffectsVec.push_back(GoapEffect::SurviveEnemey);
}

BehaviorState GOAPActionShootEnemy::ExecuteAction(Blackboard* pBlackBoard)
{
	// execute when facing enemy
	
	// get info
	std::vector<AgentMemoryInventory>* memoryInventory;
	if (pBlackBoard->GetData("MemoryInventory", memoryInventory) == false || memoryInventory == nullptr)
	{
		std::cout << "GoapActionChoseItem: ExecuteAction: no memory inventory found.\n";
		return BehaviorState::Failure;
	}
	IExamInterface* pInterface;
	if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "GOAPActionRunFromEnemy: ExecuteAction: no interface found.\n";
		return BehaviorState::Failure;
	}

	bool shot{ false };
	for (const AgentMemoryInventory& currMemItem : *memoryInventory)
	{
		if (currMemItem.itemType == eItemType::SHOTGUN)
		{
			// shoot
			pInterface->Inventory_UseItem(currMemItem.slotID);
			ItemInfo gunInfo;

			// check ammo
			pInterface->Inventory_GetItem(currMemItem.slotID, gunInfo);
			if (pInterface->Weapon_GetAmmo(gunInfo) == 0)
			{
				pInterface->Inventory_RemoveItem(currMemItem.slotID);
			}

			pBlackBoard->ChangeData("DoneDefending", true);
			shot = true;
			break;
		}
	}

	if (!shot)
	{
		for (const AgentMemoryInventory& currMemItem : *memoryInventory)
		{
			if (currMemItem.itemType == eItemType::PISTOL)
			{
				// shoot
				pInterface->Inventory_UseItem(currMemItem.slotID);
				ItemInfo gunInfo;

				// check ammo
				pInterface->Inventory_GetItem(currMemItem.slotID, gunInfo);
				if (pInterface->Weapon_GetAmmo(gunInfo) == 0)
				{
					pInterface->Inventory_RemoveItem(currMemItem.slotID);
				}

				pBlackBoard->ChangeData("DoneDefending", true);
				shot = true;
				break;
			}
		}
	}

	return BehaviorState::Success;
}

void GOAPActionShootEnemy::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (!m_TargetSet)
	{
		std::vector<EntityInfo>* enemies;
		if (pBlackBoard->GetData("FOVEnemyInfo", enemies) == false)
		{
			std::cout << "GOAPActionRunFromEnemy: GetRequiredPos: no enemy vector found.\n";
			return;
		}
		IExamInterface* pInterface;
		if (pBlackBoard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			std::cout << "GOAPActionRunFromEnemy: GetRequiredPos: no interface found.\n";
			return;
		}

		EnemyInfo enemy{};
		pInterface->Enemy_GetInfo((*enemies)[0], enemy);

		m_TargetPos = enemy.Location;
		m_TargetVel = enemy.LinearVelocity;
	}

	moveInfo.targetType = MoveTargetType::fromAgent;

	moveInfo.targetPos = m_TargetPos;
	moveInfo.acceptedDistance = 0.15f;
	moveInfo.targetVelocity = m_TargetVel;
}

#pragma endregion

#pragma region LookAround_ID_12

GOAPActionFindEnemy::GOAPActionFindEnemy()
{
	m_PositionRequired = true;
	m_ActionID = 12;

	//m_PreconditionsVec empty
	m_EffectsVec.push_back(GoapEffect::EnemyVisible);
}

BehaviorState GOAPActionFindEnemy::ExecuteAction(Blackboard* pBlackBoard)
{
	if (m_FirstTimeCalled)
		CalculateCheckPoints(pBlackBoard);

	m_Checkpoints.pop_front();	// checkpoint reached

	if (m_Checkpoints.empty())
	{
		m_FirstTimeCalled = true;
		return BehaviorState::Success;
	}

	std::vector<EntityInfo>* enemies;
	if (pBlackBoard->GetData("FOVEnemyInfo", enemies) == false)
	{
		std::cout << "GOAPActionFindEnemy: ExecuteAction: no enemy vector found.\n";
		m_FirstTimeCalled = true;
		return BehaviorState::Failure;
	}

	if (!enemies->empty())
	{
		// enemy found
		m_FirstTimeCalled = true;
		return BehaviorState::Success;
	}
	
	m_FirstTimeCalled = true;
	return BehaviorState::Success;
}

void GOAPActionFindEnemy::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo)
{
	if (m_FirstTimeCalled)
		CalculateCheckPoints(pBlackBoard);

	moveInfo.targetType = MoveTargetType::faceTo;

	if (m_Checkpoints.empty())
	{
		std::cout << "GOAPActionFindEnemy: no checkpoint available.\n";
		moveInfo.targetPos = { 0,0 };
		return;
	}

	moveInfo.targetPos = m_Checkpoints.front();
	moveInfo.acceptedDistance = 0.2;
	moveInfo.targetVelocity = { 0,0 };
}

int GOAP_Actions::GOAPActionFindEnemy::GetValue(Blackboard* pBlackBoard)
{
	std::vector<EntityInfo>* enemies;
	if (pBlackBoard->GetData("FOVEnemyInfo", enemies) == false)
	{
		std::cout << "GOAPActionFindEnemy: GetValue: no enemy vector found.\n";
		return 0;
	}

	if (enemies->empty())
	{
		return 100;
	}
	else
	{
		return 0;
	}
}

void GOAPActionFindEnemy::CalculateCheckPoints(Blackboard* pBlackBoard)
{
	AgentInfo agent;
	if (pBlackBoard->GetData("AgentInfo", agent) == false)
	{
		std::cout << "GOAPActionFindEnemy: agent not found.\n";
	}

	Vector2 center{ agent.Position };

	// calculate checkpoints
	m_Checkpoints.clear();

	int dotsPerCircle{ 3 };
	float angle{ static_cast<float>(2 * M_PI / dotsPerCircle) };

	for (int dot{ 1 }; dot <= dotsPerCircle; ++dot)
	{
		Elite::Vector2 circleCenter{ center.x + cosf(dot * angle), center.y + sinf(dot * angle) };
		m_Checkpoints.push_back(circleCenter);
	}

	m_FirstTimeCalled = false;
}

#pragma endregion