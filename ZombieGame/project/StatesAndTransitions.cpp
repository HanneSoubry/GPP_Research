

#include "stdafx.h"
#include "StatesAndTransitions.h"
#include "GoapActions.h"
#include "EBlackboard.h"
#include "HelperStructs.h"
#include <set>
#include "IExamInterface.h"

//------------
//---STATES---
//------------

#pragma region ExploringState

FSMStates::ExploringState::ExploringState()
{
	m_pGoapPlanner = new Elite::GoapPlanner;
	m_pGoapExecutor = new Elite::GoapExecutor;

	// make actions
	GoapAction* actionFindCheckpoint{ new GOAP_Actions::GOAPActionFindCheckpoint };
	GoapAction* actionLookAround{ new GOAP_Actions::GOAPActionLookAround };

	// save actions in vector
	m_pGoapActions.push_back(actionFindCheckpoint);
	m_pGoapActions.push_back(actionLookAround);

	// add actions to planner
	m_pGoapPlanner->AddAction(actionFindCheckpoint);
	m_pGoapPlanner->AddAction(actionLookAround);
}

void FSMStates::ExploringState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::stack<GoapAction*>* plan{ m_pGoapPlanner->MakePlan(pBlackboard, GoapEffect::ExploreMap, std::vector<GoapEffect>{}) };
	m_pGoapExecutor->SetPlan(plan);
}

#pragma endregion

#pragma region EnterHouseState

FSMStates::EnterHouseState::EnterHouseState()
{
	m_pGoapPlanner = new Elite::GoapPlanner;
	m_pGoapExecutor = new Elite::GoapExecutor;

	// make actions
	GoapAction* actionSearchHouse{ new GOAP_Actions::GOAPActionSearchHouse };

	// save actions in vector
	m_pGoapActions.push_back(actionSearchHouse);

	// add actions to planner
	m_pGoapPlanner->AddAction(actionSearchHouse);
}

void FSMStates::EnterHouseState::OnEnter(Elite::Blackboard* pBlackboard)
{
	std::vector<AgentMemoryHouse>* pFoundHouses;
	if (pBlackboard->GetData("MemoryHouses", pFoundHouses) == false || pFoundHouses == nullptr)
	{
		std::cout << "FSMState: EnterHouseState: no vector of memory houses found.\n";
		return;
	}
	int index;
	if (pBlackboard->GetData("HouseIndex", index) == false)
	{
		std::cout << "FSMState: EnterHouseState: no house index found.\n";
		return;
	}

	// safety check
	std::vector<GoapEffect> currentEffects{};
	if (index >= 0 && index < pFoundHouses->size())
	{
		// valid index
		currentEffects.push_back(GoapEffect::HouseAvailable);
		// if not, current effects == empty
	}

	// make plan
	std::stack<GoapAction*>* plan{	m_pGoapPlanner->MakePlan(pBlackboard, GoapEffect::SearchHouse, currentEffects) };
	m_pGoapExecutor->SetPlan(plan);
}

#pragma endregion

#pragma region ItemHandlingState

FSMStates::ItemHandlingState::ItemHandlingState()
{
	m_pGoapPlanner = new Elite::GoapPlanner;
	m_pGoapExecutor = new Elite::GoapExecutor;

	// make actions
	GoapAction* destoyGarbage{ new GOAP_Actions::GOAPActionDestroyGarbage };
	GoapAction* choseItem{ new GOAP_Actions::GOAPActionChoseItem };
	GoapAction* findItem{ new GOAP_Actions::GOAPActionFindItem };

	// save actions in vector
	m_pGoapActions.push_back(destoyGarbage);
	m_pGoapActions.push_back(choseItem);
	m_pGoapActions.push_back(findItem);

	// add actions to planner
	m_pGoapPlanner->AddAction(destoyGarbage);
	m_pGoapPlanner->AddAction(choseItem);
	m_pGoapPlanner->AddAction(findItem);
}

