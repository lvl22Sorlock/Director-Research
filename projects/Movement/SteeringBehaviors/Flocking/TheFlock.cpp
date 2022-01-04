#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "projects/Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include "projects/Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"
#include "projects/Movement/SteeringBehaviors/Flocking/App_Flocking.h"

//Constructor & Destructor
Flock::Flock(
	int flockSize, 
	float worldSize, 
	SteeringAgent* pAgentToEvade, 
	bool trimWorld)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
	, m_CanDebugRender{false}
	, m_pSeparationBehavior{nullptr}
	, m_pWanderBehavior{nullptr}
	, m_pEvadeBehavior{nullptr}
	, m_pCohesionBehavior{nullptr}
	, m_pVelMatchBehavior{nullptr}
	, m_pSeekBehavior{nullptr}
	, m_pBlendedSteering{nullptr}
	, m_pPrioritySteering{nullptr}
	, m_CellWidth{25.f}
	, m_pCellSpace{new CellSpace(worldSize, worldSize, int(worldSize/ m_CellWidth), int(worldSize/ m_CellWidth), 150)}
	, m_UseCellSpace{true}
	, m_pAppFlocking{nullptr}
{
	
	
	m_Agents.resize(m_FlockSize);
	m_pCellSpace->SetNeighbordhoodRadius(m_NeighborhoodRadius);
	// TODO: initialize the flock and the memory pool

	m_Neighbors.resize(m_FlockSize / 5);

	m_OldAgentPositions.resize(m_FlockSize);

	
	Separation* m_pSeparationBehavior = new Separation{ this };
	Cohesion* m_pCohesionBehavior = new Cohesion{this};
	Alignment* m_pVelMatchBehavior = new Alignment{ this };
	Seek* m_pSeekBehavior = new Seek{};
	Wander* m_pWanderBehavior = new Wander{};
	Evade* m_pEvadeBehavior = new Evade{};
	std::vector<BlendedSteering::WeightedBehavior> weightedSteeringBehaviors;
	weightedSteeringBehaviors.push_back(BlendedSteering::WeightedBehavior{ m_pWanderBehavior, 0.5f });
	weightedSteeringBehaviors.push_back(BlendedSteering::WeightedBehavior{ m_pSeekBehavior, 0.0f });
	weightedSteeringBehaviors.push_back(BlendedSteering::WeightedBehavior{ m_pCohesionBehavior, 0.5f });
	weightedSteeringBehaviors.push_back(BlendedSteering::WeightedBehavior{ m_pSeparationBehavior, 0.52f });
	weightedSteeringBehaviors.push_back(BlendedSteering::WeightedBehavior{ m_pVelMatchBehavior, 0.57f });
	m_pBlendedSteering = new BlendedSteering(weightedSteeringBehaviors);

	std::vector<ISteeringBehavior*> steeringBehaviors;
	steeringBehaviors.push_back(m_pEvadeBehavior);
	steeringBehaviors.push_back(m_pBlendedSteering);
	m_pPrioritySteering = new PrioritySteering(steeringBehaviors);

	for (int i = 0; i < m_FlockSize; ++i)
	{
		SteeringAgent* pAgent{ nullptr };
		pAgent = new SteeringAgent();
		pAgent->SetAutoOrient(true);
		pAgent->SetMaxAngularSpeed(80.f);
		pAgent->SetMaxLinearSpeed(60.f);
		pAgent->SetMass(1.f);

		pAgent->SetSteeringBehavior(m_pPrioritySteering);
		pAgent->SetPosition(Elite::Vector2{ float(rand() % int(worldSize)), float(rand() % int(worldSize)) });

		m_Agents[i] = pAgent;

	}

	for (SteeringAgent* agent : m_Agents)
	{
		m_pCellSpace->AddAgent(agent);
	}
	m_pCellSpace->SetDebugAgent(m_Agents[0]);

}

Flock::~Flock()
{
	// TODO: clean up any additional data


	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);


	SAFE_DELETE(m_pCellSpace);


	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
}

void Flock::Update(float deltaT)
{
	// TODO: update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world

	SetCanDebugRender(m_CanDebugRender);

	if (m_pAgentToEvade)
	{

		TargetData agentToEvadeData{ m_pAgentToEvade->GetPosition(), m_pAgentToEvade->GetOrientation(), m_pAgentToEvade->GetLinearVelocity(),
			m_pAgentToEvade->GetAngularVelocity()};
	
		m_pPrioritySteering->GetPriorityBehaviorsRef()[0]->SetTarget(agentToEvadeData);

	}

	int idx{ 0 };
	for (SteeringAgent* element : m_Agents)
	{
		if (!m_UseCellSpace)
		{
			RegisterNeighbors(element);
		}
		else
		{
			m_pCellSpace->RegisterNeighbors(element, m_NeighborhoodRadius);
		}
		element->Update(deltaT);
		element->TrimToWorld(Elite::Vector2{ 0, 0 }, Elite::Vector2{ m_WorldSize, m_WorldSize });
		if (m_UseCellSpace)
		{
			m_pCellSpace->UpdateAgentCell(element, m_OldAgentPositions[idx]);
		}
		m_OldAgentPositions[idx] = element->GetPosition();
		++idx;
	}

	m_pAgentToEvade->SetRenderBehavior(m_CanDebugRender);


}

