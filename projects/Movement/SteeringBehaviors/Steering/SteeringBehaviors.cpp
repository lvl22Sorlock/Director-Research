//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"
#include <cmath>
#include <math.h>
#include "framework/EliteMath/EMath.h"


//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);
	
	const float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();



	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5.0f, Elite::Color{ 0, 1, 0, 1 });
	}

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);
	if (distanceToTarget > m_FleeRadius)
	{
		return SteeringOutput({0, 0}, 0.f, false);
	}
	
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);

	const float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	steering.LinearVelocity = -(m_Target.Position - pAgent->GetPosition());
	steering.LinearVelocity.Normalize();


	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();



	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5.0f, Elite::Color{ 0, 1, 0, 1 });
	}

	return steering;
}

void Flee::SetFleeRadius(float newFleeRadius)
{
	m_FleeRadius = newFleeRadius;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);

	//const float slowRadius{ 15.0f };

	const float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();


	if (distance < m_SlowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * distance / m_SlowRadius;
	}
	else
	{
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}


	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5.0f, Elite::Color{ 0, 1, 0, 1 });
	}

	return steering;
}

void Arrive::SetSlowRadius(float newSlowRadius)
{
	m_SlowRadius = newSlowRadius;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float pi{ float(E_PI) };
	const float playerAngleOffset{pi/2};
	const float maxAngleDifference{0.05f};
	const float minAngleSpeed{ pAgent->GetMaxAngularSpeed() / 5.0f};

	SteeringOutput steering = {};
	pAgent->SetAutoOrient(false);

	Elite::Vector2 vectorToTarget{ m_Target.Position - pAgent->GetPosition() };
	const float angleInRadians{ atan2(vectorToTarget.y, vectorToTarget.x) + playerAngleOffset };

	steering.AngularVelocity = pAgent->GetMaxAngularSpeed() * ClampAngle(angleInRadians - pAgent->GetRotation());


	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5.0f, Elite::Color{ 0, 1, 0, 1 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), vectorToTarget, 5.0f, Elite::Color{ 0, 0, 1, 1 });
	}

	return steering;
}




float Face::ClampAngle(float angle) const
{
	const float pi{ float(E_PI) };

	angle = fmodf(angle, 2 * pi);
	if (angle > pi)
	{
		angle -= 2*pi;
	}


	return angle;
}

//WANDER
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const float pi{ 3.1415927410125732421875f };
	const float agentLookDirection{pAgent->GetRotation() - pi/2};
	pAgent->SetAutoOrient(true);
	const float angleChangeMultiplier{ 15.00f };
	const Elite::Vector2 circleCenter
		{pAgent->GetPosition().x + (m_OffsetDistance * cosf(agentLookDirection)),
		 pAgent->GetPosition().y + (m_OffsetDistance * sinf(agentLookDirection)) };

	m_WanderAngle += deltaT *  angleChangeMultiplier * float(rand() % (2 * int(1000 * m_MaxAngleChange)) - int(1000 * m_MaxAngleChange)) / 1000.f;
	if (m_WanderAngle >= 2 * pi)
	{
		m_WanderAngle = fmodf(m_WanderAngle, 2 * pi);
	}

	Elite::Vector2 targetPos{circleCenter.x + (m_Radius * cosf(m_WanderAngle)),
							 circleCenter.y + (m_Radius * sinf(m_WanderAngle)) };

	steering.LinearVelocity = targetPos - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5.0f, Elite::Color{ 0, 1, 0, 1 });
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, Elite::Color{ 0, 0, 1, 1 }, 0.f);
		DEBUGRENDERER2D->DrawDirection
							(   pAgent->GetPosition(),
								Elite::Vector2
									{circleCenter.x + m_Radius*cosf(m_WanderAngle)
									,circleCenter.y + m_Radius * sinf(m_WanderAngle) },
								5.f,
								Elite::Color{ 1, 0, 0, 1 });
		DEBUGRENDERER2D->DrawCircle({ circleCenter.x + m_Radius * cosf(m_WanderAngle)
									,circleCenter.y + m_Radius * sinf(m_WanderAngle) },
			2.0f,
			Elite::Color{ 1, 0, 0, 1 },
			0.0f);
	}

	return steering;
}

//Pursuit
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);
	const float distance{Elite::DistanceSquared(m_Target.Position, pAgent->GetPosition())};
	float secTillTargetAtPos{ sqrtf(distance / m_Target.LinearVelocity.MagnitudeSquared()) };


	Elite::Vector2 futureTargetPos{m_Target.Position + m_Target.LinearVelocity* secTillTargetAtPos };

	steering.LinearVelocity = (futureTargetPos - pAgent->GetPosition());
	steering.LinearVelocity.Normalize();

	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();



	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5.0f, Elite::Color{ 0, 1, 0, 1 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), m_Target.Position - pAgent->GetPosition(), 5.0f, Elite::Color{ 0, 0, 1, 1 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), futureTargetPos - pAgent->GetPosition(), 5.0f, Elite::Color{ 1, 0, 0, 1 });
	}

	return steering;
}


//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	float distanceToTargetSquared = Elite::DistanceSquared(pAgent->GetPosition(), m_Target.Position);
	if (distanceToTargetSquared > m_EvadeRadius*m_EvadeRadius)
	{
		return SteeringOutput({ 0, 0 }, 0.f, false);
	}
	
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(true);

	
	const float distance{ Elite::DistanceSquared(m_Target.Position, pAgent->GetPosition()) };
	const float secTillTargetAtPos{ sqrtf(distance / m_Target.LinearVelocity.MagnitudeSquared()) };
	Elite::Vector2 futureTargetPos{ m_Target.Position + m_Target.LinearVelocity * secTillTargetAtPos };

	steering.LinearVelocity = -(futureTargetPos - pAgent->GetPosition());
	steering.LinearVelocity.Normalize();

	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();



	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), 5.0f, Elite::Color{ 0, 1, 0, 1 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), m_Target.Position - pAgent->GetPosition(), 5.0f, Elite::Color{ 0, 0, 1, 1 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), futureTargetPos - pAgent->GetPosition(), 5.0f, Elite::Color{ 1, 0, 0, 1 });
	}

	return steering;
}
