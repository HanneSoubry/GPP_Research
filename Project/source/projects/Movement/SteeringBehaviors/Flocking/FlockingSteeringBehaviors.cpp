#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Seek::SetTarget(m_pFlock->GetAverageNeighborPos());
	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 0, 1 }, 0);
	}

	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	int nrNeighbors{ m_pFlock->GetNrOfNeighbors() };
	if (nrNeighbors > 0)
	{
		std::vector<Elite::Vector2> vectorsFromNeighbors;
		std::vector<SteeringAgent*> flock{ m_pFlock->GetNeighbors() };

		for (int i{}; i < nrNeighbors; i++)
		{
			Elite::Vector2 tempVector{ pAgent->GetPosition().x - flock[i]->GetPosition().x,
										pAgent->GetPosition().y - flock[i]->GetPosition().y };

			float distance{ tempVector.Magnitude() };
			tempVector.Normalize();
			tempVector *= pAgent->GetMaxLinearSpeed() * (1.f - (distance / m_pFlock->GetNeighborhoodRadius()));		
															// distance / radius => waarde tussen 0 en 1

			vectorsFromNeighbors.push_back(tempVector);
		}

		for (int i{}; i < nrNeighbors; ++i)
		{
			steering.LinearVelocity.x += vectorsFromNeighbors[i].x;
			steering.LinearVelocity.y += vectorsFromNeighbors[i].y;
		}

		steering.LinearVelocity.x /= nrNeighbors;
		steering.LinearVelocity.y /= nrNeighbors;

		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

		if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 1, 0, 1 });
		}
	}
	else
	{
		steering.LinearVelocity = { 0, 0 };
	}

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	if (m_pFlock->GetNrOfNeighbors() > 0)
	{
		steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();

		if (pAgent->CanRenderBehavior())
		{
			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 1 });
		}
	}
	else
	{
		steering.LinearVelocity = { 0,0 };
	}

	return steering;
}
