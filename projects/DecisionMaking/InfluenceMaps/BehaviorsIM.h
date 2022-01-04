/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

Elite::BehaviorState ChangeToWanderIM(Elite::Blackboard* pBlackboard)
{
    AgarioAgent* pAgent = nullptr;
    pBlackboard->GetData("Agent", pAgent);

    if (!pAgent)
    {
        return Elite::BehaviorState::Failure;
    }

    pAgent->SetToWander();
    return Elite::BehaviorState::Success;

}

Elite::BehaviorState ChangeToSeekIM(Elite::Blackboard* pBlackboard)
{
    AgarioAgent* pAgent{ nullptr };
    pBlackboard->GetData("Agent", pAgent);
    Elite::Vector2 target = Elite::Vector2();
    pBlackboard->GetData("Target", target);
    if (!pAgent)
    {
        return Elite::BehaviorState::Failure;
    }

    pAgent->SetToSeek(target);
    return Elite::BehaviorState::Success;

}

Elite::BehaviorState ChangeToFleeIM(Elite::Blackboard* pBlackboard)
{
    AgarioAgent* pAgent{ nullptr };
    pBlackboard->GetData("Agent", pAgent);
    AgarioAgent* agentFleeTarget{ nullptr };
    pBlackboard->GetData("AgentFleeTarget", agentFleeTarget);
    if (!pAgent || !agentFleeTarget)
    {
        return Elite::BehaviorState::Failure;
    }

    pAgent->SetToFlee(agentFleeTarget->GetPosition());
    //DEBUGRENDERER2D->DrawCircle(agentFleeTarget->GetPosition(), 5.0f + agentFleeTarget->GetRadius(), Elite::Color(1,0,0), 0.7f);

    return Elite::BehaviorState::Success;
}

//condition
bool IsFoodCloseByIM(Elite::Blackboard* pBlackboard)
{
    AgarioAgent* pAgent{ nullptr };
    bool succes{ pBlackboard->GetData("Agent", pAgent) };
    if (!succes)
    {
        std::cout << "Agent not found in function IsFoodCloseBy\n";
        return false;
    }

    std::vector<AgarioFood*>* pFoodVec;
    succes = pBlackboard->GetData("FoodVec", pFoodVec);
    if (!succes)
    {
       std::cout << "FoodVec not found in function IsFoodCloseBy\n";
        return false;
    }

    if (pFoodVec->size() < 1) return false;

    const float range{ 20.0f + pAgent->GetRadius() * 0.75f};
    //DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), range, Elite::Color(0, 1, 0), 0.7f);


    AgarioFood* closestfood = (*pFoodVec)[0];
    float closestDistance{ DistanceSquared(pAgent->GetPosition(),closestfood->GetPosition()) };
    for (unsigned int i{ 1 }; i < pFoodVec->size(); i++)
    {
        //DEBUGRENDERER2D->DrawCircle((*pFoodVec)[i]->GetPosition(), 2.0f, Elite::Color(0, 1, 0), 0.7f);
        float distancefood = DistanceSquared(pAgent->GetPosition(), (*pFoodVec)[i]->GetPosition());
        if (distancefood < closestDistance)
        {
            closestfood = (*pFoodVec)[i];
            closestDistance = distancefood;
        }
    }

    if (closestDistance < range * range)
    {
        pBlackboard->ChangeData("Target", closestfood->GetPosition());
        return true;
    }
    return false;
}

