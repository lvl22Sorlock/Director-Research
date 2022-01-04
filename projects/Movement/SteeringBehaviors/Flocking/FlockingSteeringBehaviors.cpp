#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);
	Elite::Vector2 neighborhoodCenter{m_pFlock->GetAverageNeighborPos()};

	steering.LinearVelocity = neighborhoodCenter - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();

	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();



	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.0f, Elite::Color{ 0, 0, 1, 1 });

		for (int i = 0; i < m_pFlock->GetNrOfNeighbors(); ++i)
		{
			if (m_pFlock->GetNeighbors()[i])
			{
				DEBUGRENDERER2D->DrawCircle(m_pFlock->GetNeighbors()[i]->GetPosition(), 3.5f, Elite::Color{ 0.5f, 1.0f, 0.5f, 1 }, 0.f);
			}
		}
	}

	return steering;
}


//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 totalVelocity{};;
	pAgent->SetAutoOrient(true);
	float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };

	for (int i = 0; i < m_pFlock->GetNrOfNeighbors(); ++i)
	{
		if (m_pFlock->GetNeighbors()[i])
		{
			distance = (m_pFlock->GetNeighbors()[i]->GetPosition() - pAgent->GetPosition()).Magnitude();

			if (distance > 0)
			{
				totalVelocity += -(m_pFlock->GetNeighbors()[i]->GetPosition() - pAgent->GetPosition()) / (distance);
			}
			
		}
	}
	steering.LinearVelocity = totalVelocity;

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();



	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.0f, Elite::Color{ 1, 0, 0, 1 });

		for (int i = 0; i < m_pFlock->GetNrOfNeighbors(); ++i)
		{
			if (m_pFlock->GetNeighbors()[i])
			{
				DEBUGRENDERER2D->DrawCircle(m_pFlock->GetNeighbors()[i]->GetPosition(), 3.5f, Elite::Color{ 0.5f, 1.0f, 0.5f, 1 }, 0.f);
			}
		}
	}

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)

SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.0f, Elite::Color{ 1, 1, 1, 1 });

		for (int i = 0; i < m_pFlock->GetNrOfNeighbors(); ++i)
		{
			if (m_pFlock->GetNeighbors()[i])
			{
				DEBUGRENDERER2D->DrawCircle(m_pFlock->GetNeighbors()[i]->GetPosition(), 3.5f, Elite::Color{ 0.5f, 1.0f, 0.5f, 1 }, 0.f);
			}
		}
	}

	return steering;
}