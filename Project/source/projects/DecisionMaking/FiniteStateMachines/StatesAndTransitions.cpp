#include "stdafx.h"
#include "StatesAndTransitions.h"
#include <iostream>

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

//------------
//---STATES---
//------------

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (isValid == false || pAgent == nullptr)
		return;

	pAgent->SetToWander();
	std::cout << "Wander\n";
}

void SeekFoodState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (isValid == false || pAgent == nullptr)
		return;

	AgarioFood* nearestFood;
	if (pBlackboard->GetData("NearestFood", nearestFood) == false || nearestFood == nullptr)
		return;

	pAgent->SetToSeek(nearestFood->GetPosition());
	std::cout << "Seek food\n";
}

void FleeAgentState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	AgarioAgent* pNearestAgent;

	if ( pBlackboard->GetData("Agent", pAgent)== false || pAgent == nullptr)
		return;
	if (pBlackboard->GetData("NearestAgent", pNearestAgent) == false || pNearestAgent == nullptr)
		return;

	pAgent->SetToFlee(pNearestAgent->GetPosition());
	std::cout << "Flee enemy\n";
}

void FleeAgentState::Update(Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	AgarioAgent* pNearestAgent;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return;
	if (pBlackboard->GetData("NearestAgent", pNearestAgent) == false || pNearestAgent == nullptr)
		return;

	const float distance{ (pAgent->GetPosition() - pNearestAgent->GetPosition()).Magnitude() + 1 };
	pAgent->SetToFlee(pNearestAgent->GetPosition(), distance);
}

void PursueAgentState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	AgarioAgent* pNearestAgent;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return;
	if (pBlackboard->GetData("NearestAgent", pNearestAgent) == false || pNearestAgent == nullptr)
		return;

	pAgent->SetToSeek(pNearestAgent->GetPosition());
	std::cout << "Seek enemy\n";
}

void PursueAgentState::Update(Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	AgarioAgent* pNearestAgent;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return;
	if (pBlackboard->GetData("NearestAgent", pNearestAgent) == false || pNearestAgent == nullptr)
		return;

	pAgent->SetToSeek(pNearestAgent->GetPosition());
}

//-----------------
//---TRANSITIONS---
//-----------------

bool FoodNearByCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;
	float searchRadius;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return false;
	if (pBlackboard->GetData("FoodVec", pFoodVec) == false || pFoodVec == nullptr)
		return false;
	if (pBlackboard->GetData("SearchRadius", searchRadius) == false)
		return false;

	searchRadius += pAgent->GetRadius();

	Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, Color{ 1, 0, 0 }, DEBUGRENDERER2D->NextDepthSlice());

	// [] -> variabelen meegeven
	// () -> variabelen voor bewerking
	// {} -> bewerking, MOET bool returnen
	auto isCloser = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2) 
	{
		float dist1 = pFood1->GetPosition().DistanceSquared(agentPos);
		float dist2 = pFood2->GetPosition().DistanceSquared(agentPos);

		return dist1 < dist2;
	};

	auto closestElementIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), isCloser);

	if (closestElementIt == pFoodVec->end())
		return false;

	AgarioFood* closestFood = *closestElementIt;
	if (closestFood->GetPosition().DistanceSquared(agentPos) < searchRadius * searchRadius)
	{
		pBlackboard->ChangeData("NearestFood", closestFood);
		return true;
	}

	return false;
}

bool TargetFoodGoneCondition::Evaluate(Blackboard* pBlackboard) const
{
	std::vector<AgarioFood*>* pFoodVec;
	AgarioFood* pTargetFood;

	if (pBlackboard->GetData("FoodVec", pFoodVec) == false || pFoodVec == nullptr)
		return false;
	if (pBlackboard->GetData("NearestFood", pTargetFood) == false || pTargetFood == nullptr)
		return false;

	for (auto& food : *pFoodVec)
	{
		if (pTargetFood == food)
			return false;
	}

	return true;
}

