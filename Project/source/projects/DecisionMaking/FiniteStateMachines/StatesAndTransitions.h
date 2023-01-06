/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------
namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class FleeAgentState : public Elite::FSMState
	{
	public:
		FleeAgentState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	};

	class PursueAgentState : public Elite::FSMState
	{
	public:
		PursueAgentState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	};
}

//-----------------
//---TRANSITIONS---
//-----------------
namespace FSMConditions
{
	class FoodNearByCondition : public Elite::FSMCondition
	{
	public:
		FoodNearByCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class TargetFoodGoneCondition : public Elite::FSMCondition
	{
	public:
		TargetFoodGoneCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BigAgentNearCondition : public Elite::FSMCondition
	{
	public:
		BigAgentNearCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BigAgentFarCondition : public Elite::FSMCondition
	{
	public:
		BigAgentFarCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallAgentNearCondition : public Elite::FSMCondition
	{
	public:
		SmallAgentNearCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallAgentFarCondition : public Elite::FSMCondition
	{
	public:
		SmallAgentFarCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};
}

#endif