void Flock::Render(float deltaT)
{
	// TODO: render the flock
	for (SteeringAgent* element : m_Agents)
	{
		element->Render(deltaT);
	}

	if (m_CanDebugRender && m_UseCellSpace)
	{
		m_pCellSpace->RenderCells();
	}

	if (m_pAgentToEvade && m_CanDebugRender)
	{
		DEBUGRENDERER2D->DrawCircle(m_pAgentToEvade->GetPosition(), 35.f, Elite::Color{ 1, 0, 0, 1 }, 0);
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here

	ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);

	ImGui::Spacing();

	ImGui::Spacing();
	
	ImGui::Checkbox("Using Cellspace", &m_UseCellSpace);
	ImGui::Spacing();

	if (m_pWanderBehavior)
	{
		ImGui::SliderFloat("Wander",
			GetWeight(m_pWanderBehavior), 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Seek",
			GetWeight(m_pSeekBehavior), 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Cohesion",
			GetWeight(m_pCohesionBehavior), 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Separation",
			GetWeight(m_pSeparationBehavior), 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Alignment",
			GetWeight(m_pVelMatchBehavior), 0.f, 2.f, "%.2f");
	}
	else
	{
		ImGui::SliderFloat("Wander",
			&m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Seek",
			&m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Cohesion",
			&m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Separation",
			&m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 2.f, "%.2f");
		ImGui::SliderFloat("Alignment",
			&m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 2.f, "%.2f");
	}


	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 totalPos{};
	if (!m_UseCellSpace)
	{
		for (int i = 0; i < m_NrOfNeighbors; ++i)
		{
			if (m_Neighbors[i])
				totalPos += m_Neighbors[i]->GetPosition();
		}

		totalPos /= float(m_NrOfNeighbors);
	}
	else
	{
		for (int i = 0; i < m_pCellSpace->GetNrOfNeighbors(); ++i)
		{
			if (m_pCellSpace->GetNeighbors()[i])
				totalPos += m_pCellSpace->GetNeighbors()[i]->GetPosition();
		}
		totalPos /= float(m_pCellSpace->GetNrOfNeighbors());
	}

	return totalPos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 totalVelocity{};
	if (!m_UseCellSpace)
	{
		for (int i = 0; i < m_NrOfNeighbors; ++i)
		{
			if (m_Neighbors[i])
				totalVelocity += m_Neighbors[i]->GetLinearVelocity();
		}

		totalVelocity /= float(m_NrOfNeighbors);
	}
	else
	{
		for (int i = 0; i < m_pCellSpace->GetNrOfNeighbors(); ++i)
		{
			if (m_pCellSpace->GetNeighbors()[i])
				totalVelocity += m_pCellSpace->GetNeighbors()[i]->GetLinearVelocity();
		}
		totalVelocity /= float(m_pCellSpace->GetNrOfNeighbors());
	}
	
	return totalVelocity;
}

void Flock::SetSeekTarget(TargetData target)
{
	m_pBlendedSteering->GetWeightedBehaviorsRef()[1].pBehavior->SetTarget(target);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	for (SteeringAgent* element : m_Agents)
	{
		if 
			(Elite::DistanceSquared(pAgent->GetPosition(), element->GetPosition()) <= m_NeighborhoodRadius* m_NeighborhoodRadius)
		{
			if (m_NrOfNeighbors < int(m_Neighbors.size()))
			{
				m_Neighbors[m_NrOfNeighbors] = element;
				++m_NrOfNeighbors;
			}
			else
			{
				m_Neighbors.push_back(element);
				++m_NrOfNeighbors;
			}
		}
	}
}

void Flock::SetCanDebugRender(bool canDebugRender)
{
	m_Agents[0]->SetRenderBehavior(canDebugRender);
	if (m_pAgentToEvade)
		m_pAgentToEvade->SetRenderBehavior(canDebugRender);
}

void Flock::SetAppFlocking(App_Flocking* app_flocking)
{
	m_pAppFlocking = app_flocking;
}

int Flock::GetNrOfNeighbors() const
{
	if (m_UseCellSpace)
	{
		return m_pCellSpace->GetNrOfNeighbors();
	}
	return m_NrOfNeighbors; 
}
const vector<SteeringAgent*>& Flock::GetNeighbors() const
{
	if (m_UseCellSpace)
	{
		return m_pCellSpace->GetNeighbors();
	}
	return m_Neighbors;
}