bool IsSmallerAgentNearbyIM(Elite::Blackboard* pBlackboard)
{
    AgarioAgent* pAgent{ nullptr };
    bool succes{ pBlackboard->GetData("Agent", pAgent) };
    if (!succes)
    {
        std::cout << "Agent not found in function SmallerAgentNearby\n";
        return false;
    }

    std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
    succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
    if (!succes)
    {
        std::cout << "AgentsVec not found in function SmallerAgentNearby\n";
        return false;
    }

    const float MIN_DISTANCE_TO_SMALLER_AGENT{ 25.0f + pAgent->GetRadius() * 1.25f};
    const float MIN_AGENT_RADIUS_DIFFERENCE{ 10.0f };
    //DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), MIN_DISTANCE_TO_SMALLER_AGENT, Elite::Color(0, 1, 0.7f), 0.7f);


    AgarioAgent* pClosestCloseSmallerAgent{ nullptr };
    for (AgarioAgent* pOtherAgent : *pAgentPtrVec)
    {
        const float NEW_MIN_DISTANCE_TO_SMALLER_AGENT{ MIN_DISTANCE_TO_SMALLER_AGENT + pOtherAgent->GetRadius() };
        //DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), NEW_MIN_DISTANCE_TO_SMALLER_AGENT, Elite::Color(0, 0, 0), 0.7f);

        if (!(pAgent->GetRadius() - pOtherAgent->GetRadius() > MIN_AGENT_RADIUS_DIFFERENCE)) continue;

        //DEBUGRENDERER2D->DrawCircle(pOtherAgent->GetPosition(), 10.0f + pOtherAgent->GetRadius(), Elite::Color(1, 1, 1), 0.7f);

        if (Elite::DistanceSquared(pAgent->GetPosition(), pOtherAgent->GetPosition()) > NEW_MIN_DISTANCE_TO_SMALLER_AGENT * NEW_MIN_DISTANCE_TO_SMALLER_AGENT) continue;

        if (!pClosestCloseSmallerAgent)
        {
            pClosestCloseSmallerAgent = pOtherAgent;
            continue;
        }

        if (Elite::DistanceSquared(pAgent->GetPosition(), pOtherAgent->GetPosition())
            <
            Elite::DistanceSquared(pAgent->GetPosition(), pClosestCloseSmallerAgent->GetPosition()))
        {
            pClosestCloseSmallerAgent = pOtherAgent;
        }
    }

    if (pClosestCloseSmallerAgent)
    {
        pBlackboard->ChangeData("Target", pClosestCloseSmallerAgent->GetPosition());
        //DEBUGRENDERER2D->DrawCircle(pClosestCloseSmallerAgent->GetPosition(), 10.0f, Elite::Color(0, 1, 0), 0.7f);
        return true;
    }
    return false;
}

bool IsLargerAgentNearbyIM(Elite::Blackboard* pBlackboard)
{
    AgarioAgent* pAgent{ nullptr };
    bool succes{ pBlackboard->GetData("Agent", pAgent) };
    if (!succes)
    {
        std::cout << "Agent not found in function LargerAgentNearby\n";
        return false;
    }    

    std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
    succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
    if (!succes)
    {
        std::cout << "AgentsVec not found in function LargerAgentNearby\n";
        return false;
    }
    
    const float MIN_DISTANCE_TO_BIGGER_AGENT{ 20.0f};
    const float MIN_AGENT_RADIUS_DIFFERENCE{ 0.4f };
    //DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), MIN_DISTANCE_TO_BIGGER_AGENT, Elite::Color(1, 0, 0), 0.7f);


    AgarioAgent* pClosestCloseBiggerAgent{ nullptr };
    for (AgarioAgent* pOtherAgent : *pAgentPtrVec)
    {
        const float NEW_MIN_DISTANCE_TO_BIGGER_AGENT{ MIN_DISTANCE_TO_BIGGER_AGENT + pAgent->GetRadius() + pOtherAgent->GetRadius() };

        if (!(pOtherAgent->GetRadius() - pAgent->GetRadius() > MIN_AGENT_RADIUS_DIFFERENCE)) continue;
       

        if (Elite::DistanceSquared(pAgent->GetPosition(), pOtherAgent->GetPosition()) > NEW_MIN_DISTANCE_TO_BIGGER_AGENT * NEW_MIN_DISTANCE_TO_BIGGER_AGENT) continue;

        if (!pClosestCloseBiggerAgent)
        {
            pClosestCloseBiggerAgent = pOtherAgent;
            continue;
        }

        if (Elite::DistanceSquared(pAgent->GetPosition(), pOtherAgent->GetPosition())
            <
            Elite::DistanceSquared(pAgent->GetPosition(), pClosestCloseBiggerAgent->GetPosition()))
        {
            pClosestCloseBiggerAgent = pOtherAgent;
        }
    }

    if (pClosestCloseBiggerAgent)
    {
        pBlackboard->ChangeData("AgentFleeTarget", pClosestCloseBiggerAgent);
        return true;
    }
    return false;
}


