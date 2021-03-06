#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{
		std::queue<T_NodeType*> openList; // nodes to check
		std::map<T_NodeType*, T_NodeType*> closedList; // Already checked nodes AND history of made connections
		
		openList.push(pStartNode);

		while (!openList.empty())
		{
			T_NodeType* pCurrentNode = openList.front(); // taking a node from the openList
			openList.pop(); // Removing said node

			if (pCurrentNode == pDestinationNode)
			{
				break; // Found end node=> Stop this madness
			}

			for (auto con : m_pGraph->GetNodeConnections(pCurrentNode->GetIndex()))
			{
				T_NodeType* pNextNode = m_pGraph->GetNode(con->GetTo());
				if (closedList.find(pNextNode) == closedList.end())
				{
					//We did not find this node in the closedList
					openList.push(pNextNode);
					closedList[pNextNode] = pCurrentNode;
				}
			}
		}

		//Start tracking back from the end node
		vector<T_NodeType*> path;
		T_NodeType* pCurrentNode = pDestinationNode;
		while (pCurrentNode != pStartNode)
		{
			path.push_back(pCurrentNode);
			pCurrentNode = closedList[pCurrentNode];
		}
		path.push_back(pStartNode); // doesn't necessarily need to be part of the path
		std::reverse(path.begin(), path.end()); // Reversing the path
					// (you could also skip this step if you instead do path.push_front for every node

		return path;

		//return vector<T_NodeType*>();
	}
}

