#pragma once

#include "stdafx.h"
#include "Exam_HelperStructs.h"
#include "EBlackboard.h"
#include <stack>

namespace Elite
{
	//-----------------------------------------------------------------
	// HELPER
	//-----------------------------------------------------------------
	enum class MoveTargetType
	{
		toAgent,
		fromAgent,
		toPosition,
		fromPosition,
		faceTo,
		seekAndFace,
		fleeAndFace
	};

	struct MoveInfo
	{
		MoveTargetType targetType{};
		Vector2 targetPos{};
		Vector2 targetVelocity{};
		float acceptedDistance{};
	};

	enum class BehaviorState
	{
		Failure,
		Success,
		Running
	};

	enum class GoapEffect
	{
		// TODO: add goap effects and preconditions here

		ExploreMap,
		SearchHouse, 
		HouseAvailable,
		ItemFound,
		GarbageFound,
		HandleItem,
		UnknowItemFound,
		Heal,
		EscapePurge,
		SurviveEnemey,
		LoadedWeaponAvailable,
		EnemyVisible
	};

	//-----------------------------------------------------------------
	// GOAP ACTION
	//-----------------------------------------------------------------
	class GoapAction
	{
	public:
		explicit GoapAction() = default;
		virtual ~GoapAction() = default;

		// functions
		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard) = 0;

		virtual int GetValue(Blackboard* pBlackBoard) { return m_Value; }

		virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }

		bool RequiresPosition() { return m_PositionRequired; }
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo) {};

		// equal ID is considered equal action
		bool operator==(const GoapAction& rhs);

	protected:
		bool m_PositionRequired{ false };
		int m_Value{};

		// ID for easier compare of actions
		int m_ActionID{0};

		// preconditions & effects
		std::vector<GoapEffect> m_PreconditionsVec{};
		std::vector<GoapEffect> m_EffectsVec{};
	};

	class GoapPlanner
	{
	public:
		explicit GoapPlanner() = default;
		virtual ~GoapPlanner() = default;

		std::stack<GoapAction*>* MakePlan(Blackboard* pBlackBoard, GoapEffect goal, const std::vector<GoapEffect>& startState);

		void AddAction(GoapAction* action);
		void RemoveAction(GoapAction* action);

	private:
		bool DoesCompleteGoal(const std::vector<GoapEffect>& effects, GoapEffect goal);
		bool DoesCompleteGoal(const std::vector<GoapEffect>& effects, const std::vector<GoapEffect>& goals);

		GoapAction* FindActionForPrecondition(Blackboard* pBlackBoard, GoapEffect precondition);

		std::vector<GoapAction*> m_ActionsVec{};
	};

	class GoapExecutor
	{
	public:
		explicit GoapExecutor() = default;
		virtual ~GoapExecutor()
		{
			if (m_pPlan != nullptr)
				delete m_pPlan;
		};

		BehaviorState Update(Blackboard* pBlackBoard, float deltaT);

			// Use when returning from other state, checks location of the first action and adds a move if needed
		bool ContinuePlan(Blackboard* pBlackBoard);

			// takes ownership of the plan
		void SetPlan(std::stack<GoapAction*>* newPlan);

	private:

		std::stack<GoapAction*>* m_pPlan;
		bool m_Moving{ false };
		bool m_MoveTargetReached{ false };
	};
}