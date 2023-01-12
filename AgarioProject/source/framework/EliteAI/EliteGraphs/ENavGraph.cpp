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
	for (Elite::Line* line : m_pNavMeshPolygon->GetLines())
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() == 2)	// 2 triangles on this line
		{
			Vector2 pos{ line->p1 + (line->p2 - line->p1) / 2 };
			NavGraphNode* newNode = new NavGraphNode{ GetNrOfNodes(), line->index, pos };
			AddNode(newNode);
		}
	}

	//2. Create connections now that every node is created
	for (Elite::Triangle* triangle : m_pNavMeshPolygon->GetTriangles())
	{
		int nrValidNodes{};
		int validNodeIndicies[3]{};

		for (int lineIndex : triangle->metaData.IndexLines)
		{
			int nodeIndex{ GetNodeIdxFromLineIdx(lineIndex) };
			if (nodeIndex != invalid_node_index)
			{
				validNodeIndicies[nrValidNodes] = nodeIndex;
				++nrValidNodes;
			}
		}

		if (nrValidNodes == 2)
		{
			GraphConnection2D* connection = new GraphConnection2D{ validNodeIndicies[0], validNodeIndicies[1]};
			AddConnection(connection);
		}
		else if (nrValidNodes == 3)
		{
			GraphConnection2D* connection1 = new GraphConnection2D{ validNodeIndicies[0], validNodeIndicies[1] };
			GraphConnection2D* connection2 = new GraphConnection2D{ validNodeIndicies[1], validNodeIndicies[2] };
			GraphConnection2D* connection3 = new GraphConnection2D{ validNodeIndicies[2], validNodeIndicies[0] };
			AddConnection(connection1);
			AddConnection(connection2);
			AddConnection(connection3);
		}
	}

	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();
}

