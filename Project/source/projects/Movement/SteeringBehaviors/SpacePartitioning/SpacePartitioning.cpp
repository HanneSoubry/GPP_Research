#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

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
{
	m_CellHeight = m_SpaceHeight / m_NrOfRows;
	m_CellWidth = m_SpaceWidth / m_NrOfCols;

	for (int r{}; r < rows; r++)
	{
		for (int c{}; c < cols; c++)
		{
			m_Cells.push_back(Cell{ c * m_CellWidth, m_SpaceHeight - (r + 1) * m_CellHeight, m_CellWidth, m_CellHeight });	// left to right, top to bottom
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int index{ PositionToIndex(agent->GetPosition()) };
	m_Cells[index].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	int oldIndex{ PositionToIndex(oldPos) };
	int newIndex{ PositionToIndex(agent->GetPosition()) };

	if (oldIndex != newIndex)
	{
		m_Cells[oldIndex].agents.remove(agent);
		m_Cells[newIndex].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	m_NrOfNeighbors = 0;

	Elite::Vector2 agentPos{ agent->GetPosition() };
	Elite::Rect neighborhood{ {agentPos.x - queryRadius, agentPos.y - queryRadius}, 2 * queryRadius, 2 * queryRadius };

	for (Cell& c : m_Cells)
	{
		if (Elite::IsOverlapping(neighborhood, c.boundingBox))
		{
			for (SteeringAgent* pAgent : c.agents)
			{
				float distance{ (pAgent->GetPosition() - agentPos).Magnitude() };
				if (distance <= queryRadius)
				{
					m_Neighbors[m_NrOfNeighbors] = pAgent;
					++m_NrOfNeighbors;
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	for (int r{ 1 }; r < (m_NrOfRows); r++)
	{
		DEBUGRENDERER2D->DrawSegment({ 0, r * m_CellHeight }, { m_SpaceWidth, r * m_CellHeight }, { 1,0,0 });
	}

	for (int c{ 1 }; c < (m_NrOfCols); c++)
	{
		DEBUGRENDERER2D->DrawSegment({ c * m_CellWidth, 0 }, { c * m_CellWidth, m_SpaceHeight }, { 1,0,0 });
	}

	for (Cell c : m_Cells)
	{
		DEBUGRENDERER2D->DrawString(Elite::Vector2{ c.boundingBox.bottomLeft.x + 1, c.boundingBox.bottomLeft.y + c.boundingBox.height - 1 }, std::to_string(c.agents.size()).c_str() );
	}
}

void CellSpace::RenderNeighborhood(SteeringAgent* agent, float queryRadius) const
{
	Elite::Vector2 agentPos{ agent->GetPosition() };
	Elite::Rect neighborhood{ {agentPos.x - queryRadius, agentPos.y - queryRadius}, 2 * queryRadius, 2 * queryRadius };

	for (const Cell& c : m_Cells)
	{
		if (Elite::IsOverlapping(neighborhood, c.boundingBox))
		{
			std::vector<Elite::Vector2> cellPoints{ c.GetRectPoints() };
			DEBUGRENDERER2D->DrawSegment(cellPoints[0], cellPoints[1], { 1,1,0 });
			DEBUGRENDERER2D->DrawSegment(cellPoints[1], cellPoints[2], { 1,1,0 });
			DEBUGRENDERER2D->DrawSegment(cellPoints[2], cellPoints[3], { 1,1,0 });
			DEBUGRENDERER2D->DrawSegment(cellPoints[3], cellPoints[0], { 1,1,0 });
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int rowNr{ m_NrOfRows - static_cast<int>(pos.y / m_CellHeight) - 1 };
	int colNr{ static_cast<int>(pos.x / m_CellWidth) };

	// fix out of bound
	if (pos.x >= m_SpaceWidth)
		colNr = m_NrOfCols - 1;
	else if (pos.x <= 0)
		colNr = 0;
	if (pos.y >= m_SpaceHeight)
		rowNr = 0;
	else if (pos.y <= 0)
		rowNr = m_NrOfRows - 1;

	int index{ rowNr * m_NrOfCols + colNr };
	return index;
}