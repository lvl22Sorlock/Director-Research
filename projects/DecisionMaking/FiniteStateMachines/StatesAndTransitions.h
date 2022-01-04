/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"


//------------
//---STATES---
//------------

//A state that makes the agent go wander
class WanderState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };

		if (!succes)
			return;

		pAgent->SetToWander();
	}

private:
	float m_Timer = 0.0f;
	
};

class FindFoodState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };

		if (!succes)
			return;

		AgarioFood* pFoodTarget{ nullptr};
		succes = pBlackboard->GetData("FoodTarget", pFoodTarget);
		if (!succes)
			return;

		if (pFoodTarget)
		{
			pAgent->SetToSeek(pFoodTarget->GetPosition());
		}
		else
		{
			pAgent->SetToWander();
		}
	}

	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };

		if (!succes)
		{
			return;
		}

		std::vector<AgarioFood*>* pFoodPtrVector{ nullptr };

		succes = pBlackboard->GetData("FoodVec", pFoodPtrVector);
		if (!succes)
		{
			return;
		}

		AgarioFood* pClosestFood = nullptr;

		if (pFoodPtrVector != nullptr)
		{
			for (AgarioFood* pFood : *pFoodPtrVector)	// find closest food
			{
				if (!pFood->CanBeDestroyed())
				{
					if (pClosestFood == nullptr
						||
						Elite::DistanceSquared(pFood->GetPosition(), pAgent->GetPosition())
						<
						Elite::DistanceSquared(pClosestFood->GetPosition(), pAgent->GetPosition()))
					{
						pClosestFood = pFood;
					}
				}
			}
		}

		

		if (pClosestFood)
		{
			pBlackboard->ChangeData("FoodTarget", pClosestFood);
			pAgent->SetToSeek(pClosestFood->GetPosition());

			// Debug
			//std::cout << "FindFoodState\n";
			//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pClosestFood->GetPosition() - pAgent->GetPosition(), 15.0f, Elite::Color(0.2f, 0.9f, 0.2f));
		}
	}

private:
	float m_Timer = 0.0f;
	//AgarioFood* m_pCurrentFood = nullptr;
	//bool m_IsGoingToFood = false;

};

class EvadeState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };

		if (!succes)
			return;

		pAgent->SetToWander();
	}

	static bool FindBiggerAgent(AgarioAgent* pAgent, std::vector<AgarioAgent*>* pAgentPtrVec, AgarioAgent* pLargerAgent, const float MIN_DISTANCE_TO_BIGGER_AGENT = 10.0f)
	{
		const float MIN_AGENT_RADIUS_DIFFERENCE{0.4f};
		

		AgarioAgent* pClosestCloseBiggerAgent{ nullptr };
		for (AgarioAgent* pOtherAgent : *pAgentPtrVec)
		{
			//const float NEW_MIN_DISTANCE_TO_BIGGER_AGENT{ MIN_DISTANCE_TO_BIGGER_AGENT + pAgent->GetRadius() + pOtherAgent->GetRadius() };

			if (!(pOtherAgent->GetRadius() - pAgent->GetRadius() > MIN_AGENT_RADIUS_DIFFERENCE)) continue;

			if (Elite::DistanceSquared(pAgent->GetPosition(), pOtherAgent->GetPosition()) > MIN_DISTANCE_TO_BIGGER_AGENT * MIN_DISTANCE_TO_BIGGER_AGENT) continue;

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
			pLargerAgent = pClosestCloseBiggerAgent;
			return true;
		}
		return false;
	}

	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };
		if (!succes)
		{
			std::cout << "Agent not found while updating EvadeState\n";
			return;
		}

		const float NEW_MIN_DISTANCE_TO_BIGGER_AGENT{ pAgent->GetRadius() + MIN_DISTANCE_TO_BIGGER_AGENT };

		std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
		succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
		if (!succes)
		{
			std::cout << "AgentsVec not found while updating EvadeState\n";
			return;
		}

		AgarioAgent* pClosestCloseBiggerAgent{ nullptr };
		FindBiggerAgent(pAgent, pAgentPtrVec, pClosestCloseBiggerAgent, NEW_MIN_DISTANCE_TO_BIGGER_AGENT);

		if (pClosestCloseBiggerAgent)
		{
			
			pAgent->SetToFlee(pClosestCloseBiggerAgent->GetPosition());
			// Debug
			//std::cout << "EvadeState\n";
			//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pClosestCloseBiggerAgent->GetPosition() - pAgent->GetPosition(), 15.0f, Elite::Color(0.9f, 0.2f, 0.2f));
			//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), NEW_MIN_DISTANCE_TO_BIGGER_AGENT, Elite::Color(0.9f, 0.2f, 0.2f), 0.9f);
		}



			//if (!pClosestCloseBiggerAgent); // no bigger agent nearby
	}

