#pragma once

#include "stdafx.h"
#include "Exam_HelperStructs.h"
#include "EGoalOrientedActionPlanning.h"
#include "EBlackboard.h"
#include <deque>

using namespace Elite;

namespace GOAP_Actions
{
	// TODO: define action classes

	class GOAPActionFindCheckpoint final : public GoapAction
	{
	public:
		explicit GOAPActionFindCheckpoint();
		virtual ~GOAPActionFindCheckpoint() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);
		virtual int GetValue(Blackboard* pBlackBoard);

		// Keep default
		// --------------
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }

	private:
		bool m_FirstTimeCalled{ true };
		bool m_FindHouses{ false };
		void CalculateCheckPoints(Blackboard* pBlackBoard);
		std::deque<Vector2> m_Checkpoints{};
	};

	class GOAPActionSearchHouse final : public GoapAction
	{
	public:
		explicit GOAPActionSearchHouse();
		virtual ~GOAPActionSearchHouse() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }

	private:
		bool m_StartNewHouse{ true };
		void CalculateCheckPoints(Blackboard* pBlackBoard);
		std::deque<Vector2> m_Checkpoints{};
	};

	class GOAPActionLookAround final : public GoapAction
	{
	public:
		explicit GOAPActionLookAround();
		virtual ~GOAPActionLookAround() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }

	private:
		bool m_FirstTimeCalled{ true };
		void CalculateCheckPoints(Blackboard* pBlackBoard);
		std::deque<Vector2> m_Checkpoints{};
	};

	class GOAPActionDestroyGarbage final : public GoapAction
	{
	public:
		explicit GOAPActionDestroyGarbage();
		virtual ~GOAPActionDestroyGarbage() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }
	private:
		bool m_TargetSet{ false };
		Vector2 m_TargetPos{};
	};

	class GOAPActionChoseItem final : public GoapAction
	{
	public:
		explicit GOAPActionChoseItem();
		virtual ~GOAPActionChoseItem() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }
	private:
		bool m_TargetSet{ false };
		Vector2 m_TargetPos{};
	};

	class GOAPActionFindItem final : public GoapAction
	{
	public:
		explicit GOAPActionFindItem();
		virtual ~GOAPActionFindItem() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }
	private:
		bool m_TargetSet{ false };
		Vector2 m_TargetPos{};
	};

	class GOAPActionHeal final : public GoapAction
	{
	public:
		explicit GOAPActionHeal();
		virtual ~GOAPActionHeal() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);

		// Keep default
		// --------------
		//virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }
	};

	class GOAPActionEscapePurgeZone final : public GoapAction
	{
	public:
		explicit GOAPActionEscapePurgeZone();
		virtual ~GOAPActionEscapePurgeZone() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }
	private:
		bool m_TargetSet{ false };
		Vector2 m_TargetPos{};
		float m_Distance{};
	};

	class GOAPActionRunFromEnemy final : public GoapAction
	{
	public:
		explicit GOAPActionRunFromEnemy();
		virtual ~GOAPActionRunFromEnemy() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }
	private:
		bool m_TargetSet{ false };
		Vector2 m_TargetPos{};
		Vector2 m_TargetVel{};
	};

	class GOAPActionShootEnemy final : public GoapAction
	{
	public:
		explicit GOAPActionShootEnemy();
		virtual ~GOAPActionShootEnemy() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);

		// Keep default
		// --------------
		//virtual int GetValue(Blackboard* pBlackBoard);
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }
	private:
		bool m_TargetSet{ false };
		Vector2 m_TargetPos{};
		Vector2 m_TargetVel{};
	};

	class GOAPActionFindEnemy final : public GoapAction
	{
	public:
		explicit GOAPActionFindEnemy();
		virtual ~GOAPActionFindEnemy() = default;

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard);
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveInfo& moveInfo);
		virtual int GetValue(Blackboard* pBlackBoard);

		// Keep default
		// --------------
		//virtual const std::vector<GoapEffect>& GetPreconditions(Blackboard* pBlackBoard) { return m_PreconditionsVec; }
		//virtual const std::vector<GoapEffect>& GetEffects(Blackboard* pBlackBoard) { return m_EffectsVec; }
		//bool RequiresPosition() { return m_PositionRequired; }

	private:
		bool m_FirstTimeCalled{ true };
		void CalculateCheckPoints(Blackboard* pBlackBoard);
		std::deque<Vector2> m_Checkpoints{};
	};
}