
#include "stdafx.h"
#include "Steering.h"

using namespace Steering;

///////////////////////////////////////
//SEEK
//****

SteeringPlugin_Output Seek::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering{};

	steering.LinearVelocity = m_TargetPosition - agent.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agent.MaxLinearSpeed;

	return steering;
}

///////////////////////////////////////
//SEEK & Face
//****
SteeringPlugin_Output SeekFace::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering{};

	// face to
	Face face {};
	face.SetTarget(m_TargetPosition);
	steering = face.CalculateSteering(deltaT, agent);

	// move to
	Elite::Vector2 toTarget{ m_TargetPosition - agent.Position };

	if (toTarget.Magnitude() <= m_Radius)	// close enough
		steering.LinearVelocity = { 0,0 };
	else
	{
		steering.LinearVelocity = toTarget;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= agent.MaxLinearSpeed;
	}

	return steering;
}

///////////////////////////////////////
//FLEE
//****
SteeringPlugin_Output Flee::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering{};

	steering.LinearVelocity = agent.Position - m_TargetPosition;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agent.MaxLinearSpeed;

	return steering;
}

///////////////////////////////////////
//FLEE & FACE
//****
SteeringPlugin_Output FleeFace::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering{};

	// face to
	Face face{};
	face.SetTarget(m_TargetPosition);
	steering = face.CalculateSteering(deltaT, agent);

	// move from
	Elite::Vector2 fromTarget{ agent.Position - m_TargetPosition };

	steering.LinearVelocity = fromTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agent.MaxLinearSpeed;

	return steering;
}

///////////////////////////////////////
//FACE
//****
SteeringPlugin_Output Face::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering{};
	steering.AutoOrient = false;

	Elite::Vector2 toTarget{ m_TargetPosition - agent.Position };
	float differenceAngle{ atan2f(toTarget.y, toTarget.x) - agent.Orientation };

	const float pi{ static_cast<float>(M_PI) };
	if (differenceAngle < -pi)
	{
		differenceAngle = differenceAngle + 2 * pi;
	}
	if (differenceAngle > pi)
	{
		differenceAngle = differenceAngle - 2 * pi;
	}

	if (differenceAngle > 0)
	{
		steering.AngularVelocity = agent.MaxAngularSpeed;
	}
	else if (differenceAngle < 0)
	{
		steering.AngularVelocity = -agent.MaxAngularSpeed;
	}

	return steering;
}

///////////////////////////////////////
//WANDER
//****
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	float maxAngleChangeDegrees(Elite::ToDegrees(m_MaxAngleChange));
	float angleChangeDegrees{ (rand() % (2 * static_cast<int>(maxAngleChangeDegrees) + 1)) - maxAngleChangeDegrees };

	const float pi{ static_cast<float>(M_PI) };
	m_WanderAngle += Elite::ToRadians(angleChangeDegrees);
	if (m_WanderAngle > 2 * pi)
	{
		m_WanderAngle -= 2 * pi;
	}
	else if (m_WanderAngle < -2 * pi)
	{
		m_WanderAngle += 2 * pi;
	}

	Elite::Vector2 agentDirection{ agent.LinearVelocity.GetNormalized() };
	float directionAngle{ atan2f(agentDirection.y, agentDirection.x) };

	Elite::Vector2 circlePosition{ agent.Position.x + m_OffsetDistance * cosf(directionAngle),
										 agent.Position.y + m_OffsetDistance * sinf(directionAngle) };
	Elite::Vector2 targetPosition{ circlePosition.x + m_Radius * cosf(m_WanderAngle),
								   circlePosition.y + m_Radius * sinf(m_WanderAngle) };

	SteeringPlugin_Output steering{};

	steering.LinearVelocity = targetPosition - agent.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agent.MaxLinearSpeed;

	return steering;
}

///////////////////////////////////////
//EVADE
//****
SteeringPlugin_Output Evade::CalculateSteering(float deltaT, const AgentInfo& agent)
{
	SteeringPlugin_Output steering = {};
	float distance{ (m_TargetPosition - agent.Position).Magnitude() };

	float time{ distance / agent.MaxLinearSpeed };
	Elite::Vector2 predictedTarget{ m_TargetPosition.x + m_TargetVelocity.x * time, m_TargetPosition.y + m_TargetVelocity.y * time };
	Elite::Vector2 desiredDirection{ -(predictedTarget - agent.Position) };

	steering.LinearVelocity = desiredDirection - agent.LinearVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agent.MaxLinearSpeed;

	return steering;
}

