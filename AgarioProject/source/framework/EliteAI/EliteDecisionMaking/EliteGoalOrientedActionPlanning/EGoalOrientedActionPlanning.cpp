//=== General Includes ===
#include "stdafx.h"
#include "EGoalOrientedActionPlanning.h"
#include <memory>

using namespace Elite;

GoapBehavior::GoapBehavior(Blackboard* pBlackBoard)
	: m_pBlackBoard(pBlackBoard)
{
	auto pNearbyAgents{new std::vector<AgarioAgent*> };
	auto pNearbyFood{ new std::vector<AgarioFood*> };

	m_pBlackBoard->AddData("NearbyAgents", pNearbyAgents);
	m_pBlackBoard->AddData("NearbyFood", pNearbyFood);
}

GoapBehavior::~GoapBehavior()
{
	// pNearbyAgents and pNearbyFood point to objects owned (and deleted) by App_AgarioGame

	if (!m_ActionsVec.empty())
	{
		for (GoapAction* currAction : m_ActionsVec)
		{
			delete currAction;
		}

		m_ActionsVec.clear();
	}

	SAFE_DELETE(m_pBlackBoard); //Takes ownership of passed blackboard!
}

void GoapBehavior::Update(float deltaTime)
{
	FindNearbyThings();
	if (LargerEnemyNearby())
	{	// if enemy nearby is larger => make a new plan (to escape)
		m_CurrentState = GoapState::MakePlan;
	}
	else
	{
		// world changes => every while, check environment and adjust plan
		m_ActionTimer += deltaTime;
		if(m_ActionTimer >= m_MaxActionTimer)
			m_CurrentState = GoapState::MakePlan;
	}

	switch (m_CurrentState)
	{
	case Elite::GoapState::MakePlan:
		if (MakePlan())
		{
			m_CurrentState = GoapState::DoAction;
			m_ActionTimer = 0;
		}
		break;

	case Elite::GoapState::DoAction:
		DoAction();
		m_ActionTimer += deltaTime;
		break;

	case Elite::GoapState::MoveTo:
		MoveTo();
		m_ActionTimer += deltaTime;
		break;
	}
}

void Elite::GoapBehavior::AddAction(GoapAction* action)
{
	bool actionAlreadyExists{ false };
	for (const GoapAction* currAction : m_ActionsVec)
	{
		if (currAction == action)	// overloaded operator== comparing type of the action
		{
			actionAlreadyExists = true;
			break;
		}
	}

	// avoid 2 equal actions
	if (!actionAlreadyExists)
		m_ActionsVec.push_back(action);
}

void Elite::GoapBehavior::RemoveAction(GoapAction* action)
{
	if (m_ActionsVec.size() == 0)
		return;	// nothing to remove
	
	int count{ 0 };
	int lastIndex{ static_cast<int>(m_ActionsVec.size() - 1) };

	for (const GoapAction* currAction : m_ActionsVec)
	{
		if (currAction == action)	// overloaded operator== comparing type of the action
		{
			if (count != lastIndex)	// last element
			{
				m_ActionsVec[count] = m_ActionsVec[lastIndex];
			}

			m_ActionsVec.pop_back();
			return;	// action found and removed
		}

		++count;
	}
}

