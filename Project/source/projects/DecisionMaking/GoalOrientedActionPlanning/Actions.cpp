#include "stdafx.h"
#include "Actions.h"
#include <iostream>

using namespace Elite;
using namespace GOAP_Actions;

//------------
//---ACTIONS---
//------------

#pragma region Wander

GOAP_Actions::GOAPActionWander::GOAPActionWander()
{
	m_PositionRequired = false;
	m_ActionID = 1;
}

GOAP_Actions::GOAPActionWander::~GOAPActionWander()
{
	// nothing to destroy
}

BehaviorState GOAPActionWander::ExecuteAction(Blackboard* pBlackBoard)
{
	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return BehaviorState::Failure;

	pAgent->SetToWander();
	return BehaviorState::Success;
}

bool GOAP_Actions::GOAPActionWander::AbortAction(Blackboard* pBlackBoard)
{
	return false;	// wander cannot go wrong
}

int GOAPActionWander::GetValue(Blackboard* pBlackBoard)
{
	return 0;	// no value
}

bool GOAPActionWander::IsPreconditionTrue(Blackboard* pBlackBoard)
{	
	return true;	// always true because no precondition
}

void GOAPActionWander::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType)
{
	// no required pos, class must override this function
}

#pragma endregion

#pragma region EatFood

GOAP_Actions::GOAPActionEatFood::GOAPActionEatFood()
{
	m_PositionRequired = true;
	m_ActionID = 2;
}

GOAP_Actions::GOAPActionEatFood::~GOAPActionEatFood()
{
	// nothing to destroy
}

BehaviorState GOAPActionEatFood::ExecuteAction(Blackboard* pBlackBoard)
{
	// touching food is enough, nothing to do
	return BehaviorState::Success;
}

bool GOAP_Actions::GOAPActionEatFood::AbortAction(Blackboard* pBlackBoard)
{
	AgarioFood* pTargetFood;
	if (pBlackBoard->GetData("TargetFood", pTargetFood) == false || pTargetFood == nullptr)
	{
		// cannot find food or food is gone
		return true;
	}

	return false;
}

int GOAPActionEatFood::GetValue(Blackboard* pBlackBoard)
{
	return 1;	// food always has value 1
}

bool GOAPActionEatFood::IsPreconditionTrue(Blackboard* pBlackBoard)
{
	std::vector<AgarioFood*>* pNearbyFood;
	if (pBlackBoard->GetData("NearbyFood", pNearbyFood) == false || pNearbyFood == nullptr)
	{
		std::cout << "Warning: pNearbyFood does not exist\n";
		return false;
	}

	if (!pNearbyFood->empty())	// food available
		return true;
	else
		return false;
}

void GOAPActionEatFood::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType)
{
	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return;
	}
	std::vector<AgarioFood*>* pNearbyFood;
	if (pBlackBoard->GetData("NearbyFood", pNearbyFood) == false || pNearbyFood == nullptr)
	{
		std::cout << "Warning: pNearbyFood does not exist\n";
		return;
	}

	int count{ 0 };
	float closestSqrtDistance{ FLT_MAX };
	Vector2 closestFoodPos{};
	AgarioFood* closestFood = nullptr;
	Vector2 agentPos{ pAgent->GetPosition() };

	for (AgarioFood* currFood : *pNearbyFood)
	{
		const Vector2 currFoodPos{ currFood->GetPosition() };
		const float currSqrtDistance{ currFoodPos.DistanceSquared(agentPos) };

		if ( currSqrtDistance < closestSqrtDistance)
		{
			closestSqrtDistance = currSqrtDistance;
			closestFoodPos = currFoodPos;
			closestFood = currFood;
		}

		++count;
	}

	moveTargetType = MoveTargetType::toPosition;
	pBlackBoard->ChangeData("Target", closestFoodPos);
	pBlackBoard->ChangeData("TargetFood", closestFood);
}

#pragma endregion

#pragma region EatEnemy

GOAP_Actions::GOAPActionEatEnemy::GOAPActionEatEnemy()
{
	m_PositionRequired = true;
	m_ActionID = 3;
}

