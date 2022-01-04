#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{		
		vector<T_NodeType*> path;
		//Creating open list
		vector<NodeRecord> openList;
		//Creating closed list
		vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		NodeRecord startRecord{};
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.costSoFar = 0;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(startRecord);


		while (!openList.empty())
		{
			// A Get connection with lowest f-score
			currentRecord = openList[0];
			for (const NodeRecord& nodeRecord : openList)
			{
				if (nodeRecord < currentRecord)
				{
					currentRecord = nodeRecord;
				}
			}
			

			// B if that connection leads to the end node break
			if (currentRecord.pNode == pGoalNode)
			{				
				break;
			}

			// C get all the connections of the NodeRecord's node
			for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				float totalCostSoFarG{currentRecord.costSoFar + connection->GetCost()};
				bool shouldContinueToNextConnection = false;

				// Check if any connections lead to a node already on the closed list
				for (NodeRecord& nodeRecord : closedList)
				{
					if (shouldContinueToNextConnection) break;

					if (connection->GetTo() == nodeRecord.pNode->GetIndex())
					{
						if (nodeRecord.costSoFar < totalCostSoFarG)
						{
							shouldContinueToNextConnection = true;
							continue;
						}
						else
							closedList.erase(std::find(closedList.begin(), closedList.end(), nodeRecord));
						// F Any expensive connection that may have existed should be removed
					}
				}
				if (shouldContinueToNextConnection) continue;

				// Check if any connections lead to a node already on the open list
				for (NodeRecord& nodeRecord : openList)
				{
					if (shouldContinueToNextConnection) break;

					if (connection->GetTo() == nodeRecord.pNode->GetIndex())
					{
						if (nodeRecord.costSoFar < totalCostSoFarG)
						{
							shouldContinueToNextConnection = true;
							continue;
						}
						else
							openList.erase(std::find(openList.begin(), openList.end(), nodeRecord));
						// F Any expensive connection that may have existed should be removed
					}
				}
				if (shouldContinueToNextConnection) continue;

				

				// F Create a new nodeRecord and add it to the openList

				NodeRecord newRecord{};
				newRecord.pNode = m_pGraph->GetNode(connection->GetTo());
				newRecord.costSoFar = totalCostSoFarG;
				newRecord.pConnection = connection;
				newRecord.estimatedTotalCost = totalCostSoFarG + GetHeuristicCost(newRecord.pNode, pGoalNode);
				openList.push_back(newRecord);
			}

			// G Remove NodeRecord from the openList and add it to the closedList
			openList.erase(std::find(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);

		}

		// 3. Reconstruct path from last connection to start node
		NodeRecord currentPathRecord = currentRecord;
		while (currentPathRecord.pNode != pStartNode)
		{
			path.push_back(currentPathRecord.pNode);

			// find node that points to currentNode
			for (const NodeRecord& record : closedList)
			{
				if (currentPathRecord.pConnection->GetFrom() == record.pNode->GetIndex())
				{
					currentPathRecord = record;
					break;
				}
			}
		}


		path.push_back(pStartNode);

		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}