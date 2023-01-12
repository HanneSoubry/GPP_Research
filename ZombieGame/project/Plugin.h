#pragma once

#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "EBlackboard.h"
#include "StatesAndTransitions.h"
#include "HelperStructs.h"

class IBaseInterface;
class IExamInterface;

// Steering
//class Seek;
//class Flee;
//class Face;
//class Wander;
//class Evade;

class Plugin :public IExamPlugin
{ 
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	std::vector<HouseInfo> GetHousesInFOV() const;
	std::vector<EntityInfo> GetEntitiesInFOV() const;
	void SortEntities(const std::vector<EntityInfo>& entities, 
						std::vector<EntityInfo>& items, 
						std::vector<EntityInfo>& enemies,
						std::vector<EntityInfo>& purgeZones) const;

	// Debug drawing toggles
	bool m_ShowExploringCheckpoints{ true };
	bool m_ShowHouseCheckpoints{ false };

	// DEBUG Steering
	Steering::Seek* m_pSeek;
	Steering::Flee* m_pFlee;
	Steering::Face* m_pFace;
	Steering::Wander* m_pWander;
	Steering::Evade* m_pEvade;

	// DecisionMaking
	Elite::Blackboard* CreateBlackBoard();

	Elite::FiniteStateMachine* m_pStateMachine;
	std::vector<Elite::FSMState*> m_pStates{};
	std::vector<Elite::FSMCondition*> m_pConditions{};

	// memory
	std::vector<AgentMemoryHouse>* m_pMemoryHouses{ new std::vector<AgentMemoryHouse>{} };
	std::vector<AgentMemoryInventory>* m_pMemoryInventory{ new std::vector<AgentMemoryInventory>{} };
	std::vector<eItemType> m_IdealInventory{};
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}