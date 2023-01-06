/*=============================================================================*/
// Implementation of a GOAP
/*=============================================================================*/
#ifndef ELITE_GOAL_ORIENTED_ACTION_PLANNING
#define ELITE_GOAL_ORIENTED_ACTION_PLANNING

//--- Includes ---
#include "framework/EliteAI/EliteData/EBlackboard.h"
#include "projects/Shared/Agario/AgarioAgent.h"
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

	//-----------------------------------------------------------------
	// GOAP ACTION (IBehavior)
	//-----------------------------------------------------------------
	class GoapAction : public IBehavior
	{
	public:
		explicit GoapAction(std::function<BehaviorState(Blackboard*)> fp) : m_fpAction(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<BehaviorState(Blackboard*)> m_fpAction = nullptr;
	};

	//-----------------------------------------------------------------
	// GOAP (BASE)
	//-----------------------------------------------------------------
	class GoapBehavior final : public Elite::IDecisionMaking
	{		// behavior root (similar to fsm with 3 states)
	public:
		explicit GoapBehavior(Blackboard* pBlackBoard)
			: m_pBlackBoard(pBlackBoard) {};
		~GoapBehavior()
		{
			SAFE_DELETE(m_pBlackBoard); //Takes ownership of passed blackboard!
		};

		virtual void Update(float deltaTime) override;

		Blackboard* GetBlackboard() const { return m_pBlackBoard; }

	private:
		void MakePlan();
		void DoAction();
		void MoveTo();
		void FindNearbyThings();
		bool LargerEnemyNearby();

		GoapState m_CurrentState = GoapState::MakePlan;
		Blackboard* m_pBlackBoard = nullptr;
		// TODO: add pot of actions to blackboard

		// Plan variables
		std::stack<GoapAction*> m_CurrentPlan{};
		
		// refresh the plan and check environment every ... seconds
		float m_ActionTimer{};
		float m_MaxActionTimer{2};

		// Move variables
		bool m_ToAgent = false;
		bool m_RunFromAgent = false;
	};
}
#endif