void FSMStates::ItemHandlingState::OnEnter(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface;
	if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no interface found.\n";
		return;
	}
	std::vector<EntityInfo> pFOVItems;
	if (pBlackboard->GetData("FOVItemInfo", pFOVItems) == false)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no vector of FOV items found.\n";
		return;
	}
	std::vector<EntityInfo> pMemoryItems;
	if (pBlackboard->GetData("ItemsToGrab", pMemoryItems) == false)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no vector of remebered items found.\n";
		return;
	}

	bool garbage{ false };
	bool item{ false };
	for (const EntityInfo& currItem : pFOVItems)
	{
		ItemInfo itemInfo{};
		pInterface->Item_GetInfo(currItem, itemInfo);
		if(itemInfo.Type == eItemType::GARBAGE)
		{
			garbage = true;
		}
		else
		{
			item = true;
		}
	}
	std::vector<GoapEffect> currentEffects{};

	if (!pMemoryItems.empty())
		currentEffects.push_back(GoapEffect::UnknowItemFound);

	if (garbage)
		currentEffects.push_back(GoapEffect::GarbageFound);
	if(item)
		currentEffects.push_back(GoapEffect::ItemFound);

	// make plan
	std::stack<GoapAction*>* plan{ m_pGoapPlanner->MakePlan(pBlackboard, GoapEffect::HandleItem, currentEffects) };
	m_pGoapExecutor->SetPlan(plan);
}

#pragma endregion

#pragma region HealingState

FSMStates::HealingState::HealingState()
{
	m_pGoapPlanner = new Elite::GoapPlanner;
	m_pGoapExecutor = new Elite::GoapExecutor;

	// make actions
	GoapAction* heal{ new GOAP_Actions::GOAPActionHeal };

	// save actions in vector
	m_pGoapActions.push_back(heal);

	// add actions to planner
	m_pGoapPlanner->AddAction(heal);
}

void FSMStates::HealingState::OnEnter(Elite::Blackboard* pBlackboard)
{
	// make plan
	std::stack<GoapAction*>* plan{ m_pGoapPlanner->MakePlan(pBlackboard, GoapEffect::HandleItem, std::vector<GoapEffect>{})};
	m_pGoapExecutor->SetPlan(plan);
}

#pragma endregion

#pragma region RunFromPurgeZoneState

FSMStates::RunFromPurgeZoneState::RunFromPurgeZoneState()
{
	m_pGoapPlanner = new Elite::GoapPlanner;
	m_pGoapExecutor = new Elite::GoapExecutor;

	// make actions
	GoapAction* destoyGarbage{ new GOAP_Actions::GOAPActionDestroyGarbage };
	GoapAction* choseItem{ new GOAP_Actions::GOAPActionChoseItem };
	GoapAction* findItem{ new GOAP_Actions::GOAPActionFindItem };

	// save actions in vector
	m_pGoapActions.push_back(destoyGarbage);
	m_pGoapActions.push_back(choseItem);
	m_pGoapActions.push_back(findItem);

	// add actions to planner
	m_pGoapPlanner->AddAction(destoyGarbage);
	m_pGoapPlanner->AddAction(choseItem);
	m_pGoapPlanner->AddAction(findItem);
}

void FSMStates::RunFromPurgeZoneState::OnEnter(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface;
	if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no interface found.\n";
		return;
	}
	std::vector<EntityInfo> pFOVItems;
	if (pBlackboard->GetData("FOVItemInfo", pFOVItems) == false)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no vector of FOV items found.\n";
		return;
	}
	std::vector<EntityInfo> pMemoryItems;
	if (pBlackboard->GetData("ItemsToGrab", pMemoryItems) == false)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no vector of remebered items found.\n";
		return;
	}

	bool garbage{ false };
	bool item{ false };
	for (const EntityInfo& currItem : pFOVItems)
	{
		ItemInfo itemInfo{};
		pInterface->Item_GetInfo(currItem, itemInfo);
		if (itemInfo.Type == eItemType::GARBAGE)
		{
			garbage = true;
		}
		else
		{
			item = true;
		}
	}
	std::vector<GoapEffect> currentEffects{};

	if (!pMemoryItems.empty())
		currentEffects.push_back(GoapEffect::UnknowItemFound);

	if (garbage)
		currentEffects.push_back(GoapEffect::GarbageFound);
	if (item)
		currentEffects.push_back(GoapEffect::ItemFound);

	// make plan
	std::stack<GoapAction*>* plan{ m_pGoapPlanner->MakePlan(pBlackboard, GoapEffect::HandleItem, currentEffects) };
	m_pGoapExecutor->SetPlan(plan);
}