bool BigAgentNearCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioAgent*>* pAgentsVec;
	float searchRadius;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return false;
	if (pBlackboard->GetData("AllAgents", pAgentsVec) == false || pAgentsVec == nullptr)
		return false;
	if (pBlackboard->GetData("SearchRadius", searchRadius) == false)
		return false;

	searchRadius += pAgent->GetRadius();

	Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, Color{ 1, 0, 0 }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestBigAgent = nullptr;
	float closestDistance = FLT_MAX;
	const float agentRadius{ pAgent->GetRadius() };

	for (AgarioAgent* otherAgent : *pAgentsVec)
	{
		const float otherRadius{ otherAgent->GetRadius() };
		if (otherRadius > agentRadius + 1)
		{
			const float distance{ otherAgent->GetPosition().Distance(agentPos) - otherRadius };
			if (distance < searchRadius && distance < closestDistance)
			{
				closestDistance = distance;
				closestBigAgent = otherAgent;
			}
		}
	}

	if (closestBigAgent == nullptr)
		return false;

	pBlackboard->ChangeData("NearestAgent", closestBigAgent);
	std::cout << "flee agent\n";
	return true;
}

bool FSMConditions::BigAgentFarCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	AgarioAgent* pBigEnemy;
	std::vector<AgarioAgent*>* pAgentsVec;
	float searchRadius;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return false; 
	if (pBlackboard->GetData("NearestAgent", pBigEnemy) == false || pBigEnemy == nullptr)
		return false;
	if (pBlackboard->GetData("AllAgents", pAgentsVec) == false || pAgentsVec == nullptr)
		return false;
	if (pBlackboard->GetData("SearchRadius", searchRadius) == false)
		return false;

	searchRadius += pAgent->GetRadius();

	Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, Color{ 1, 0, 0 }, DEBUGRENDERER2D->NextDepthSlice());

	const float agentRadius{ pAgent->GetRadius() };

	for (AgarioAgent* otherAgent : *pAgentsVec)
	{
		if (otherAgent == pBigEnemy)
		{
			const float otherRadius{ otherAgent->GetRadius() };
			const float distance{ otherAgent->GetPosition().Distance(agentPos) - otherRadius };
			if (distance < searchRadius)
			{
				return false;
			}
		}
	}

	std::cout << "stop flee\n";
	return true;
}

bool FSMConditions::SmallAgentNearCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioAgent*>* pAgentsVec;
	float searchRadius;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return false;
	if (pBlackboard->GetData("AllAgents", pAgentsVec) == false || pAgentsVec == nullptr)
		return false;
	if (pBlackboard->GetData("SearchRadius", searchRadius) == false)
		return false;

	searchRadius += pAgent->GetRadius();

	Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, Color{ 1, 0, 0 }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestSmallAgent = nullptr;
	float closestDistance = FLT_MAX;
	const float agentRadius{ pAgent->GetRadius() };

	for (AgarioAgent* otherAgent : *pAgentsVec)
	{
		const float otherRadius{ otherAgent->GetRadius() };
		if (otherRadius + 1 < agentRadius)
		{
			const float distance{ otherAgent->GetPosition().Distance(agentPos) - otherRadius };
			if (distance < searchRadius && distance < closestDistance)
			{
				closestDistance = distance;
				closestSmallAgent = otherAgent;
			}
		}
	}

	if (closestSmallAgent == nullptr)
		return false;

	pBlackboard->ChangeData("NearestAgent", closestSmallAgent);
	std::cout << "pursue agent\n";
	return true;
}

bool FSMConditions::SmallAgentFarCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	AgarioAgent* pSmallEnemy;
	std::vector<AgarioAgent*>* pAgentsVec;
	float searchRadius;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return false;
	if (pBlackboard->GetData("NearestAgent", pSmallEnemy) == false || pSmallEnemy == nullptr)
		return false;
	if (pBlackboard->GetData("AllAgents", pAgentsVec) == false || pAgentsVec == nullptr)
		return false;
	if (pBlackboard->GetData("SearchRadius", searchRadius) == false)
		return false;

	searchRadius += pAgent->GetRadius();

	Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, Color{ 1, 0, 0 }, DEBUGRENDERER2D->NextDepthSlice());

	const float agentRadius{ pAgent->GetRadius() };

	for (AgarioAgent* otherAgent : *pAgentsVec)
	{
		if (otherAgent == pSmallEnemy)
		{
			const float otherRadius{ otherAgent->GetRadius() };
			const float distance{ otherAgent->GetPosition().Distance(agentPos) - otherRadius };
			if (distance < searchRadius)
			{
				return false;
			}
		}
	}

	std::cout << "stop pursue\n";
	return true;
}