private:
	float m_Timer = 0.0f;
	const float MIN_DISTANCE_TO_BIGGER_AGENT = 20.0f;
};

class FindSmallerAgentState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };

		if (!succes)
			return;

		pAgent->SetToWander();
	}

	static bool FindSmallerAgent(AgarioAgent* pAgent, std::vector<AgarioAgent*>* pAgentPtrVec, AgarioAgent* pSmallerAgent, const float MIN_DISTANCE_TO_SMALLER_AGENT = 25.0f)
	{
		AgarioAgent* pClosestCloseSmallerAgent{ nullptr };
		for (AgarioAgent* pOtherAgent : *pAgentPtrVec)
		{
			//if (!(pOtherAgent->GetMass() < pAgent->GetMass())) continue;
			if (!(pAgent->GetRadius() - pOtherAgent->GetRadius() > 1.0f)) continue;
			if (Elite::Distance(pAgent->GetPosition(), pOtherAgent->GetPosition()) > MIN_DISTANCE_TO_SMALLER_AGENT) continue;

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
			pSmallerAgent = pClosestCloseSmallerAgent;
			return true;
		}
		return false;
	}


	virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };
		if (!succes)
		{
			std::cout << "Agent not found while updating FindSmallerAgentState\n";
			return;
		}

		const float NEW_MIN_DISTANCE_TO_SMALLER_AGENT{ pAgent->GetRadius() + MIN_DISTANCE_TO_SMALLER_AGENT };

		std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
		succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
		if (!succes)
		{
			std::cout << "AgentsVec not found while updating FindSmallerAgentState\n";
			return;
		}

		AgarioAgent* pClosestCloseSmallerAgent{ nullptr };
		FindSmallerAgent(pAgent, pAgentPtrVec, pClosestCloseSmallerAgent, NEW_MIN_DISTANCE_TO_SMALLER_AGENT);

		if (pClosestCloseSmallerAgent)
		{
			std::cout << "FindSmallerAgentState\n";
			pAgent->SetToSeek(pClosestCloseSmallerAgent->GetPosition());
			// Debug
			//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pClosestCloseSmallerAgent->GetPosition() - pAgent->GetPosition(), 15.0f, Elite::Color(0.2f, 0.9f, 0.2f));
			//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), NEW_MIN_DISTANCE_TO_SMALLER_AGENT, Elite::Color(0.2f, 0.9f, 0.2f), 0.9f);
		}



		//if (!pClosestCloseSmallerAgent); // no smaller agent nearby
	}

private:
	float m_Timer = 0.0f;
	const float MIN_DISTANCE_TO_SMALLER_AGENT = 35.0f;
};

class MoveAwayFromBoundariesState : public Elite::FSMState
{
public:
	virtual void OnEnter(Elite::Blackboard* pBlackboard) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };

		if (!succes)
			return;

		pAgent->SetToWander();
	}

	virtual void Update(Elite::Blackboard* pBlackboard, float deltaT)
	{						
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };

		if (!succes)
		{
			std::cout << "Agent not found while updating MoveAwayFromBoundariesState\n";
			return;
		}

		const float NEW_MIN_DISTANCE_FROM_BOUNDARIES(pAgent->GetRadius() + MIN_DISTANCE_FROM_BOUNDARIES);

		float worldSize{ 0.0f };
		succes = pBlackboard->GetData("WorldSize", worldSize);
		if (!succes)
		{
			std::cout << "worldSize not found while updating MoveAwayFromBoundariesState\n";
			return;
		}

		float newWorldSize{worldSize - NEW_MIN_DISTANCE_FROM_BOUNDARIES };
		if (newWorldSize <= 0) return;

		Elite::Vector2 targetOffsetToAgentPos{0.0f, 0.0f};
		// Near top boundary
		if (pAgent->GetPosition().y > worldSize - NEW_MIN_DISTANCE_FROM_BOUNDARIES)
		{
			targetOffsetToAgentPos.y -= NEW_MIN_DISTANCE_FROM_BOUNDARIES;
		}

		// Near bottom boundary
		if (pAgent->GetPosition().y < -worldSize + NEW_MIN_DISTANCE_FROM_BOUNDARIES)
		{
			targetOffsetToAgentPos.y += NEW_MIN_DISTANCE_FROM_BOUNDARIES;
		}

		// Near left boundary
		if (pAgent->GetPosition().x < -worldSize + NEW_MIN_DISTANCE_FROM_BOUNDARIES)
		{
			targetOffsetToAgentPos.x += NEW_MIN_DISTANCE_FROM_BOUNDARIES;
		}

		// Near right boundary
		if (pAgent->GetPosition().x > worldSize - NEW_MIN_DISTANCE_FROM_BOUNDARIES)
		{
			targetOffsetToAgentPos.x -= MIN_DISTANCE_FROM_BOUNDARIES;
		}

		pAgent->SetToSeek(pAgent->GetPosition() + targetOffsetToAgentPos);

		// Debug
		//std::cout << "MoveAwayFromBoundariesState\n";
		//const float SMALL_NUMBER{ 0.1f };
		//if (targetOffsetToAgentPos.MagnitudeSquared() < SMALL_NUMBER * SMALL_NUMBER)
		//{
