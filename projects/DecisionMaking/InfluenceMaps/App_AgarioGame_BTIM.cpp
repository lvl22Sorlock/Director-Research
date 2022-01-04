#include "stdafx.h"
#include "App_AgarioGame_BTIM.h"
#include "BehaviorsIM.h"

#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"
#include "projects/Shared/NavigationColliderElement.h"



using namespace Elite;
App_AgarioGame_BTIM::App_AgarioGame_BTIM()
	// Research Project
	:  m_RadiusDifferenceLargestOpponent{}
	, m_AverageRadiusDifference{}
	, m_NumberOfOpponents{}
	, m_DebugTotalTime{0.0f}
	, m_DebugAverageNumber1{0.0f}
	, m_DebugAverageNumber2{0.0f}
	//
{
	m_pInfluenceGrid = new InfluenceMap<InfluenceGrid>(false);
	m_pInfluenceGrid->InitializeGrid(14*2 * 2, 14*2 * 2, 10/2, false, true);
	m_pInfluenceGrid->InitializeBuffer();

	m_GraphRenderer.SetNumberPrintPrecision(0);
}

App_AgarioGame_BTIM::~App_AgarioGame_BTIM()
{
	SAFE_DELETE(m_pInfluenceGrid);
	
	
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pUberAgent);

	for (auto pNC : m_vNavigationColliders)
		SAFE_DELETE(pNC);
	m_vNavigationColliders.clear();
}

void App_AgarioGame_BTIM::Start()
{
	//Create Boundaries
	const float blockSize{ 2.0f };
	const float hBlockSize{ blockSize / 2.0f };
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldLeftBottomAbsXY - hBlockSize, 0.f), blockSize, (m_TrimWorldRightTopAbsXY + blockSize) * 2.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(m_TrimWorldRightTopAbsXY + hBlockSize, 0.f), blockSize, (m_TrimWorldRightTopAbsXY + blockSize) * 2.0f));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(0.0f, m_TrimWorldRightTopAbsXY + hBlockSize), m_TrimWorldRightTopAbsXY * 2.0f, blockSize));
	m_vNavigationColliders.push_back(new NavigationColliderElement(Elite::Vector2(0.0f, m_TrimWorldLeftBottomAbsXY - hBlockSize), m_TrimWorldRightTopAbsXY * 2.0f, blockSize));
	

	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(m_TrimWorldLeftBottomAbsXY, m_TrimWorldRightTopAbsXY);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//Create agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Vector2 randomPos = randomVector2(m_TrimWorldLeftBottomAbsXY, m_TrimWorldRightTopAbsXY);
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		//1. Create Blackboard
		Elite::Blackboard* pBlackBoard = CreateBlackboard(newAgent);
		//2. Create BehaviorTree
		Elite::BehaviorTree* pBehaviorTree = new Elite::BehaviorTree(pBlackBoard
			, new BehaviorAction(ChangeToWanderIM));
		//3. Set the BehaviorTree active on the agent 
		newAgent->SetDecisionMaking(pBehaviorTree);

		
		
		
		
		m_pAgentVec.push_back(newAgent);
	}


	//-------------------
	//Create The Uber Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(m_TrimWorldLeftBottomAbsXY, m_TrimWorldRightTopAbsXY);
	Color customColor = Color{ randomFloat(), randomFloat(), randomFloat() };
	m_pUberAgent = new AgarioAgent(randomPos, customColor);

	//Create and add the necessary blackboard data
   //1. Create Blackboard
	Elite::Blackboard* pBlackBoard = CreateBlackboard(m_pUberAgent);

	//2. Create BehaviorTree (make more conditions/actions and create a more advanced tree than the simple agents
	Elite::BehaviorTree* pBehaviorTree = new Elite::BehaviorTree(pBlackBoard
		, new BehaviorSequence(
			{
			//new BehaviorAction(InfluenceMapAtAgent)
			new BehaviorAction(GoToHighestInfluence)
			,new BehaviorAction(ChangeToSeekIM)
			/*,
			new BehaviorSelector(
			{
				new BehaviorSequence(
					{
						new BehaviorConditional(IsLargerAgentNearbyIM),
						new BehaviorAction(ChangeToFleeIM)
					}),
				new BehaviorSelector(
					{
						new BehaviorSequence(
						{
							new BehaviorConditional(IsSmallerAgentNearbyIM),
							new BehaviorAction(ChangeToSeekIM)
						}),
						new BehaviorSequence(
						{
							new BehaviorConditional(IsFoodCloseByIM),
							new BehaviorAction(ChangeToSeekIM)
						}),
					}),
				new BehaviorAction(ChangeToWanderIM)
			})*/
			}
		));
	//3. Set the BehaviorTree active on the agent 
	m_pUberAgent->SetDecisionMaking(pBehaviorTree);
				
		//2. Create BehaviorTree (make more conditions/actions and create a more advanced tree than the simple agents
		//3. Set the BehaviorTree active on the agent 

	// Research Project	
	CalculateNumberOfOpponents();

	CalculateUberAgentRadiusDifferenceLargestOpponent();
	CalculateAverageUberAgentRadiusDifference();
	//
	
}