#pragma endregion

#pragma region EnemyHandlingState

FSMStates::EnemyHandlingState::EnemyHandlingState()
{
	m_pGoapPlanner = new Elite::GoapPlanner;
	m_pGoapExecutor = new Elite::GoapExecutor;

	// TODO: push actions for enemy handling
	// make actions
	GoapAction* run{ new GOAP_Actions::GOAPActionRunFromEnemy };
	GoapAction* attack{ new GOAP_Actions::GOAPActionShootEnemy };
	GoapAction* findEnemy{ new GOAP_Actions::GOAPActionFindEnemy };

	// save actions in vector
	m_pGoapActions.push_back(run);
	m_pGoapActions.push_back(attack);
	m_pGoapActions.push_back(findEnemy);

	// add actions to planner
	m_pGoapPlanner->AddAction(run);
	m_pGoapPlanner->AddAction(attack);
	m_pGoapPlanner->AddAction(findEnemy);
}

void FSMStates::EnemyHandlingState::OnEnter(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface;
	if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
	{
		std::cout << "FSMState: EnemyHandlingState: no interface found.\n";
		return;
	}
	std::vector<EntityInfo> pFOVEnemies;
	if (pBlackboard->GetData("FOVEnemyInfo", pFOVEnemies) == false)
	{
		std::cout << "FSMState: EnemyHandlingState: no vector of FOV items found.\n";
		return;
	}
	std::vector<AgentMemoryInventory>* memoryInventory;
	if (pBlackboard->GetData("MemoryInventory", memoryInventory) == false || memoryInventory == nullptr)
	{
		std::cout << "FSMState: EnemyHandlingState: no memory inventory found.\n";
		return;
	}

	std::vector<GoapEffect> currentEffects{};
	if (!pFOVEnemies.empty())
		currentEffects.push_back(GoapEffect::EnemyVisible);

	bool weaponAvailable{ false };
	for (const AgentMemoryInventory& currMemItem : *memoryInventory)
	{
		if (currMemItem.itemType == eItemType::PISTOL || currMemItem.itemType == eItemType::SHOTGUN)
		{
			ItemInfo itemInfo;
			pInterface->Inventory_GetItem(currMemItem.slotID, itemInfo);
			if (pInterface->Weapon_GetAmmo(itemInfo) > 0)
			{
				weaponAvailable = true;
				break;
			}
		}
	}

	if (weaponAvailable)
		currentEffects.push_back(GoapEffect::LoadedWeaponAvailable);

	// make plan
	std::stack<GoapAction*>* plan{ m_pGoapPlanner->MakePlan(pBlackboard, GoapEffect::HandleItem, currentEffects) };
	m_pGoapExecutor->SetPlan(plan);
}

#pragma endregion

//-----------------
//---TRANSITIONS---
//-----------------

bool FSMConditions::FoundHouseCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	// if house in fov
	std::vector<HouseInfo> pFOVHouses;
	if (pBlackboard->GetData("FOVHouseInfo", pFOVHouses) == false)
	{
		std::cout << "FSMState: EnterHouseState: no vector of FOV houses found.\n";
		return false;
	}

	if (pFOVHouses.empty())
		return false;

	// get info
	std::vector<AgentMemoryHouse>* pFoundHouses;
	if (pBlackboard->GetData("MemoryHouses", pFoundHouses) == false || pFoundHouses == nullptr)
	{
		std::cout << "FSMState: EnterHouseState: no vector of memory houses found.\n";
		return false;
	}

	for (const HouseInfo& currFOVHouse : pFOVHouses)
	{
		bool foundHouse{ false };

		// check if this house was found already
		for(int index{}; index < pFoundHouses->size(); ++index)
		{
			AgentMemoryHouse currHouse{ (*pFoundHouses)[index]};

			if (currHouse.Center == currFOVHouse.Center)
			{
				if (currHouse.recentlyVisited)
				{
					// skip this house
					foundHouse = true;
					continue;
				}
				else
				{
					// visit this house
					pBlackboard->ChangeData("HouseIndex", index);
					return true;
				}
			}
		}

		if (!foundHouse)
		{
			// if not found yet
			AgentMemoryHouse thisHouse{};

			thisHouse.Center = currFOVHouse.Center;
			thisHouse.Size = currFOVHouse.Size;
			thisHouse.recentlyVisited = false;

			pFoundHouses->push_back(thisHouse);
			pBlackboard->ChangeData("HouseIndex", static_cast<int>(pFoundHouses->size()) - 1);
			return true;
		}
	}

	return false;
}

