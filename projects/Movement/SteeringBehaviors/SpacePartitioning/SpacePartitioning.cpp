#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"
#include <algorithm>

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
	, m_CellWidth{ width /cols}
	, m_CellHeight{height / rows}
	, m_pDebugAgent{nullptr}
	, m_QueryRadius{10.f}

{
	InitialiseCells();
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	if (agent == nullptr) return;
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	
	if ((
		PositionToIndex(agent->GetPosition())
		!=
		PositionToIndex(oldPos))) // if not in same cell
	{
		// unregister in old cell
		m_Cells[PositionToIndex(oldPos)].agents.remove(agent);

		
		// register in new cell
		AddAgent(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	m_NrOfNeighbors = 0;

	Elite::Vector2 agentPos{ agent->GetPosition() };
	std::pair<int, int> agentRowColIdxs{ int(agentPos.y / (m_CellHeight)),  int(agentPos.x / (m_CellWidth)) };

	Elite::Vector2 boundingBoxBottomLeft{ agent->GetPosition().x - queryRadius, agent->GetPosition().y - queryRadius };
	Elite::Vector2 boundingBoxTopRight{ agent->GetPosition().x + queryRadius, agent->GetPosition().y + queryRadius };
	int nrCellsLeft{ agentRowColIdxs.second - int(boundingBoxBottomLeft.x / (m_CellWidth)) };
	int nrCellsRight{ int(boundingBoxTopRight.x / (m_CellWidth)) - agentRowColIdxs.second };
	int nrCellsBelow{ agentRowColIdxs.first - int(boundingBoxBottomLeft.y / (m_CellWidth)) };
	int nrCellsAbove{ int(boundingBoxTopRight.y / (m_CellWidth)) - agentRowColIdxs.first };
	std::pair<int, int> leftBottomIdxs{ agentRowColIdxs.first - nrCellsBelow, agentRowColIdxs.second - nrCellsLeft };

	for (int i = 0; i < nrCellsBelow + nrCellsAbove + 1; ++i) // rows
	{
		for (int i2 = 0; i2 < nrCellsLeft + nrCellsRight + 1; ++i2) // cols
		{
			int rowIdx{ leftBottomIdxs.first + i };
			int colIdx{ leftBottomIdxs.second + i2 };
			int currentIdx{ colIdx + rowIdx * m_NrOfCols };
			if (currentIdx < 0 || currentIdx >= (m_NrOfRows * m_NrOfCols)) { continue; } // If we're trying to check a cell that doesn't exist, we can skip it
			
			for (SteeringAgent* pAgent : m_Cells[currentIdx].agents)
			{
				if (pAgent != agent)
				{
					if
						(Elite::DistanceSquared(pAgent->GetPosition(), agent->GetPosition()) <= queryRadius * queryRadius)
					{
						if (m_NrOfNeighbors < int(m_Neighbors.size()))
						{
							m_Neighbors[m_NrOfNeighbors] = pAgent;
							++m_NrOfNeighbors;
						}
						else
						{
							m_Neighbors.push_back(pAgent);
							++m_NrOfNeighbors;
						}
					}
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	
	Elite::Color cellColor{0.8f, 0.3f, 0.3f, 1};
	Elite::Polygon cellPolygon{};

	for (const Cell& cell : m_Cells)
	{
		// Draw Cells
		cellPolygon = Elite::Polygon(cell.GetRectPoints());
		DEBUGRENDERER2D->DrawPolygon(&cellPolygon, cellColor, 0.f);		

		// Draw the number of agents in each cell
		DEBUGRENDERER2D->DrawString(cell.GetRectPoints()[1], std::to_string(cell.agents.size()).c_str());

	}

	Elite::Color queryColor{0.7f, 0.7f, 0.7f, 1};

	// Highlight one agent's neighbors + render its radius and bounding box
	std::vector<Elite::Vector2> boundingBoxPoints;
	boundingBoxPoints.push_back({
		m_pDebugAgent->GetPosition().x - (m_QueryRadius),
		m_pDebugAgent->GetPosition().y - (m_QueryRadius) });
	boundingBoxPoints.push_back({
		m_pDebugAgent->GetPosition().x + (m_QueryRadius),
		m_pDebugAgent->GetPosition().y - (m_QueryRadius) });
	boundingBoxPoints.push_back({
		m_pDebugAgent->GetPosition().x +  (m_QueryRadius),
		m_pDebugAgent->GetPosition().y + (m_QueryRadius) });
	boundingBoxPoints.push_back({
		m_pDebugAgent->GetPosition().x - (m_QueryRadius),
		m_pDebugAgent->GetPosition().y + (m_QueryRadius) });
	

	DEBUGRENDERER2D->DrawCircle(m_pDebugAgent->GetPosition(), m_QueryRadius, queryColor, 0);
	DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon{ boundingBoxPoints }, queryColor);
	
	// neighbours of this agent are already being rendered in TheFlock.cpp

	Elite::Color neighborCellsColor{ 0.2f, 0.2f, 0.7f, 1 };
	
	Elite::Vector2 agentPos{ m_pDebugAgent->GetPosition() };
	std::pair<int, int> agentRowColIdxs{ int(agentPos.y / (m_CellHeight)),  int(agentPos.x / (m_CellWidth)) };
	
	Elite::Vector2 boundingBoxBottomLeft{ m_pDebugAgent->GetPosition().x - m_QueryRadius, m_pDebugAgent->GetPosition().y - m_QueryRadius };
	Elite::Vector2 boundingBoxTopRight{ m_pDebugAgent->GetPosition().x + m_QueryRadius, m_pDebugAgent->GetPosition().y + m_QueryRadius };
	int nrCellsLeft{ agentRowColIdxs.second - int(boundingBoxBottomLeft.x / (m_CellWidth)) };
	int nrCellsRight{ int(boundingBoxTopRight.x / (m_CellWidth)) - agentRowColIdxs.second };
	int nrCellsBelow{ agentRowColIdxs.first - int(boundingBoxBottomLeft.y / (m_CellWidth)) };
	int nrCellsAbove{ int(boundingBoxTopRight.y / (m_CellWidth)) - agentRowColIdxs.first };
	std::pair<int, int> leftBottomIdxs{ agentRowColIdxs.first - nrCellsBelow, agentRowColIdxs.second - nrCellsLeft };

	for (int i = 0; i < nrCellsBelow + nrCellsAbove + 1; ++i) // rows
	{
		for (int i2 = 0; i2 < nrCellsLeft + nrCellsRight + 1; ++i2) // cols
		{
			int rowIdx{ leftBottomIdxs.first + i };
			int colIdx{ leftBottomIdxs.second + i2 };
			int currentIdx{ colIdx + rowIdx * m_NrOfCols };
			if (currentIdx < 0 || currentIdx >= (m_NrOfRows * m_NrOfCols)) { continue; } // If we're trying to check a cell that doesn't exist, we can skip it

			cellPolygon = Elite::Polygon(m_Cells[currentIdx].GetRectPoints());
			DEBUGRENDERER2D->DrawPolygon(&cellPolygon, neighborCellsColor, -0.1f);			
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	if (pos.y >= m_SpaceHeight || pos.y <= 0
		|| pos.x >= m_SpaceWidth || pos.x <= 0)
	{
		return 0;
	}

	// row idx
	int rowIdx{ int(pos.y / (m_CellHeight)) };

	// col idx
	int colIdx{ int(pos.x / (m_CellWidth)) };

	// final idx
	int finalIdx{ colIdx + rowIdx * m_NrOfCols };

	return finalIdx;

	/*
	(3, 0) (3, 1) (3, 2) (3, 3)			12 13 14 15
	(2, 0) (2, 1) (2, 2) (2, 3)			8  9  10 11
	(1, 0) (1, 1) (1, 2) (1, 3)			4  5  6  7
	(0, 0) (0, 1) (0, 2) (0, 3)			0  1  2  3
	*/
}

void CellSpace::InitialiseCells()
{
	// These cells are made from left bottom to top right (left bottom idx = [0][0])
	m_Cells.reserve(m_NrOfRows * m_NrOfCols);
	for (int i = 0; i < m_NrOfRows; ++i) // for every row (i = index)
	{
		for (int i2 = 0; i2 < m_NrOfCols; ++i2) // for every column (i2 = index)
		{
			m_Cells.push_back(Cell{0 + i2*(m_SpaceWidth/m_NrOfCols) , 0 + i*(m_SpaceHeight/m_NrOfRows), m_CellWidth, m_CellHeight});
		}
	}
}

void CellSpace::SetDebugAgent(SteeringAgent* debugAgent)
{
	m_pDebugAgent = debugAgent;
}

void CellSpace::SetNeighbordhoodRadius(float neighBorhoodRadius)
{
	m_QueryRadius = neighBorhoodRadius;
}