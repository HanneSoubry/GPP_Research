
#include "stdafx.h"
#include "EGoalOrientedActionPlanning.h"
#include "Steering.h"
#include "IExamInterface.h"

using namespace Elite;

bool GoapAction::operator==(const GoapAction& rhs)
{
	// quicker check if actions are the same type
	return (this->m_ActionID == rhs.m_ActionID);
}

std::stack<GoapAction*>* GoapPlanner::MakePlan(Blackboard* pBlackBoard, GoapEffect goal, const std::vector<GoapEffect>& startState)
{
	const int nrActions{ static_cast<int>(m_ActionsVec.size()) };

	if (nrActions == 0)
	{	// no actions should not happen !!!
		std::cout << "GoapPlanner: Failed to make a plan: No actions.\n";
		return false;
	}

	// setup possible plans
	std::vector< std::stack<GoapAction*>* > possiblePlansVec{};
	std::vector<int> planValues{};

	for (GoapAction* currAction : m_ActionsVec)
	{
		// if action completes goal
		if (DoesCompleteGoal(currAction->GetEffects(pBlackBoard), goal))
		{
			// make stack for new plan
			std::stack<GoapAction*>* newPlan{ new std::stack<GoapAction*>{} };
			newPlan->push(currAction);
			int newPlanValue{ currAction->GetValue(pBlackBoard) };

			// remember preconditions of the first action
			std::vector<GoapEffect> preconditions{ newPlan->top()->GetPreconditions(pBlackBoard) };

			bool planFound{ true };

			// as long as there are unfulfilled preconditions, search another action to fulfill them
			while (DoesCompleteGoal(startState, preconditions) == false)
			{		
				const int lastIndex{ static_cast<int>(preconditions.size()) - 1 };
				// find fulfilling action
				GoapAction* nextAction{ FindActionForPrecondition(pBlackBoard, preconditions[lastIndex]) };

				if (nextAction != nullptr)
				{
					// action found
					newPlan->push(nextAction);
					newPlanValue += nextAction->GetValue(pBlackBoard);

					// precondition done
					preconditions.pop_back();

					// add preconditions of new action
					std::vector<GoapEffect> newPreconditions{ nextAction->GetPreconditions(pBlackBoard) };
					for (GoapEffect currEffect : newPreconditions)
					{
						preconditions.push_back(currEffect);
					}
				}
				else
				{
					// no action found for precondition
					planFound = false;
					break;	// out of while loop
				}
			}

			if (planFound)	// else trash plan and continue
			{
				// if plan found => add to possible plans
				possiblePlansVec.push_back(newPlan);
				planValues.push_back(newPlanValue);
			}
		}
	}

	if (possiblePlansVec.empty())
	{
		std::cout << "GoapPlanner: No plan found.\n";
		return nullptr;
	}
	else
	{
		int maxValue{ 0 };
		int maxValueIndex{ 0 };
		int count{ 0 };

		for (int currValue : planValues)
		{
			if (currValue > maxValue)
			{
				maxValue = currValue;
				maxValueIndex = count;
			}

			++count;
		}

		std::cout << "GoapPlanner: Succeeded making a plan.\n";
		return possiblePlansVec[maxValueIndex];
	}
}

void GoapPlanner::AddAction(GoapAction* action)
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

void GoapPlanner::RemoveAction(GoapAction* action)
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

	// this action did not exist so nothing to remove
}

bool GoapPlanner::DoesCompleteGoal(const std::vector<GoapEffect>& effects, GoapEffect goal)
{
	for (GoapEffect currEffect : effects)
	{
		if (currEffect == goal)
			return true;
	}

	return false;
}

bool Elite::GoapPlanner::DoesCompleteGoal(const std::vector<GoapEffect>& effects, const std::vector<GoapEffect>& goals)
{
	if (goals.empty())
		return true;	// if no goal, consider true

	for (GoapEffect currGoal : goals)
	{
		if (DoesCompleteGoal(effects, currGoal) == false)
			return false;
	}

	return true;
}

