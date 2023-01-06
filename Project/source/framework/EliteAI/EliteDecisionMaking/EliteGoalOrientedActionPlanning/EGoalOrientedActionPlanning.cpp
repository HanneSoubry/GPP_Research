//=== General Includes ===
#include "stdafx.h"
#include "EGoalOrientedActionPlanning.h"
using namespace Elite;

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
		MakePlan();
		m_CurrentState = GoapState::DoAction;
		m_ActionTimer = 0;
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

void GoapBehavior::MakePlan()
{
	// TODO: function MakePlan
}

void GoapBehavior::DoAction()
{
	// TODO: function DoAction
}

void GoapBehavior::MoveTo()
{
	// TODO: function MoveTo
}

void GoapBehavior::FindNearbyThings()
{
	// TODO: function FindNearbyThings
}

bool GoapBehavior::LargerEnemyNearby()
{
	AgarioAgent* pAgent;

	if (m_pBlackBoard->GetData("Agent", pAgent) == false || pAgent == nullptr)
		return false;

	// TODO: function LargerEnemyNearby
}


BehaviorState GoapAction::Execute(Blackboard* pBlackBoard)
{
	if (m_fpAction == nullptr)
		return BehaviorState::Failure;

	m_CurrentState = m_fpAction(pBlackBoard);
	return m_CurrentState;
}