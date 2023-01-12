
#include "stdafx.h"
#include "Exam_HelperStructs.h"
#include "EBlackboard.h"
#include "EGoalOrientedActionPlanning.h"
#include <map>

namespace Elite
{
	class FSMState
	{
	public:
		FSMState() = default;
		virtual ~FSMState()		
		{
			delete m_pGoapPlanner;
			delete m_pGoapExecutor;

			for (GoapAction* currAction : m_pGoapActions)
			{
				delete currAction;
			}
			m_pGoapActions.clear();
		};
	
		virtual void OnEnter(Blackboard* pBlackboard) {};
		virtual void OnExit(Blackboard* pBlackboard) {};
		virtual void Update(Blackboard* pBlackboard, float deltaTime);

	protected:
		GoapPlanner* m_pGoapPlanner;
		GoapExecutor* m_pGoapExecutor;
		std::vector<GoapAction*> m_pGoapActions{};
	};

	class FSMCondition
	{
	public:
		FSMCondition() = default;
		virtual ~FSMCondition() = default;
		virtual bool Evaluate(Blackboard* pBlackboard) const = 0;
	};

	class FiniteStateMachine final
	{
	public:
		FiniteStateMachine(FSMState* startState, Blackboard* pBlackboard);
		~FiniteStateMachine();
	
		void AddTransition(FSMState* startState, FSMState* toState, FSMCondition* transition);
		void Update(float deltaTime);
		Elite::Blackboard* GetBlackboard() const;
	
	private:
		void ChangeState(FSMState* newState);
	
		typedef std::pair<FSMCondition*, FSMState*> TransitionStatePair;
		typedef std::vector<TransitionStatePair> Transitions;
	
		std::map<FSMState*, Transitions> m_Transitions; //Key is the state, value are all the transitions for that current state 
		FSMState* m_pCurrentState;
		Blackboard* m_pBlackboard = nullptr; // takes ownership of the blackboard
	};
}