void App_AgarioGame_BTIM::Update(float deltaTime)
{
	// Research Project
	ProcessUserInput(deltaTime);

	CalculateNumberOfOpponents();

	CalculateUberAgentRadiusDifferenceLargestOpponent();
	CalculateAverageUberAgentRadiusDifference();

	/*float newOpponentUpgradeModifier{ 1 };
	if (m_pAgentVec.size() >= 1)
	{
		newOpponentUpgradeModifier = GetCalculatedUpgradeMultiplierForAgent(m_pAgentVec[0]);
	}*/
	float largestOpponentUpgradeModifier{ 0 };
	const AgarioAgent* pLargestAgent{ nullptr };

	for (AgarioAgent* pAgent : m_pAgentVec)
	{
		const float UPGRADE_MODIFIER{GetCalculatedUpgradeMultiplierForAgent(pAgent)};

		//if (UPGRADE_MODIFIER > largestOpponentUpgradeModifier) {
		if (!pLargestAgent || pAgent->GetRadius() > pLargestAgent->GetRadius()){
			largestOpponentUpgradeModifier = UPGRADE_MODIFIER;
			pLargestAgent = pAgent;
		}

		pAgent->SetUpgradeModifier(UPGRADE_MODIFIER);
	}
	m_pUberAgent->SetUpgradeModifier(GetCalculatedUpgradeMultiplierForAgent(m_pUberAgent, true));

	
	m_DebugAverageNumber1 =
	{
		(m_DebugAverageNumber1 * m_DebugTotalTime
		+
		GetCalculatedUpgradeMultiplierForAgent(m_pUberAgent, true) * deltaTime)
		/
		(m_DebugTotalTime + deltaTime)
	};
	m_DebugAverageNumber2 =
	{
		(m_DebugAverageNumber2* m_DebugTotalTime
		+
		largestOpponentUpgradeModifier * deltaTime)
		/
		(m_DebugTotalTime + deltaTime)
	};
	m_DebugTotalTime += deltaTime;
	//std::cout << m_pUberAgent->GetMass() << '\n';	
	std::cout << m_DebugAverageNumber1 << '\n';
	std::cout << m_DebugAverageNumber2 << '\n';
	std::cout << "UberAgentUpgradeModifier: " << GetCalculatedUpgradeMultiplierForAgent(m_pUberAgent, true) 
		<< "  Size: " << m_pUberAgent->GetRadius()
		<< '\n';
	std::cout << "Opponent Upgrade Modifier: " << largestOpponentUpgradeModifier;
	if (pLargestAgent) std::cout << "  Size: " << pLargestAgent->GetRadius();
	std::cout	<< "\n\n";

	 
	
	//
	
	
	////
	AddInfluenceForAllFood();
	AddInfluenceForSmallerAgents();
	AddInfluenceForLargerAgents();
	////
	
	UpdateImGui();

	//Check if agent is still alive
	if (m_pUberAgent->CanBeDestroyed())
	{
		m_GameOver = true;
		return;
	}
	//Update the custom agent
	m_pUberAgent->Update(deltaTime);
	
	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(m_TrimWorldLeftBottomAbsXY, m_TrimWorldRightTopAbsXY)));
	}



	////
	m_GridEditor.UpdateGraph(m_pInfluenceGrid);
	m_pInfluenceGrid->PropagateInfluence(deltaTime);
	////
}

void App_AgarioGame_BTIM::ProcessUserInput(float& deltaTime)
{
	const Uint8* state{ SDL_GetKeyboardState(NULL) };
	if (state[SDL_SCANCODE_F] /*&& !m_WasFPressed*/)
	{
		deltaTime *= 25;
		//m_WasFPressed = true;
	}
	//if (!state[SDL_SCANCODE_F]) m_WasFPressed = false
}

void App_AgarioGame_BTIM::Render(float deltaTime) const
{
	//DEBUGRENDERER2D->DrawCircle({ 0,0 }, 70.0f, { 1,0, 0 }, 0.7f);
	DebugRenderShowUberAgentRange();
	m_pInfluenceGrid->SetNodeColorsBasedOnInfluence();
	m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);


	std::vector<Elite::Vector2> points =
	{
		{ m_TrimWorldLeftBottomAbsXY, m_TrimWorldRightTopAbsXY },
		{ m_TrimWorldRightTopAbsXY, m_TrimWorldRightTopAbsXY },
		{ m_TrimWorldRightTopAbsXY, m_TrimWorldLeftBottomAbsXY },
		{ m_TrimWorldLeftBottomAbsXY, m_TrimWorldLeftBottomAbsXY }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pUberAgent->Render(deltaTime);


	
	//m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);
	//m_pInfluenceGraph2D->SetNodeColorsBasedOnInfluence();
	//m_GraphRenderer.RenderGraph(m_pInfluenceGraph2D, true, true);
}

