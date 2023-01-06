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
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const Triangle* pStartTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			if (pStartTriangle == nullptr)
				return finalPath;
			const Triangle* pEndTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };
			if (pEndTriangle == nullptr)
				return finalPath;

			if (pStartTriangle == pEndTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto pGraphCopy{ pNavGraph->Clone() };
			std::vector<NavGraphNode*> nodes{};

			//Create extra node for the Start Node (Agent's position)
			int newNodeIndex{ pGraphCopy->GetNrOfNodes() };
			pGraphCopy->AddNode(new NavGraphNode(newNodeIndex, invalid_node_index, startPos));

			for (int lineIndex : pStartTriangle->metaData.IndexLines)
			{
				const int connectionNodeIndex{ pNavGraph->GetNodeIdxFromLineIdx(lineIndex) };
				if (connectionNodeIndex != invalid_node_index)
				{
					const float distance{ Elite::Distance(startPos, pGraphCopy->GetNode(connectionNodeIndex)->GetPosition())};
					pGraphCopy->AddConnection(new GraphConnection2D{ newNodeIndex, connectionNodeIndex, distance });
				}
			}

			//Create extra node for the endNode
			++newNodeIndex;
			pGraphCopy->AddNode(new NavGraphNode(newNodeIndex, invalid_node_index, endPos));
			
			for (int lineIndex : pEndTriangle->metaData.IndexLines)
			{
				const int connectionNodeIndex{ pNavGraph->GetNodeIdxFromLineIdx(lineIndex) };
				if (connectionNodeIndex != invalid_node_index)
				{
					const float distance{ Elite::Distance(endPos, pGraphCopy->GetNode(connectionNodeIndex)->GetPosition()) };
					pGraphCopy->AddConnection(new GraphConnection2D{ newNodeIndex, connectionNodeIndex, distance });
				}
			}

			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(pGraphCopy.get(), HeuristicFunctions::Chebyshev);
			auto startNode = pGraphCopy->GetNode(newNodeIndex - 1);
			auto endNode = pGraphCopy->GetNode(newNodeIndex);

			std::vector<NavGraphNode*> path = pathfinder.FindPath(startNode, endNode);

			debugNodePositions.clear();
			for (NavGraphNode* node : path)
			{
				Vector2 pos{ node->GetPosition() };
				finalPath.push_back(pos);
				debugNodePositions.push_back(pos);
			}

			//OPTIONAL BUT ADVICED: Debug Visualisation

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			std::vector<Elite::Portal> portals = SSFA::FindPortals(path, pNavGraph->GetNavMeshPolygon());
			debugPortals = portals;
			finalPath = SSFA::OptimizePortals(portals);

			return finalPath;
		}
	};
}