bool FSMConditions::HouseSearchDoneCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<AgentMemoryHouse>* pFoundHouses;
	if (pBlackboard->GetData("MemoryHouses", pFoundHouses) == false || pFoundHouses == nullptr)
	{
		std::cout << "FSMState: EnterHouseState: no vector of memory houses found.\n";
		return true;
	}
	int index;
	if (pBlackboard->GetData("HouseIndex", index) == false)
	{
		std::cout << "FSMState: EnterHouseState: no house index found.\n";
		return true;
	}

	return (*pFoundHouses)[index].recentlyVisited;
	// if true (changed by action) and searching through this house
}

bool FSMConditions::ItemFoundCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<EntityInfo> pFOVItems;
	if (pBlackboard->GetData("FOVItemInfo", pFOVItems) == false)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no vector of FOV items found.\n";
		return false;
	}
	std::vector<EntityInfo> pRememberedItems;
	if (pBlackboard->GetData("ItemsToGrab", pRememberedItems) == false)
	{
		std::cout << "FSMCondition: ItemFoundCondition: no vector of remebered items found.\n";
		return false;
	}

	if (pFOVItems.empty() && pRememberedItems.empty())
		return false;

	return true;
}

bool FSMConditions::ItemHandlingDoneCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	return !ItemFoundCondition().Evaluate(pBlackboard);
}

bool FSMConditions::NeedHealingCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgentInfo agentInfo;
	if (pBlackboard->GetData("AgentInfo", agentInfo) == false)
	{
		std::cout << "NeedHealingCondition: no agent found.\n";
		return false;
	}
	std::vector<AgentMemoryInventory>* pMemoryInventory;
	if (pBlackboard->GetData("MemoryInventory", pMemoryInventory) == false || pMemoryInventory == nullptr)
	{
		std::cout << "NeedHealingCondition: no inventory found.\n";
		return false;
	}

	bool needHealing{ false };
	if (agentInfo.Energy < 5)
	{
		for (const AgentMemoryInventory& currMemItem : *pMemoryInventory)
		{
			if (currMemItem.itemType == eItemType::FOOD)
			{
				needHealing = true;
				break;
			}
		}
	}
	if (!needHealing && agentInfo.Health < 5)
	{
		for (const AgentMemoryInventory& currMemItem : *pMemoryInventory)
		{
			if (currMemItem.itemType == eItemType::MEDKIT)
			{
				needHealing = true;
				break;
			}
		}
	}

	return needHealing;
}

bool FSMConditions::NoNeedHealingCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	return !NeedHealingCondition().Evaluate(pBlackboard);
}

bool FSMConditions::PurgeZoneCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<EntityInfo> purgeZones{};
	if (pBlackboard->GetData("FOVPurgeZoneInfo", purgeZones) == false)
	{
		std::cout << "PurgeZoneCondition: no purge zone vector found.\n";
		return false;
	}

	return !purgeZones.empty();
}

bool FSMConditions::NoPurgeZoneCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	return !PurgeZoneCondition().Evaluate(pBlackboard);
}

bool FSMConditions::WasAttackedCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgentInfo agentInfo;
	if (pBlackboard->GetData("AgentInfo", agentInfo) == false)
	{
		std::cout << "WasAttackedCondition: no agent found.\n";
		return false;
	}

	return agentInfo.WasBitten;
}

bool FSMConditions::EnemyGoneCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	bool done;
	if (pBlackboard->GetData("DoneDefending", done) == false)
	{
		std::cout << "EnemyGoneCondition: no doneDefending bool found.\n";
		return false;
	}

	std::vector<EntityInfo> enemies;
	if (pBlackboard->GetData("FOVEnemyInfo", enemies) == false)
	{
		std::cout << "EnemyGoneCondition: no enemy vector found.\n";
		return false;
	}

	return (done && enemies.empty());
}
