/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return Elite::BehaviorState::Failure;

		pAgent->SetToWander();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekFood(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 targetPos{};

		if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return Elite::BehaviorState::Failure;
		if (pBlackboard->GetData("Target", targetPos) == false)
			return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(targetPos);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToFleeEnemy(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		AgarioAgent* pBigEnemy;

		if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return Elite::BehaviorState::Failure;
		if (pBlackboard->GetData("AgentTarget", pBigEnemy) == false || pBigEnemy == nullptr)
			return Elite::BehaviorState::Failure;

		const Elite::Vector2 enemyPos{ pBigEnemy->GetPosition() };
		const float fleeRadius{ pAgent->GetPosition().Distance(enemyPos) + 1 };

		pAgent->SetToFlee(enemyPos, fleeRadius);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekEnemy(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		AgarioAgent* pSmallEnemy;

		if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return Elite::BehaviorState::Failure;
		if (pBlackboard->GetData("AgentTarget", pSmallEnemy) == false || pSmallEnemy == nullptr)
			return Elite::BehaviorState::Failure;

		const Elite::Vector2 enemyPos{ pSmallEnemy->GetPosition() };
		pAgent->SetToSeek(enemyPos);

		return Elite::BehaviorState::Success;
	}
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
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

		if (pFoodVec->empty())
			return false;

		searchRadius += pAgent->GetRadius();

		float closestDistSq{searchRadius * searchRadius};
		AgarioFood* pClosestFood{ nullptr };
		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pFood : *pFoodVec)
		{
			const float distSq{ pFood->GetPosition().DistanceSquared(agentPos) };

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				pClosestFood = pFood;
			}
		}

		if (pClosestFood == nullptr)
			return false;

		pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
		return true;
	}

	bool IsBigAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pOtherAgents;
		float searchRadius;

		if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return false;
		if (pBlackboard->GetData("AgentsVec", pOtherAgents) == false || pOtherAgents == nullptr)
			return false;
		if (pBlackboard->GetData("SearchRadius", searchRadius) == false)
			return false;

		if (pOtherAgents->empty())
			return false;

		const float agentRadius{ pAgent->GetRadius() };
		searchRadius += agentRadius;

		float closestDist{ searchRadius };
		AgarioAgent* pClosestEnemy{ nullptr };
		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, { 1,1,0 }, DEBUGRENDERER2D->NextDepthSlice());

		for (auto& pOther : *pOtherAgents)
		{
			const float otherRadius{ pOther->GetRadius() };
			if (otherRadius > agentRadius + 1)	// bigger agent
			{
				const float dist{ pOther->GetPosition().Distance(agentPos) - otherRadius };

				if (dist < closestDist)
				{
					closestDist = dist;
					pClosestEnemy = pOther;
				}
			}
		}

		if (pClosestEnemy == nullptr)
			return false;

		DEBUGRENDERER2D->DrawCircle(agentPos, closestDist, { 1,0,0 }, DEBUGRENDERER2D->NextDepthSlice());
		pBlackboard->ChangeData("AgentTarget", pClosestEnemy);
		return true;
	}

	bool IsSmallAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pOtherAgents;
		float searchRadius;

		if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return false;
		if (pBlackboard->GetData("AgentsVec", pOtherAgents) == false || pOtherAgents == nullptr)
			return false;
		if (pBlackboard->GetData("SearchRadius", searchRadius) == false)
			return false;

		if (pOtherAgents->empty())
			return false;

		const float agentRadius{ pAgent->GetRadius() };
		searchRadius += agentRadius;

		float closestDist{ searchRadius };
		AgarioAgent* pClosestEnemy{ nullptr };
		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, { 1,1,0 }, DEBUGRENDERER2D->NextDepthSlice());

		for (auto& pOther : *pOtherAgents)
		{
			const float otherRadius{ pOther->GetRadius() };
			if (otherRadius + 1 < agentRadius)	// smaller agent
			{
				const float dist{ pOther->GetPosition().Distance(agentPos) - otherRadius };

				if (dist < closestDist)
				{
					closestDist = dist;
					pClosestEnemy = pOther;
				}
			}
		}

		if (pClosestEnemy == nullptr)
			return false;

		DEBUGRENDERER2D->DrawCircle(agentPos, closestDist, { 0,1,0 }, DEBUGRENDERER2D->NextDepthSlice());
		pBlackboard->ChangeData("AgentTarget", pClosestEnemy);
		return true;
	}
}












#endif