Elite::InfluenceNode* GetHighestInfluenceNode(Elite::Blackboard* pBlackboard)
{    
    AgarioAgent* pAgent{ nullptr };
    bool succes{ pBlackboard->GetData("Agent", pAgent) };
    if (!succes)
    {
        std::cout << "Agent not found in function GetHighestInfluenceNode\n";
        return nullptr;
    }
    Elite::InfluenceMap<Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>>* pInfluenceGrid{ nullptr };
    succes = pBlackboard->GetData("InfluenceGrid", pInfluenceGrid);
    if (!succes)
    {
        std::cout << "InfluenceGrid not found in function GetHighestInfluenceNode\n";
        return nullptr;
    }

    const float TOO_FAR_AWAY_FROM_AGENT_RANGE{ 5.0f + pAgent->GetRadius()}; // used to be 35.0f, then 35.0f + (radius / 2)

    Elite::InfluenceNode* pHighestInfluenceNode{ nullptr };

    for (Elite::InfluenceNode* pNode : pInfluenceGrid->GetAllNodes())
    {
        if (pHighestInfluenceNode == nullptr
            ||
            (
                pNode->GetInfluence() > pHighestInfluenceNode->GetInfluence()
                &&
                    Elite::DistanceSquared(pInfluenceGrid->GetNodeWorldPos(pNode), pAgent->GetPosition()) 
                    < 
                    TOO_FAR_AWAY_FROM_AGENT_RANGE * TOO_FAR_AWAY_FROM_AGENT_RANGE
                ))
        {
            pHighestInfluenceNode = pNode;
            continue;
        }
    }
    return pHighestInfluenceNode;
}

Elite::BehaviorState InfluenceMapAtAgent(Elite::Blackboard* pBlackboard)
{
    Elite::InfluenceMap<Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>>* pInfluenceGrid{ nullptr };
    bool succes{ pBlackboard->GetData("InfluenceGrid", pInfluenceGrid) };
    if (!succes)
    {
        std::cout << "InfluenceGrid not found in function InfluenceMapAtAgent\n";
        return Elite::BehaviorState::Failure;
    }
    AgarioAgent* pAgent{ nullptr };
    succes = pBlackboard->GetData("Agent", pAgent);
    if (!succes)
    {
        std::cout << "Agent not found in function InfluenceMapAtAgent\n";
        return Elite::BehaviorState::Failure;
    }

    pInfluenceGrid->SetInfluenceAtPosition(
        pAgent->GetPosition(),
        10.f + pInfluenceGrid->GetNodeAtWorldPos(pAgent->GetPosition())->GetInfluence());
    return Elite::BehaviorState::Success;
}

Elite::BehaviorState GoToHighestInfluence(Elite::Blackboard* pBlackboard)
{    
    AgarioAgent* pAgent{ nullptr };
    bool succes{ pBlackboard->GetData("Agent", pAgent) };
    if (!succes)
    {
        std::cout << "Agent not found in function GoToHighestInfluence\n";
        return Elite::BehaviorState::Failure;
    }    
    Elite::InfluenceMap<Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>>* pInfluenceGrid{ nullptr };
    succes = pBlackboard->GetData("InfluenceGrid", pInfluenceGrid);
    if (!succes)
    {
        std::cout << "InfluenceGrid not found in function GoToHighestInfluence\n";
        return Elite::BehaviorState::Failure;
    }

    pBlackboard->ChangeData("Target", pInfluenceGrid->GetNodeWorldPos(GetHighestInfluenceNode(pBlackboard)));

    return Elite::BehaviorState::Success;
}










#endif