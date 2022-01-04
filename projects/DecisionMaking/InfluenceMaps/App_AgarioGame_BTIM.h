#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"

#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

#include <vector>

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;
class NavigationColliderElement;

class App_AgarioGame_BTIM final : public IApp
{
public:
	App_AgarioGame_BTIM();
	~App_AgarioGame_BTIM();

	using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;
	using InfluenceGraph = Elite::Graph2D<Elite::InfluenceNode, Elite::GraphConnection2D>;

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;
private:
	float m_TrimWorldSize = 70.f;
	float m_TrimWorldLeftBottomAbsXY = 0.f;	// replaced -m_TrimWorldSize by this
	float m_TrimWorldRightTopAbsXY = 140.0f * 2;	// then replaced m_TrimWorldSize by this
	const int m_AmountOfAgents{ 20 * 2 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pUberAgent = nullptr;

	const int m_AmountOfFood{ 40 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;

	//--Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};
private:	
	template<class T_AgarioType>
	void UpdateAgarioEntities(vector<T_AgarioType*>& entities, float deltaTime);

	Elite::Blackboard* CreateBlackboard(AgarioAgent* a);
	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_AgarioGame_BTIM(const App_AgarioGame_BTIM&) {};
	App_AgarioGame_BTIM& operator=(const App_AgarioGame_BTIM&) {};

	void AddInfluenceForAllFood();
	void AddInfluenceForSmallerAgents();
	void AddInfluenceForLargerAgents();
	void DebugShowHighestInfluenceNode() const;
	void DebugRenderShowUberAgentRange() const;




	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid = nullptr;
	Elite::GraphRenderer m_GraphRenderer{};
	Elite::GraphEditor m_GridEditor{};


	// Research Project
private:
	void CalculateUberAgentRadiusDifferenceLargestOpponent();
	void CalculateAverageUberAgentRadiusDifference();			// needs correct m_NumberOfOpponents
	void CalculateNumberOfOpponents();
	float GetCalculatedUpgradeMultiplierForAgent(const AgarioAgent* pAgent, bool isUberAgent = false);

	void ProcessUserInput(float& deltaTime);

	float m_RadiusDifferenceLargestOpponent;
	float m_RadiusPercentageLargestOpponent;
	float m_AverageRadiusDifference;
	int m_NumberOfOpponents;

	float m_DebugTotalTime;
	float m_DebugAverageNumber1;
	float m_DebugAverageNumber2;
};

template<class T_AgarioType>
inline void App_AgarioGame_BTIM::UpdateAgarioEntities(vector<T_AgarioType*>& entities, float deltaTime)
{
	for (auto& e : entities)
	{
		e->Update(deltaTime);

		if (e->CanBeDestroyed())
			SAFE_DELETE(e);
	}

	auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
		[](T_AgarioType* e) {return e == nullptr; });
	if (toRemoveEntityIt != entities.end())
	{
		entities.erase(toRemoveEntityIt, entities.end());
	}
}
#endif