GOAP_Actions::GOAPActionEatEnemy::~GOAPActionEatEnemy()
{
	// nothing to destroy
}

BehaviorState GOAPActionEatEnemy::ExecuteAction(Blackboard* pBlackBoard)
{
	// touching enemy is enough, nothing to do
	return BehaviorState::Success;
}

bool GOAP_Actions::GOAPActionEatEnemy::AbortAction(Blackboard* pBlackBoard)
{
	AgarioAgent* pTargetAgent;
	if (pBlackBoard->GetData("TargetAgent", pTargetAgent) == false || pTargetAgent == nullptr)
		return true;	// cannot find enemy

	int agentIndex{};
	if (pBlackBoard->GetData("TargetAgentIndex", agentIndex) == false)
		return true;


	std::vector<AgarioAgent*>* pOtherAgents;
	if (pBlackBoard->GetData("AgentsVec", pOtherAgents) == false || pOtherAgents == nullptr)
		return true;

	if ((*pOtherAgents)[agentIndex] == nullptr)
		return true;	// enemy was destroyed

	return false;
}

int GOAPActionEatEnemy::GetValue(Blackboard* pBlackBoard)
{
	std::vector<AgarioAgent*>* pNearbyAgents;
	if (pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
	{
		std::cout << "Warning: pNearbyAgents does not exist\n";
		return 0;
	}

	if (pNearbyAgents->empty())
		return 0;

	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return 0;
	}
	const float agentSize{ pAgent->GetRadius() - 1 };	// agent should be 2 larger to eat the enemy

	for (AgarioAgent* currAgent : *pNearbyAgents)
	{
		if (currAgent->GetRadius() < agentSize)
			return true;
	}

	return 0;	// food always has value 1
}

bool GOAPActionEatEnemy::IsPreconditionTrue(Blackboard* pBlackBoard)
{
	std::vector<AgarioAgent*>* pNearbyAgents;
	if (pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
	{
		std::cout << "Warning: pNearbyAgents does not exist\n";
		return false;
	}

	if (pNearbyAgents->empty())
		return false;

	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return false;
	}

	int count{ 0 };
	float targetRadius{ 0 };
	const float agentRadius{ pAgent->GetRadius() - 1 };

	for (AgarioAgent* currEnemy : *pNearbyAgents)
	{
		const float currEnemyRadius{ currEnemy->GetRadius() };
		if (currEnemyRadius < agentRadius && currEnemyRadius > targetRadius)
		{
			// find largest eatable enemy in nearby field
			targetRadius = currEnemyRadius;
		}

		++count;
	}

	return targetRadius;
}

void GOAPActionEatEnemy::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType)
{
	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return;
	}
	std::vector<AgarioAgent*>* pNearbyAgents;
	if (pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
	{
		std::cout << "Warning: pNearbyAgents does not exist\n";
		return;
	}

	int count{ 0 };
	float targetRadius{0};
	const float agentRadius{ pAgent->GetRadius()  - 1};
	AgarioAgent* targetAgent = nullptr;

	for (AgarioAgent* currEnemy : *pNearbyAgents)
	{
		const float currEnemyRadius{ currEnemy->GetRadius() };
		if (currEnemyRadius < agentRadius && currEnemyRadius > targetRadius)
		{
			// find largest eatable enemy in nearby field
			targetRadius = currEnemyRadius;
			targetAgent = currEnemy;
		}

		++count;
	}

	if (targetAgent != nullptr)
		// should find something if precondition is true
	{
		moveTargetType = MoveTargetType::toAgent;
		pBlackBoard->ChangeData("TargetAgent", targetAgent);
	}
}

#pragma endregion

#pragma region RunFromEnemy

GOAP_Actions::GOAPActionRunFromEnemy::GOAPActionRunFromEnemy()
{
	m_PositionRequired = true;
	m_ActionID = 4;
}

GOAP_Actions::GOAPActionRunFromEnemy::~GOAPActionRunFromEnemy()
{
	// nothing to destroy
}

