#pragma once

#include "stdafx.h"
#include "EFiniteStateMachine.h"
#include "EBlackboard.h"
#include "Steering.h"

//------------
//---STATES---
//------------
namespace FSMStates
{
	class ExploringState : public Elite::FSMState
	{
	public:
		ExploringState();
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class EnterHouseState : public Elite::FSMState
	{
	public:
		EnterHouseState();
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;

	private:
		int indexCurrHouse{ 0 };
	};

	class ItemHandlingState : public Elite::FSMState
	{
	public:
		ItemHandlingState();
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class HealingState : public Elite::FSMState
	{
	public:
		HealingState();
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class RunFromPurgeZoneState : public Elite::FSMState
	{
	public:
		RunFromPurgeZoneState();
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class EnemyHandlingState : public Elite::FSMState
	{
	public:
		EnemyHandlingState();
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};
}

//-----------------
//---TRANSITIONS---
//-----------------
namespace FSMConditions
{
	class FoundHouseCondition : public Elite::FSMCondition
	{
	public:
		FoundHouseCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class HouseSearchDoneCondition : public Elite::FSMCondition
	{
	public:
		HouseSearchDoneCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class ItemFoundCondition : public Elite::FSMCondition
	{
	public:
		ItemFoundCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class ItemHandlingDoneCondition : public Elite::FSMCondition
	{
	public:
		ItemHandlingDoneCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NeedHealingCondition : public Elite::FSMCondition
	{
	public:
		NeedHealingCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoNeedHealingCondition : public Elite::FSMCondition
	{
	public:
		NoNeedHealingCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class PurgeZoneCondition : public Elite::FSMCondition
	{
	public:
		PurgeZoneCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoPurgeZoneCondition : public Elite::FSMCondition
	{
	public:
		NoPurgeZoneCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class WasAttackedCondition : public Elite::FSMCondition
	{
	public:
		WasAttackedCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class EnemyGoneCondition : public Elite::FSMCondition
	{
	public:
		EnemyGoneCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};
}

