//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0 });
	}

	return steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 fromTarget = pAgent->GetPosition() - m_Target.Position;
	float distance = fromTarget.Magnitude();
	SteeringOutput steering{};

	if (distance > m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	float slowRadius{ 10.f };
	float arriveRadius{ 1.f };

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	float distance{ steering.LinearVelocity.Magnitude() };

	if (distance < arriveRadius)
	{
		steering.LinearVelocity = { 0,0 };
	}
	else
	{
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

		if (distance < slowRadius)
		{
			steering.LinearVelocity *= (distance/slowRadius);
		}
	}
	
	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	Elite::Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	float differenceAngle{ atan2f(toTarget.y, toTarget.x) - pAgent->GetRotation() };
	if (differenceAngle < -M_PI)
	{
		differenceAngle = differenceAngle + 2 * M_PI;
	}
	if (differenceAngle > M_PI)
	{
		differenceAngle = differenceAngle - 2 * M_PI;
	}
	float arriveAngle{ 0.1f };

	if (abs(differenceAngle) < arriveAngle)
	{
		steering.AngularVelocity = 0;
	}
	else if (differenceAngle > 0)
	{
		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	}
	else if (differenceAngle < 0)
	{
		steering.AngularVelocity = - pAgent->GetMaxAngularSpeed();
	}

	return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float maxAngleChangeDegrees(Elite::ToDegrees(m_MaxAngleChange));
	float angleChangeDegrees{ (rand() % (2 * static_cast<int>(maxAngleChangeDegrees) + 1)) - maxAngleChangeDegrees };
	m_WanderAngle += Elite::ToRadians(angleChangeDegrees);
	if (m_WanderAngle > 2 * M_PI)
	{
		m_WanderAngle -= 2 * M_PI;
	}
	else if (m_WanderAngle < -2 * M_PI)
	{
		m_WanderAngle += 2 * M_PI;
	}

	float directionAngle{ atan2f(pAgent->GetDirection().y, pAgent->GetDirection().x) };
	Elite::Vector2 circlePosition{ pAgent->GetPosition().x + m_OffsetDistance * cosf(directionAngle),
										 pAgent->GetPosition().y + m_OffsetDistance * sinf(directionAngle) };
	Elite::Vector2 targetPosition{ circlePosition.x + m_Radius * cosf(m_WanderAngle),
								   circlePosition.y + m_Radius * sinf(m_WanderAngle) };

	SteeringOutput steering = {};

	steering.LinearVelocity = targetPosition - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawCircle(circlePosition, m_Radius, Elite::Color{ 1,0,0 }, 0);
		//DEBUGRENDERER2D->DrawCircle(targetPosition, 0.5f, Elite::Color{ 0,1,0 }, 0);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, Elite::Color{ 1,1,0 });
	}

	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };
	Elite::Vector2 agentPosition{ pAgent->GetPosition() };

	if((m_Target.Position - agentPosition).Magnitude() < 0.5f)	// target reached -> return empty steering
	{
		return SteeringOutput();
	}

	float distance{ (m_Target.Position - agentPosition).Magnitude() };
	float time{ distance / pAgent->GetMaxLinearSpeed() };
	Elite::Vector2 predictedTarget{ m_Target.Position.x + m_Target.LinearVelocity.x * time, m_Target.Position.y + m_Target.LinearVelocity.y * time };
	Elite::Vector2 desiredDirection{ predictedTarget - agentPosition };

	SteeringOutput steering = {};
	steering.LinearVelocity = desiredDirection - agentVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//if (pAgent->CanRenderBehavior())
	//{
	//	DEBUGRENDERER2D->DrawPoint(predictedTarget, 2.f, Elite::Color{ 0, 0, 1 });
	//	DEBUGRENDERER2D->DrawDirection(agentPosition, steering.LinearVelocity, 5.f, Elite::Color{ 1, 0, 0 });
	//	DEBUGRENDERER2D->DrawDirection(agentPosition, desiredDirection, 15.f, Elite::Color{ 0, 0, 1 });
	//	DEBUGRENDERER2D->DrawDirection(agentPosition, agentVelocity, 10.f, Elite::Color{ 0, 1, 0 });
	//}

	return steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 agentPosition{ pAgent->GetPosition() };
	float distance{ (m_Target.Position - agentPosition).Magnitude() };

	if (distance > m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };

	float time{ distance / pAgent->GetMaxLinearSpeed() };
	Elite::Vector2 predictedTarget{ m_Target.Position.x + m_Target.LinearVelocity.x * time, m_Target.Position.y + m_Target.LinearVelocity.y * time };
	Elite::Vector2 desiredDirection{ - (predictedTarget - agentPosition) };

	steering.LinearVelocity = desiredDirection - agentVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
	//	DEBUGRENDERER2D->DrawPoint(predictedTarget, 2.f, Elite::Color{ 0, 0, 1 });
		DEBUGRENDERER2D->DrawDirection(agentPosition, steering.LinearVelocity, 5.f, Elite::Color{ 1, 0, 0 });
	//	DEBUGRENDERER2D->DrawDirection(agentPosition, desiredDirection, 15.f, Elite::Color{ 0, 0, 1 });
	//	DEBUGRENDERER2D->DrawDirection(agentPosition, agentVelocity, 10.f, Elite::Color{ 0, 1, 0 });
	}

	return steering;
}
