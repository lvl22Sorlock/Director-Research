//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../SteeringAgent.h"
#include "TheFlock.h"

using namespace Elite;

//Destructor
App_Flocking::~App_Flocking()
{	
	SAFE_DELETE(m_pFlock);
	SAFE_DELETE(m_pAgentToEvade);
	SAFE_DELETE(m_pWanderBehaviour);
}

//Functions
void App_Flocking::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize / 1.5f, m_TrimWorldSize / 2));

	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetMaxLinearSpeed(30.f);
	m_pAgentToEvade->SetMaxAngularSpeed(25.f);
	m_pWanderBehaviour = new Wander();
	m_pAgentToEvade->SetSteeringBehavior(m_pWanderBehaviour);
	m_pAgentToEvade->SetBodyColor(Elite::Color{0.8f, 0.1f, 0.1f});

	m_pFlock = new Flock(m_FlockSize, m_TrimWorldSize, m_pAgentToEvade, true);
	m_pFlock->SetAppFlocking(this);
}

void App_Flocking::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}

	if (m_pAgentToEvade)
	{
		m_pAgentToEvade->Update(deltaTime);
		m_pAgentToEvade->TrimToWorld(Elite::Vector2{ 0, 0 }, Elite::Vector2{ m_TrimWorldSize, m_TrimWorldSize });
	}

	m_pFlock->UpdateAndRenderUI();
	m_pFlock->Update(deltaTime);
	if (m_UseMouseTarget)
		m_pFlock->SetSeekTarget(m_MouseTarget);


}

void App_Flocking::Render(float deltaTime) const
{
	std::vector<Elite::Vector2> points =
	{
		{ 0, m_TrimWorldSize },
		{ m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, 0 },
		{ 0, 0 }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	if (m_pAgentToEvade)
	{
		m_pAgentToEvade->Render(deltaTime);
	}

	m_pFlock->Render(deltaTime);

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}

void App_Flocking::SetTrimWorldSize(float newSize)
{
	m_TrimWorldSize = newSize;
}