Blackboard* App_AgarioGame_BTIM::CreateBlackboard(AgarioAgent* a)
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("AgentsVec", &m_pAgentVec);
	pBlackboard->AddData("FoodVec", &m_pFoodVec);
	pBlackboard->AddData("WorldSize", m_TrimWorldRightTopAbsXY);
	pBlackboard->AddData("Target", Elite::Vector2{});
	pBlackboard->AddData("AgentFleeTarget", static_cast<AgarioAgent*>(nullptr)); // Needs the cast for the type
	pBlackboard->AddData("Time", 0.0f); 
	pBlackboard->AddData("InfluenceGrid", m_pInfluenceGrid);

	return pBlackboard;
}

void App_AgarioGame_BTIM::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pUberAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pUberAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}

void App_AgarioGame_BTIM::AddInfluenceForAllFood()
{
	for (AgarioFood* pFood : m_pFoodVec)
	{
		
		m_pInfluenceGrid->SetInfluenceAtPosition(
			pFood->GetPosition(),
			7.5f + m_pInfluenceGrid->GetNodeAtWorldPos(pFood->GetPosition())->GetInfluence());
	}
}

void App_AgarioGame_BTIM::AddInfluenceForSmallerAgents()
{
	const float MIN_AGENT_RADIUS_DIFFERENCE{ 4.0f };
	for (AgarioAgent* pOtherAgent : m_pAgentVec)
	{
		if ((m_pUberAgent->GetRadius() - pOtherAgent->GetRadius() > MIN_AGENT_RADIUS_DIFFERENCE))
		{
			m_pInfluenceGrid->SetInfluenceAtPosition(
				pOtherAgent->GetPosition(),
				(10.0f + m_pInfluenceGrid->GetNodeAtWorldPos(pOtherAgent->GetPosition())->GetInfluence())
				/**Elite::Clamp(pOtherAgent->GetRadius()/5.0f, 1.0f, 1.5f)*/);
		}
	}
}

void App_AgarioGame_BTIM::AddInfluenceForLargerAgents()
{
	const float MIN_AGENT_RADIUS_DIFFERENCE{ 0.4f };
	for (AgarioAgent* pOtherAgent : m_pAgentVec)
	{
		if (pOtherAgent->GetRadius() - m_pUberAgent->GetRadius() > MIN_AGENT_RADIUS_DIFFERENCE)
		{
			m_pInfluenceGrid->SetInfluenceAtPosition(
				pOtherAgent->GetPosition(),
				-30.0f + m_pInfluenceGrid->GetNodeAtWorldPos(pOtherAgent->GetPosition())->GetInfluence());
		}
	}
}

void App_AgarioGame_BTIM::DebugShowHighestInfluenceNode() const
{
	//Elite::InfluenceNode* pHighestInfluenceNode{ nullptr };

	//for (Elite::InfluenceNode* pNode : m_pInfluenceGrid->GetAllNodes())
	//{
	//	if (pHighestInfluenceNode == nullptr
	//		||
	//		pNode->GetInfluence() > pHighestInfluenceNode->GetInfluence())
	//	{
	//		pHighestInfluenceNode = pNode;
	//		continue;
	//	}
	//}
	const float TOO_FAR_AWAY_FROM_AGENT_RANGE{ 35.0f };

	Elite::InfluenceNode* pHighestInfluenceNode{ nullptr };

	for (Elite::InfluenceNode* pNode : m_pInfluenceGrid->GetAllNodes())
	{
		if (pHighestInfluenceNode == nullptr
			||
			(
				pNode->GetInfluence() > pHighestInfluenceNode->GetInfluence()
				&&
				Elite::DistanceSquared(m_pInfluenceGrid->GetNodeWorldPos(pNode), m_pUberAgent->GetPosition())
				<
				TOO_FAR_AWAY_FROM_AGENT_RANGE * TOO_FAR_AWAY_FROM_AGENT_RANGE
				))
		{
			pHighestInfluenceNode = pNode;
			continue;
		}
	}

	if (pHighestInfluenceNode)
	{ 
		DEBUGRENDERER2D->DrawCircle(m_pInfluenceGrid->GetNodeWorldPos(pHighestInfluenceNode), 10.f, Elite::Color(1, 0, 0), 0.7f);
	}
	else
	{
		DEBUGRENDERER2D->DrawCircle(Elite::Vector2(0,0), 15.f, Elite::Color(1, 0, 1), 0.7f);

	}
}