bool GoapBehavior::MakePlan()
{
	const int nrActions{ static_cast<int>(m_ActionsVec.size()) };

	if (nrActions == 0)
	{	// no actions should not happen !!!
		AgarioAgent* pAgent;
		if (m_pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return false;

		pAgent->SetToWander();
		return false;
	}

	// goal == more score, only action value counts
	std::vector< std::stack<GoapAction*> > possiblePlansVec{};
	std::vector<int> planValues{};

	// there will never be more plans than actions
	possiblePlansVec.reserve(nrActions);
	planValues.reserve(nrActions);

	int count{ 0 };

	for (GoapAction* currAction : m_ActionsVec)
	{
		if (currAction->IsPreconditionTrue(m_pBlackBoard))
		{	
			// in this game preconditions (e.g. size of enemy) cannot be changed by actions 
			// no sequence is needed, just choose the one best action

			// first run through actions => make new stack and value
			std::stack<GoapAction*> stack{};
			stack.push(currAction);
			possiblePlansVec.push_back( stack );

			planValues.push_back(currAction->GetValue(m_pBlackBoard));
		}

		++count;
	}

	if (possiblePlansVec.empty())
	{
		// no plan should not happen !!!
		std::cout << "No plan possible\n";
		AgarioAgent* pAgent;
		if (m_pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
			return false;

		pAgent->SetToWander();
		return false;
	}

	int maxValue{ 0 };
	int maxValueIndex{ 0 };
	count = 0;

	for (int currValue : planValues)
	{
		if (currValue > maxValue)
		{
			maxValue = currValue;
			maxValueIndex = count;
		}

		++count;
	}

	m_CurrentState = GoapState::DoAction;
	m_CurrentPlan = possiblePlansVec[maxValueIndex];

	return true;
}

void GoapBehavior::DoAction()
{
	GoapAction* currentAction = m_CurrentPlan.top();

	if (currentAction->AbortAction(m_pBlackBoard))
	{
		m_CurrentState = GoapState::MakePlan;
	}

	if (m_MoveTargetReached || !currentAction->RequiresPosition())
	{	// if target reached or no position required
		switch (currentAction->ExecuteAction(m_pBlackBoard))
		{
		case BehaviorState::Success:
			m_CurrentPlan.pop();
			m_MoveTargetReached = false;
			if (m_CurrentPlan.empty())
				m_CurrentState = GoapState::MakePlan;
			break;
		case BehaviorState::Failure:
			m_CurrentState = GoapState::MakePlan;
			break;
		// case Running: just continue
		}
	}
	else
	{
		m_CurrentState = GoapState::MoveTo;
		currentAction->GetRequiredPosInfo(m_pBlackBoard, m_MoveTargetType);
	}
}

void GoapBehavior::MoveTo()
{
	AgarioAgent* pAgent;
	if (m_pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return;		// if no agent exists => quick exit

	if (m_CurrentPlan.top()->AbortAction(m_pBlackBoard))
	{
		m_CurrentState = GoapState::MakePlan;
		return;
	}

	switch (m_MoveTargetType)
	{
	case MoveTargetType::toAgent:
		{
			AgarioAgent* pEnemy;
			if (m_pBlackBoard->GetData("TargetAgent", pEnemy) == false || pEnemy == nullptr)
			{	// if no enemy available
				m_CurrentState = GoapState::MakePlan;
				std::cout << "no move to agent available\n";
				return;
			}

			const Vector2 enemyPos{ pEnemy->GetPosition() };
			if (pAgent->GetPosition().DistanceSquared(enemyPos) <= powf(pAgent->GetRadius() + pEnemy->GetRadius(), 2))
			{	// agent hit the enemy
				m_MoveTargetReached = true;
				m_CurrentState = GoapState::DoAction;
				return;
			}

			pAgent->SetToSeek(enemyPos);
			return;
		}
	case MoveTargetType::fromAgent:
		{
			AgarioAgent* pEnemy;
			if (m_pBlackBoard->GetData("TargetAgent", pEnemy) == false || pEnemy == nullptr)
			{	// if no enemy available
				m_CurrentState = GoapState::MakePlan;
				std::cout << "no move from agent available\n";
				return;
			}

			float searchRadius;
			if (m_pBlackBoard->GetData("SearchRadius", searchRadius) == false)
				searchRadius = 0;

			const Vector2 enemyPos{ pEnemy->GetPosition() };
			if (pAgent->GetPosition().DistanceSquared(enemyPos) > powf(pAgent->GetRadius() + pEnemy->GetRadius() + searchRadius, 2) + 5)
			{	// enemy out of sight (+ small margin)
				m_MoveTargetReached = true;
				m_CurrentState = GoapState::DoAction;
				return;
			}

			pAgent->SetToFlee(enemyPos);
			return;
		}
	case MoveTargetType::toPosition:
		{
			Vector2 target;
			if (m_pBlackBoard->GetData("Target", target) == false)
			{	// if no target available
				m_CurrentState = GoapState::MakePlan;
				std::cout << "no move to position available\n";
				return;
			}

			if (m_StartedMovingToPosition)
			{
				if (pAgent->GetPosition().DistanceSquared(target) <= powf(pAgent->GetRadius(), 2))
				{
					m_MoveTargetReached = true;
					m_StartedMovingToPosition = false;	// reset value
					m_CurrentState = GoapState::DoAction;
				}
			}
			else
			{
				pAgent->SetToSeek(target);
				m_StartedMovingToPosition = true;
			}

			return;
		}
	case MoveTargetType::fromPosition:
	{
		Vector2 target;
		if (m_pBlackBoard->GetData("Target", target) == false)
		{	// if no target available
			m_CurrentState = GoapState::MakePlan;
			std::cout << "no move from position available\n";
			return;
		}

		float searchRadius;
		if (m_pBlackBoard->GetData("SearchRadius", searchRadius) == false)
			searchRadius = 0;

		if (m_StartedMovingToPosition)
		{
			if (pAgent->GetPosition().DistanceSquared(target) >= powf(pAgent->GetRadius() + searchRadius, 2))
			{
				m_MoveTargetReached = true;
				m_StartedMovingToPosition = false;	// reset value
				m_CurrentState = GoapState::DoAction;
			}
		}
		else
		{
			pAgent->SetToFlee(target);
			m_StartedMovingToPosition = true;
		}

		return;
	}
	}
}

void GoapBehavior::FindNearbyThings()
{
	AgarioAgent* pAgent;
	if (m_pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return;		// if no agent exists => quick exit

	// find vectors for nearby info
	std::vector<AgarioAgent*>* pNearbyAgents;
	std::vector<AgarioFood*>* pNearbyFood;

	if (m_pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
	{
		std::cout << "Warning: pNearbyAgents does not exist\n";
		return;
	}
	if (m_pBlackBoard->GetData("NearbyFood", pNearbyFood) == false || pNearbyFood == nullptr)
	{
		std::cout << "Warning: pNearbyFood does not exist\n";
		return;
	}

	// reset vectors
	pNearbyAgents->clear();
	pNearbyFood->clear();

	// find world info
	std::vector<AgarioAgent*>* pOtherAgents;
	std::vector<AgarioFood*>* pFood;
	float searchRadius;

	if (m_pBlackBoard->GetData("AgentsVec", pOtherAgents) == false)
		pOtherAgents = nullptr;
	if (m_pBlackBoard->GetData("FoodVec", pFood) == false)
		pFood = nullptr;
	if (m_pBlackBoard->GetData("SearchRadius", searchRadius) == false)
		searchRadius = 0;

	// get agent pos and total searchradius 
	searchRadius += pAgent->GetRadius();
	const Vector2 agentPos{ pAgent->GetPosition() };

	// check all other agents
	if (pOtherAgents != nullptr)
	{
		for (AgarioAgent* currAgent : *pOtherAgents)
		{
			// if closer than max searchradius => add to nearby-vector
			if (powf(searchRadius + currAgent->GetRadius(), 2) >= currAgent->GetPosition().DistanceSquared(agentPos))
			{
				pNearbyAgents->push_back(currAgent);
			}
		}
	}

	// check all food
	if (pFood != nullptr)
	{
		for (AgarioFood* currFood : *pFood)
		{
			// if closer than max searchradius => add to nearby-vector
			if (searchRadius * searchRadius >= currFood->GetPosition().DistanceSquared(agentPos))
			{
				pNearbyFood->push_back(currFood);
			}
		}
	}
}

bool GoapBehavior::LargerEnemyNearby()
{
	AgarioAgent* pAgent;
	if (m_pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return false;	// if no agent exists => quick exit

	std::vector<AgarioAgent*>* pNearbyAgents;
	if (m_pBlackBoard->GetData("NearbyAgents", pNearbyAgents) == false || pNearbyAgents == nullptr)
		return false;

	if (pNearbyAgents->size() == 0)	// if no agents nearby => no larger agents nearby
		return false;

	const float agentSize{ pAgent->GetRadius() + 1 };	// 1 size margin to be able to eat another agent
	for (AgarioAgent* currAgent : *pNearbyAgents)
	{
		if (currAgent->GetRadius() > agentSize)
			return true;
	}

	return false;
}

bool Elite::GoapAction::operator==(const GoapAction& rhs)
{
	return (this->m_ActionID == rhs.m_ActionID);
}