BehaviorState GOAPActionRunFromEnemy::ExecuteAction(Blackboard* pBlackBoard)
{
	// just running (MoveTo will handle this)
	return BehaviorState::Success;
}

bool GOAP_Actions::GOAPActionRunFromEnemy::AbortAction(Blackboard* pBlackBoard)
{
	AgarioAgent* pTargetAgent;
	if (pBlackBoard->GetData("TargetAgent", pTargetAgent) == false || pTargetAgent == nullptr)
	{
		// cannot find enemy
		return true;
	}

	int agentIndex{};
	if (pBlackBoard->GetData("TargetAgentIndex", agentIndex) == false)
		return true;


	std::vector<AgarioAgent*>* pOtherAgents;
	if (pBlackBoard->GetData("AgentsVec", pOtherAgents) == false || pOtherAgents == nullptr)
		return true;

	if ((*pOtherAgents)[agentIndex] == nullptr)
		return true;	// enemy was destroyed


	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return true;
	}

	float searchRadius;
	if (pBlackBoard->GetData("SearchRadius", searchRadius) == false)
	{
		return true;
	}

	// +5 is small margin
	if (pTargetAgent->GetPosition().DistanceSquared(pAgent->GetPosition()) > searchRadius * searchRadius + 5)
	{
		// ran far enough
		return true;
	}

	return false;
}

int GOAPActionRunFromEnemy::GetValue(Blackboard* pBlackBoard)
{
	std::vector<AgarioAgent*>* pNearbyAgents;
	if (pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
	{
		std::cout << "Warning: pNearbyAgents does not exist\n";
		return 0;
	}

	if (pNearbyAgents->empty())
		return 0;

	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return 0;
	}
	const float agentSize{ pAgent->GetRadius() - 1 };	// agent should be 2 larger to eat the enemy

	for (AgarioAgent* currAgent : *pNearbyAgents)
	{
		if (currAgent->GetRadius() < agentSize)
			return true;
	}

	return 0;	// food always has value 1
}

bool GOAPActionRunFromEnemy::IsPreconditionTrue(Blackboard* pBlackBoard)
{
	std::vector<AgarioAgent*>* pNearbyAgents;
	if (pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
	{
		std::cout << "Warning: pNearbyAgents does not exist\n";
		return false;
	}

	if (pNearbyAgents->empty())
		return false;

	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return false;
	}

	int count{ 0 };
	float targetRadius{ 0 };
	const float agentRadius{ pAgent->GetRadius() - 1 };

	for (AgarioAgent* currEnemy : *pNearbyAgents)
	{
		const float currEnemyRadius{ currEnemy->GetRadius() };
		if (currEnemyRadius < agentRadius && currEnemyRadius > targetRadius)
		{
			// find largest eatable enemy in nearby field
			targetRadius = currEnemyRadius;
		}

		++count;
	}

	return targetRadius;
}

void GOAPActionRunFromEnemy::GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType)
{
	AgarioAgent* pAgent;
	if (pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		std::cout << "Warning: pAgent does not exist\n";
		return;
	}
	std::vector<AgarioAgent*>* pNearbyAgents;
	if (pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
	{
		std::cout << "Warning: pNearbyAgents does not exist\n";
		return;
	}

	int count{ 0 };
	float targetRadius{ 0 };
	const float agentRadius{ pAgent->GetRadius() - 1 };
	AgarioAgent* targetAgent = nullptr;

	for (AgarioAgent* currEnemy : *pNearbyAgents)
	{
		const float currEnemyRadius{ currEnemy->GetRadius() };
		if (currEnemyRadius < agentRadius && currEnemyRadius > targetRadius)
		{
			// find largest eatable enemy in nearby field
			targetRadius = currEnemyRadius;
			targetAgent = currEnemy;
		}

		++count;
	}

	if (targetAgent != nullptr)
		// should find something if precondition is true
	{
		moveTargetType = MoveTargetType::toAgent;
		pBlackBoard->ChangeData("TargetAgent", targetAgent);
	}
}

#pragma endregion