void App_AgarioGame_BTIM::DebugRenderShowUberAgentRange() const
{
	const float TOO_FAR_AWAY_FROM_AGENT_RANGE{ 5.0f + m_pUberAgent->GetRadius()};	// used to be 35.0f
	DEBUGRENDERER2D->DrawCircle(m_pUberAgent->GetPosition(), TOO_FAR_AWAY_FROM_AGENT_RANGE, Elite::Color(0, 1, 0), 0.7f);
}

void App_AgarioGame_BTIM::CalculateUberAgentRadiusDifferenceLargestOpponent()
{	
	// Find largest opponent
	const AgarioAgent* largestAgent{nullptr};
	for (const AgarioAgent* pAgent : m_pAgentVec)
	{
		if (!largestAgent) largestAgent = pAgent;

		if (pAgent->GetRadius() > largestAgent->GetRadius())
		{
			largestAgent = pAgent;
		}
	}

	// largest opponent mass
	float radiusLargestOpponent{ 1 };
	if (largestAgent)
	{
		radiusLargestOpponent = largestAgent->GetRadius();

		// how much larger is the uber agent
		const float UBER_AGENT_SIZE_DIFFERENCE =
		{
			m_pUberAgent->GetRadius() - radiusLargestOpponent
		};
		m_RadiusDifferenceLargestOpponent = UBER_AGENT_SIZE_DIFFERENCE;

		m_RadiusPercentageLargestOpponent =
		{
			m_pUberAgent->GetRadius() / radiusLargestOpponent
		};
	}
	else
	{
		m_RadiusDifferenceLargestOpponent = 0;
		m_RadiusPercentageLargestOpponent = 1;
	}

	
}

void App_AgarioGame_BTIM::CalculateAverageUberAgentRadiusDifference()	// needs correct m_NumberOfOpponents
{
	if (m_NumberOfOpponents <= 0)
	{
		m_AverageRadiusDifference = 0;
		return;
	}
	
	// Calculate total opponent mass
	float totalRadius{0};
	for (const AgarioAgent* pAgent : m_pAgentVec)
	{
		totalRadius += pAgent->GetRadius();
	}

	m_AverageRadiusDifference =
	{
		m_pUberAgent->GetRadius() -	(totalRadius / m_NumberOfOpponents)
	};
}

void App_AgarioGame_BTIM::CalculateNumberOfOpponents()
{
	m_NumberOfOpponents = m_pAgentVec.size();
}

float App_AgarioGame_BTIM::GetCalculatedUpgradeMultiplierForAgent(const AgarioAgent* pAgent, bool isUberAgent)
{
	// Base scenario : for other agents

	//bool isPlayerWinning{ false };
	//if (m_RadiusDifferenceLargestOpponent >= 0) 
	//	isPlayerWinning = true;

	//const float MULTIPLIER_PER_RADIUS{ 0.15f };
	//const float MAX_MULTIPLIER{ 1 + (MULTIPLIER_PER_RADIUS * m_RadiusDifferenceLargestOpponent) };
	const float MULTIPLIER_CONSTANT_FACTOR{ 1.0f };

	const float MAX_MULTIPLIER{ 1.5f * m_RadiusPercentageLargestOpponent * MULTIPLIER_CONSTANT_FACTOR };
	float multiplier{};
	if (!isUberAgent) {
		multiplier = (m_pUberAgent->GetRadius() / pAgent->GetRadius()) * MULTIPLIER_CONSTANT_FACTOR;
		multiplier = Elite::Clamp(multiplier, 0.1f, MAX_MULTIPLIER);
	}
	else {
		multiplier = 1 / (m_RadiusPercentageLargestOpponent * MULTIPLIER_CONSTANT_FACTOR);
	}

	float result{Elite::Clamp(multiplier, 0.5f, 2.0f)};

	//result = 1 + (MULTIPLIER_PER_RADIUS * m_AverageRadiusDifference);



	//result = Elite::Clamp(result, 0.5f, MAX_MULTIPLIER);

	//if (isUberAgent) result = 1 / result;
	return result;
	
	//const float MULTIPLIER_PER_RADIUS{ 0.15f };
	//const float MAX_MULTIPLIER{1 + (MULTIPLIER_PER_RADIUS * m_RadiusDifferenceLargestOpponent)};
	//
	//float result{};

	//result = 1 + (MULTIPLIER_PER_RADIUS *  m_AverageRadiusDifference);



	//result = Elite::Clamp(result, 0.5f, MAX_MULTIPLIER);

	//if (isUberAgent) result = 1 / result;
	//return result;
}

