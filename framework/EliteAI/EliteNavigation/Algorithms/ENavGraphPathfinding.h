#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Elite::Vector2> FindPath(Elite::Vector2 startPos, Elite::Vector2 endPos, Elite::NavGraph* pNavGraph, std::vector<Elite::Vector2>& debugNodePositions, std::vector<Elite::Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Elite::Vector2> finalPath{};

			//Get the startTriangle and endTriangle	
			const Triangle* pStartTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			const Triangle* pEndTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };

			if (!pStartTriangle || !pEndTriangle) return finalPath;

			if (pStartTriangle == pEndTriangle)
			{	
				finalPath.push_back(endPos);				

				return finalPath;
			}


			

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			std::shared_ptr<Elite::IGraph<NavGraphNode, GraphConnection2D>> pClonedGraph{ pNavGraph->Clone() };

			//Create extra node for the Start Node (Agent's position)
			NavGraphNode* pNewStartNode{ new NavGraphNode(pClonedGraph.get()->GetNextFreeNodeIndex(), -1, startPos) };
			pClonedGraph->AddNode(pNewStartNode);
			for (int triangleLineIdx : pStartTriangle->metaData.IndexLines)
			{
				int nodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(triangleLineIdx) };
				if (nodeIdx == invalid_node_index) continue;

				GraphConnection2D* pNewConnection{
					new GraphConnection2D(
						pNewStartNode->GetIndex()
						, nodeIdx
						, Distance(pNewStartNode->GetPosition(), pNavGraph->GetNode(nodeIdx)->GetPosition()))};
				pClonedGraph->AddConnection(pNewConnection);
			}


			//Create extra node for the endNode
			NavGraphNode* pNewEndNode{ new NavGraphNode(pClonedGraph.get()->GetNextFreeNodeIndex(), -1, endPos) };
			pClonedGraph->AddNode(pNewEndNode);
			for (int triangleLineIdx : pEndTriangle->metaData.IndexLines)
			{
				int nodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(triangleLineIdx) };
				if (nodeIdx == invalid_node_index) continue;

				GraphConnection2D* pNewConnection{
					new GraphConnection2D(
						pNewEndNode->GetIndex()
						, nodeIdx
						, Distance(pNewEndNode->GetPosition(), pNavGraph->GetNode(nodeIdx)->GetPosition())) };
				pClonedGraph->AddConnection(pNewConnection);
			}


			//Run A star on new graph
			AStar<NavGraphNode, GraphConnection2D> pathfinder{ pClonedGraph.get(), HeuristicFunctions::Euclidean };
			auto path = pathfinder.FindPath(pNewStartNode, pNewEndNode);
			finalPath.reserve(path.size());
			finalPath.resize(path.size());
			for (size_t idx = 0; idx < path.size(); ++idx)
			{
				finalPath[idx] = path[idx]->GetPosition();
			}


			//OPTIONAL BUT ADVICED: Debug Visualisation

			debugNodePositions.clear();
			debugNodePositions.reserve(debugNodePositions.size() + finalPath.size());
			for (const Elite::Vector2& nodePos : finalPath)
			{
				debugNodePositions.push_back(nodePos);
			}


			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto portals = SSFA::FindPortals(path, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(portals);

			return finalPath;
		}
	};
}
