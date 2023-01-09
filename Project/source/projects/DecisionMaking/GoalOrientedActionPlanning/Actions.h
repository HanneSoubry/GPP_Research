#ifndef ELITE_APPLICATION_GOAP_BEHAVIORS
#define ELITE_APPLICATION_GOAP_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteGoalOrientedActionPlanning/EGoalOrientedActionPlanning.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Actions
//-----------------------------------------------------------------
using namespace Elite;

namespace GOAP_Actions
{
	// TODO: define action classes

	class GOAPActionWander final : public GoapAction
	{
	public:
		explicit GOAPActionWander();
		virtual ~GOAPActionWander();

		virtual BehaviorState ExecuteAction(Blackboard * pBlackBoard) override;
		virtual bool AbortAction(Blackboard* pBlackBoard) override;

		virtual int GetValue(Blackboard * pBlackBoard) override;
		virtual bool IsPreconditionTrue(Blackboard * pBlackBoard) override;
		virtual void GetRequiredPosInfo(Blackboard * pBlackBoard, MoveTargetType & moveTargetType) override;

	private:
	};

	class GOAPActionEatFood final : public GoapAction
	{
	public:
		explicit GOAPActionEatFood();
		virtual ~GOAPActionEatFood();

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard) override;
		virtual bool AbortAction(Blackboard* pBlackBoard) override;

		virtual int GetValue(Blackboard* pBlackBoard) override;
		virtual bool IsPreconditionTrue(Blackboard* pBlackBoard) override;
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType) override;

	private:
	};

	class GOAPActionEatEnemy final : public GoapAction
	{
	public:
		explicit GOAPActionEatEnemy();
		virtual ~GOAPActionEatEnemy();

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard) override;
		virtual bool AbortAction(Blackboard* pBlackBoard) override;

		virtual int GetValue(Blackboard* pBlackBoard) override;
		virtual bool IsPreconditionTrue(Blackboard* pBlackBoard) override;
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType) override;

	private:
	};

	class GOAPActionRunFromEnemy final : public GoapAction
	{
	public:
		explicit GOAPActionRunFromEnemy();
		virtual ~GOAPActionRunFromEnemy();

		virtual BehaviorState ExecuteAction(Blackboard* pBlackBoard) override;
		virtual bool AbortAction(Blackboard* pBlackBoard) override;

		virtual int GetValue(Blackboard* pBlackBoard) override;
		virtual bool IsPreconditionTrue(Blackboard* pBlackBoard) override;
		virtual void GetRequiredPosInfo(Blackboard* pBlackBoard, MoveTargetType& moveTargetType) override;

	private:
	};
}

#endif