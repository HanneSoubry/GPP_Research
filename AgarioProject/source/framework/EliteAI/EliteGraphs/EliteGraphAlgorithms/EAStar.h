#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

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
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		currentRecord.pNode = pStartNode;
		currentRecord.pConnection = nullptr;
		currentRecord.estimatedTotalCost = GetHeuristicCost(currentRecord.pNode, pGoalNode);

		openList.push_back(currentRecord);

		while (openList.empty() == false)
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());

			if (currentRecord.pNode == pGoalNode)
				break;

			for (auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				T_NodeType* pNextNode = m_pGraph->GetNode(connection->GetTo());
				float costSoFar{ currentRecord.costSoFar + connection->GetCost() };

				bool inClosedList{ false };
				bool costTooHigh{ false };

				for (auto& record : closedList)
				{
					if (pNextNode == record.pNode)
					{
						inClosedList = true;
						if (costSoFar < record.costSoFar)
						{
							closedList.erase(std::remove(closedList.begin(), closedList.end(), record));
						}
						else
							costTooHigh = true;

						break;
					}
				}
				if (costTooHigh)
					continue;
				else if (inClosedList == false)
				{
					for (auto& record : openList)
					{
						if (pNextNode == record.pNode)
						{
							if (costSoFar < record.costSoFar)
							{
								openList.erase(std::remove(openList.begin(), openList.end(), record));
							}
							else
								costTooHigh = true;

							break;
						}
					}
				}
				
				if (costTooHigh)
					continue;

				NodeRecord newRecord{};
				newRecord.pNode = pNextNode;
				newRecord.pConnection = connection;
				newRecord.costSoFar = costSoFar;
				newRecord.estimatedTotalCost = newRecord.costSoFar + GetHeuristicCost(newRecord.pNode, pGoalNode);

				openList.push_back(newRecord);			
			}

			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);
		}

		if (currentRecord.pNode == pGoalNode)
		{
			while (currentRecord.pNode != pStartNode)
			{
				path.push_back(currentRecord.pNode);

				for (auto& record : closedList)
				{
					if (record.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom()))
					{
						currentRecord = record;
						break;
					}
				}
			}
			path.push_back(currentRecord.pNode);

			std::reverse(path.begin(), path.end());
		}

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}