GoapAction* Elite::GoapPlanner::FindActionForPrecondition(Blackboard* pBlackBoard, GoapEffect precondition)
{
	for (GoapAction* currAction : m_ActionsVec)
	{
		if (DoesCompleteGoal(currAction->GetEffects(pBlackBoard), precondition))
		{
			return currAction;
		}
	}

	// nothing found
	return nullptr;
}

BehaviorState GoapExecutor::Update(Blackboard* pBlackBoard, float deltaT)
{
	// safety checks
	if (m_pPlan == nullptr)
		return BehaviorState::Failure;
	if (m_pPlan->empty())
		return BehaviorState::Failure;

	GoapAction* currentAction{ m_pPlan->top() };

	if (m_Moving)
	{
		// get info
		MoveInfo moveInfo{};
		if (pBlackBoard->GetData("MoveInfo", moveInfo) == false)
		{
			m_Moving = false;
			m_MoveTargetReached = false;
			std::cout << "GoapExecutor: Should move but moveInfo not found.\n";
		}
		AgentInfo agentInfo{};
		if (pBlackBoard->GetData("AgentInfo", agentInfo) == false)
		{
			std::cout << "GoapExecutor: Should move but no agent found.\n";
		}

		bool targetNotReached{ true };
		// check target reached
		if (moveInfo.targetType == MoveTargetType::seekAndFace)
		{
			Elite::Vector2 toTarget{ moveInfo.targetPos - agentInfo.Position };
			float differenceAngle{ atan2f(toTarget.y, toTarget.x) - agentInfo.Orientation };

			const float pi{ static_cast<float>(M_PI) };
			if (differenceAngle < -pi)
			{
				differenceAngle = differenceAngle + 2 * pi;
			}
			if (differenceAngle > pi)
			{
				differenceAngle = differenceAngle - 2 * pi;
			}

			if (abs(differenceAngle) <= moveInfo.acceptedDistance && 
				agentInfo.Position.DistanceSquared(moveInfo.targetPos) <= moveInfo.acceptedDistance * moveInfo.acceptedDistance)
			{
				m_MoveTargetReached = true;
				m_Moving = false;
				std::cout << "GoapExecutor: move target reached.\n";
				targetNotReached = false;
			}
		}
		else if (moveInfo.targetType == MoveTargetType::faceTo || moveInfo.targetType == MoveTargetType::fleeAndFace)
		{
			Elite::Vector2 toTarget{ moveInfo.targetPos - agentInfo.Position };
			float differenceAngle{ atan2f(toTarget.y, toTarget.x) - agentInfo.Orientation };

			const float pi{ static_cast<float>(M_PI) };
			if (differenceAngle < -pi)
			{
				differenceAngle = differenceAngle + 2 * pi;
			}
			if (differenceAngle > pi)
			{
				differenceAngle = differenceAngle - 2 * pi;
			}

			if (abs(differenceAngle) <= moveInfo.acceptedDistance)
			{
				m_MoveTargetReached = true;
				m_Moving = false;
				std::cout << "GoapExecutor: move target reached.\n";
				targetNotReached = false;
			}
		}
		else if (moveInfo.targetType == MoveTargetType::fromAgent)
		{
			bool run;
			if (pBlackBoard->GetData("ShouldRun", run) == false)
			{
				run = false;
			}

			if (run)
			{
				AgentInfo agent;
				if (pBlackBoard->GetData("AgentInfo", agent) == true)
				{
					if (agent.Stamina == 0)
					{
						m_MoveTargetReached = true;
						m_Moving = false;
						std::cout << "GoapExecutor: stop running.\n";
						targetNotReached = false;
					}
				}
			}
			else
			{
				if (agentInfo.Position.DistanceSquared(moveInfo.targetPos) <= moveInfo.acceptedDistance * moveInfo.acceptedDistance)
				{
					m_MoveTargetReached = true;
					m_Moving = false;
					std::cout << "GoapExecutor: move target reached.\n";
					targetNotReached = false;
				}
			}

		}
		else if (agentInfo.Position.DistanceSquared(moveInfo.targetPos) <= moveInfo.acceptedDistance * moveInfo.acceptedDistance)
		{
			m_MoveTargetReached = true;
			m_Moving = false;
			std::cout << "GoapExecutor: move target reached.\n";
			targetNotReached = false;
		}
		
		if(targetNotReached)
		{
			SteeringPlugin_Output steering{};
			IExamInterface* pInterface;
			if (pBlackBoard->GetData("Interface", pInterface) == false)
			{
				std::cout << "GoapExecutor: No interface found.\n";
			}

			switch (moveInfo.targetType)
			{
			case MoveTargetType::toAgent:
			{
				// TODO: when pursuit added, change move to agent
				Steering::Seek seek{};
				seek.SetTarget(pInterface->NavMesh_GetClosestPathPoint(moveInfo.targetPos));
				steering = seek.CalculateSteering(deltaT, agentInfo);
				break;
			}
			case MoveTargetType::fromAgent:
			{
				Steering::Evade evade{};
				evade.SetTarget(moveInfo.targetPos);
				evade.SetTargetVelocity(moveInfo.targetVelocity);
				steering = evade.CalculateSteering(deltaT, agentInfo);
				break;
			}
			case MoveTargetType::toPosition:
			{
				Steering::Seek seek{};
				seek.SetTarget(pInterface->NavMesh_GetClosestPathPoint(moveInfo.targetPos));
				steering = seek.CalculateSteering(deltaT, agentInfo);
				break;
			}
			case MoveTargetType::fromPosition:
			{
				Steering::Flee flee{};
				flee.SetTarget(pInterface->NavMesh_GetClosestPathPoint(moveInfo.targetPos));
				steering = flee.CalculateSteering(deltaT, agentInfo);
				break;
			}
			case MoveTargetType::faceTo:
			{
				Steering::Face face{};
				face.SetTarget(moveInfo.targetPos);
				steering = face.CalculateSteering(deltaT, agentInfo);
				break;
			}
			case MoveTargetType::seekAndFace:
			{
				Steering::SeekFace seekFace{};
				seekFace.SetTarget(moveInfo.targetPos);
				steering = seekFace.CalculateSteering(deltaT, agentInfo);
				break;
			}
			case MoveTargetType::fleeAndFace:
			{
				Steering::FleeFace fleeFace{};
				fleeFace.SetTarget(moveInfo.targetPos);
				steering = fleeFace.CalculateSteering(deltaT, agentInfo);
				break;
			}
			}

			pBlackBoard->ChangeData("SteeringOutput", steering);
		}
	}
	else
	{
		if (m_MoveTargetReached)
		{
			switch (currentAction->ExecuteAction(pBlackBoard))
			{
			case BehaviorState::Success:
				m_MoveTargetReached = false;
				m_pPlan->pop();

				if (m_pPlan->empty())
				{	
					// plan completed
					return BehaviorState::Success;
				}
				break;
			case BehaviorState::Failure:
				// something went wrong
				return BehaviorState::Failure;
				break;
			// case Running
				// repeat executeAction next frame
			}
		}
		else
		{	
			// move target not reached
			if (currentAction->RequiresPosition())
			{
				MoveInfo moveInfo{};
				currentAction->GetRequiredPosInfo(pBlackBoard, moveInfo);
				pBlackBoard->ChangeData("MoveInfo", moveInfo);
				m_Moving = true;
			}
			else
				m_MoveTargetReached = true;
		}
	}

	return BehaviorState::Running;
}

bool GoapExecutor::ContinuePlan(Blackboard* pBlackBoard)
{
	if (m_pPlan == nullptr)
		return false;
	if (m_pPlan->empty())
		return false;	
		// there was no previous plan
	
	GoapAction* nextAction{ m_pPlan->top() };

	if (nextAction->RequiresPosition())
	{
		MoveInfo moveInfo{};
		nextAction->GetRequiredPosInfo(pBlackBoard, moveInfo);
		pBlackBoard->ChangeData("MoveInfo", moveInfo);
		m_Moving = true;
	}
	else
		m_Moving = false;

	std::cout << "GoapExecutor: Continuing previous plan.\n";

	return true;	
	// ready to continue plan
}

void GoapExecutor::SetPlan(std::stack<GoapAction*>* newPlan)
{
	if (m_pPlan != nullptr)
		delete m_pPlan;

	m_pPlan = newPlan;

	std::cout << "GoapExecutor: New plan set.\n";
}

