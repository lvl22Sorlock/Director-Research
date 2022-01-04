//=== General Includes ===
#include "stdafx.h"
#include "EFiniteStateMachine.h"

Elite::FiniteStateMachine::FiniteStateMachine(Elite::FSMState* startState, Elite::Blackboard* pBlackboard)
    : m_pCurrentState(nullptr),
    m_pBlackboard(pBlackboard)
{
    ChangeState(startState);
}

Elite::FiniteStateMachine::~FiniteStateMachine()
{
    SAFE_DELETE(m_pBlackboard);
}

void Elite::FiniteStateMachine::AddTransition(Elite::FSMState* startState, Elite::FSMState* toState, Elite::FSMTransition* transition)
{
    auto it = m_Transitions.find(startState);
    if (it == m_Transitions.end())
    {
        m_Transitions[startState] = Transitions();
    }
   
    m_Transitions[startState].push_back(std::make_pair(transition, toState));
}

void Elite::FiniteStateMachine::Update(float deltaTime)
{
    //TODO 4: Look if 1 or more transition exists for the current state that we are in
    //Tip: Check the transitions map for a TransitionState pair
    auto currentStateTransitionsIterator = m_Transitions.find(m_pCurrentState);
    if (currentStateTransitionsIterator == m_Transitions.end() || currentStateTransitionsIterator->second.size() < 1)
    {
        m_pCurrentState->Update(m_pBlackboard, deltaTime);
        return;
    }

    //TODO 5: if a TransitionState exists
  
        //TODO 6: Loop over all the TransitionState pairs
        for (const TransitionStatePair& transitionStatePair : currentStateTransitionsIterator->second)
        {
            //TODO 7: If a ToTransition returns true => transition to the new corresponding state
            if (transitionStatePair.first->ToTransition(m_pBlackboard))
            {
                //m_pCurrentState = transitionStatePair.second;
                ChangeState(transitionStatePair.second);
            }
        }
        


    //TODO 8: Update the current state (if one exists ;-))
    if (m_pCurrentState)
    {
        m_pCurrentState->Update(m_pBlackboard, deltaTime);
    }
}

Elite::Blackboard* Elite::FiniteStateMachine::GetBlackboard() const
{
    return m_pBlackboard;
}

void Elite::FiniteStateMachine::ChangeState(FSMState* newState)
{
    //TODO 1. If currently in a state => make sure the OnExit of that state gets called
    if (m_pCurrentState)
    {
        m_pCurrentState->OnExit(m_pBlackboard);
    }

    //TODO 2. Change the current state to the new state
    m_pCurrentState = newState;

    //TODO 3. Call the OnEnter of the new state
    //newState->OnEnter(m_pBlackboard);
    if (m_pCurrentState)
    {
        m_pCurrentState->OnEnter(m_pBlackboard);
    }
        
}
