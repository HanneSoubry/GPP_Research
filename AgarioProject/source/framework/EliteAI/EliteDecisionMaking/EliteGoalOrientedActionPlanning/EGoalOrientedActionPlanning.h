/*=============================================================================*/
// Implementation of a GOAP
/*=============================================================================*/
#ifndef ELITE_GOAL_ORIENTED_ACTION_PLANNING
#define ELITE_GOAL_ORIENTED_ACTION_PLANNING

//--- Includes ---
#include "framework/EliteAI/EliteData/EBlackboard.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include <stack>

namespace Elite
{
	//-----------------------------------------------------------------
	// HELPER
	//-----------------------------------------------------------------
	enum class GoapState	//	3 fsm states
	{
		MakePlan,
		DoAction,
		MoveTo	
	};

	enum class MoveTargetType
	{
		toAgent,
		fromAgent,
		toPosition,
		fromPosition
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
		virtual bool AbortAction(Blackboard* pBlackBoard) = 0;

		virtual int GetValue(Blackboard* pBlackBoard) = 0;
		virtual bool IsPreconditionTrue(Blackboard* pBlackBoard) = 0;	// preconditions are size of other agents and cannot be changed by any action
															// thus true or false is enough, otherwise you need to pass the condition 
															// and check with other actions to build a sequence

		bool RequiresPosition() { return m_PositionRequired; }
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType) = 0;
										// target stored on the blackboard
										
		// ... GetEffect()		// no side effects in this game, only score increases

		// equal ID is considered equal action
		bool operator==(const GoapAction& rhs);

	protected:
		bool m_PositionRequired = false;

		// ID for easier compare of actions
		int m_ActionID = 0;
	};

	//-----------------------------------------------------------------
	// GOAP (BASE)
	//-----------------------------------------------------------------
	class GoapBehavior final : public Elite::IDecisionMaking
	{		// behavior root (similar to fsm with 3 states)
	public:
		explicit GoapBehavior(Blackboard* pBlackBoard);
		~GoapBehavior();

		// no copy or move
		GoapBehavior(const GoapBehavior&) = delete;
		GoapBehavior& operator=(const GoapBehavior&) = delete;
		GoapBehavior(GoapBehavior&&) = delete;
		GoapBehavior& operator=(GoapBehavior&&) = delete;

		// functions
		virtual void Update(float deltaTime) override;

		void AddAction(GoapAction* action);
		void RemoveAction(GoapAction* action);
		Blackboard* GetBlackboard() const { return m_pBlackBoard; }

	private:
		bool MakePlan();
		void DoAction();
		void MoveTo();
		void FindNearbyThings();
		bool LargerEnemyNearby();

		GoapState m_CurrentState = GoapState::MakePlan;
		Blackboard* m_pBlackBoard = nullptr;

		// Plan variables
		std::vector<Elite::GoapAction*> m_ActionsVec{};
		std::stack<GoapAction*> m_CurrentPlan{};
		
		// refresh the plan and check environment every ... seconds
		float m_ActionTimer{0};
		float m_MaxActionTimer{1};

		// Move variables
		MoveTargetType m_MoveTargetType{ MoveTargetType::toPosition };
		bool m_MoveTargetReached{ false };
		bool m_StartedMovingToPosition{ false };
	};
}
#endif