//			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), targetOffsetToAgentPos, 15.0f, Elite::Color(0.2f, 0.9f, 0.2f));
			//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), MIN_DISTANCE_FROM_BOUNDARIES, Elite::Color(0.2f, 0.9f, 0.2f), 0.9f);
		//}
	}

	static float MIN_DISTANCE_FROM_BOUNDARIES;

private:
	float m_Timer = 0.0f;
	
	

};


//-----------------
//---TRANSITIONS---
//-----------------

// TransitionStatePair
class TransitionToEvade : public Elite::FSMTransition
{
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{pBlackboard->GetData("Agent", pAgent)};
		if (!succes) return false;

		std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
		succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
		if (!succes) return false;


		return EvadeState::FindBiggerAgent(pAgent, pAgentPtrVec, nullptr);
	}
};

class TransitionFromEvade : public Elite::FSMTransition
{
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };
		if (!succes) return true;

		std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
		succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
		if (!succes) return true;


		return !EvadeState::FindBiggerAgent(pAgent, pAgentPtrVec, nullptr);
	}
};

class TransitionToFindSmaller : public Elite::FSMTransition
{
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };
		if (!succes) return false;

		std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
		succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
		if (!succes) return false;


		return FindSmallerAgentState::FindSmallerAgent(pAgent, pAgentPtrVec, nullptr);
	}
};

class TransitionFromFindSmaller : public Elite::FSMTransition
{
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };
		if (!succes) return true;

		std::vector<AgarioAgent*>* pAgentPtrVec{ nullptr };
		succes = pBlackboard->GetData("AgentsVec", pAgentPtrVec);
		if (!succes) return true;


		return !FindSmallerAgentState::FindSmallerAgent(pAgent, pAgentPtrVec, nullptr);
	}
};

class TransitionToMoveAwayFromBoundaries : public Elite::FSMTransition
{
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };
		if (!succes) return false;

		const float NEW_MIN_DISTANCE_FROM_BOUNDARIES(pAgent->GetRadius() + MoveAwayFromBoundariesState::MIN_DISTANCE_FROM_BOUNDARIES);

		float worldSize{ 0.0f };
		succes = pBlackboard->GetData("WorldSize", worldSize);
		if (!succes) return false;

		// Near top boundary
		if (pAgent->GetPosition().y > worldSize - NEW_MIN_DISTANCE_FROM_BOUNDARIES) return true;

		// Near bottom boundary
		if (pAgent->GetPosition().y < -worldSize + NEW_MIN_DISTANCE_FROM_BOUNDARIES) return true;

		// Near left boundary
		if (pAgent->GetPosition().x < -worldSize + NEW_MIN_DISTANCE_FROM_BOUNDARIES) return true;

		// Near right boundary
		if (pAgent->GetPosition().x > worldSize - NEW_MIN_DISTANCE_FROM_BOUNDARIES) return true;

		return false;
	}
};

class TransitionFromMoveAwayFromBoundaries : public Elite::FSMTransition
{
	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		bool succes{ pBlackboard->GetData("Agent", pAgent) };
		if (!succes) return true;

		const float NEW_MIN_DISTANCE_FROM_BOUNDARIES(pAgent->GetRadius() + MoveAwayFromBoundariesState::MIN_DISTANCE_FROM_BOUNDARIES);

		float worldSize{ 0.0f };
		succes = pBlackboard->GetData("WorldSize", worldSize);
		if (!succes) return true;

		// Near top boundary
		if (pAgent->GetPosition().y > worldSize - NEW_MIN_DISTANCE_FROM_BOUNDARIES) return false;

		// Near bottom boundary
		if (pAgent->GetPosition().y < -worldSize + NEW_MIN_DISTANCE_FROM_BOUNDARIES) return false;

		// Near left boundary
		if (pAgent->GetPosition().x < -worldSize + NEW_MIN_DISTANCE_FROM_BOUNDARIES) return false;

		// Near right boundary
		if (pAgent->GetPosition().x > worldSize - NEW_MIN_DISTANCE_FROM_BOUNDARIES) return false;

		return true;
	}
};

//class TransitionToFindFood : public Elite::FSMTransition
//{
//	virtual bool ToTransition(Elite::Blackboard* pBlackboard) const override
//	{
//		std::vector<AgarioFood*>* pFoodPtrVector{ nullptr };
//
//		bool succes{ pBlackboard->GetData("FoodVec", pFoodPtrVector) };
//		if (!succes) return false;
//		return true;
//	}
//};

#endif