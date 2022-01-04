#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	for (const Line* pLine : m_pNavMeshPolygon->GetLines())	// loop over all lines
	{	// for each line

		// Check if that line is connected to another triangle
		if (!(m_pNavMeshPolygon->GetTrianglesFromLineIndex(pLine->index).size() >= 2))
		{
			continue;
		}

		Elite::Vector2 newNodePos{(pLine->p1 + pLine->p2)/2.0f};
		NavGraphNode* newGraphNode = new NavGraphNode(GetNextFreeNodeIndex(), pLine->index, newNodePos);
		AddNode(newGraphNode);
	}

	for (const Triangle* pTriangle : m_pNavMeshPolygon->GetTriangles()) // loop over all triangles
	{
		std::vector<int> triangleNodeIdxs;


		for (int i = 0; i < 3; ++i)
		{			
			int nodeIndex{GetNodeIdxFromLineIdx(pTriangle->metaData.IndexLines[i])};

			if (nodeIndex == invalid_node_index) continue;

			triangleNodeIdxs.push_back(nodeIndex);
		}

		//2. Create connections now that every node is created
		GraphConnection2D* newConnection{ nullptr };
		switch (triangleNodeIdxs.size())
		{
		case 2:
			newConnection = new GraphConnection2D(triangleNodeIdxs[0], triangleNodeIdxs[1]);
			AddConnection(newConnection);
			break;
		case 3:
			newConnection = new GraphConnection2D(triangleNodeIdxs[0], triangleNodeIdxs[1]);
			AddConnection(newConnection);
			newConnection = new GraphConnection2D(triangleNodeIdxs[1], triangleNodeIdxs[2]);
			AddConnection(newConnection);
			newConnection = new GraphConnection2D(triangleNodeIdxs[2], triangleNodeIdxs[0]);
			AddConnection(newConnection);
			break;
		}

		
	}

	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();	
}

