
#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"

#include "EGoalOrientedActionPlanning.h"

using namespace std;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "MinionExam";
	info.Student_FirstName = "Hanne";
	info.Student_LastName = "Soubry";
	info.Student_Class = "2DAE15";

	// needs the interface
	
	// DecisionMaking
	// create blackboard
	Elite::Blackboard* pBlackBoard = CreateBlackBoard();

	// states
	Elite::FSMState* exploringState = new FSMStates::ExploringState;
	Elite::FSMState* houseState = new FSMStates::EnterHouseState;
	Elite::FSMState* itemState = new FSMStates::ItemHandlingState;
	Elite::FSMState* healingState = new FSMStates::HealingState;
	Elite::FSMState* purgeZoneState = new FSMStates::RunFromPurgeZoneState;
	Elite::FSMState* fightingState = new FSMStates::EnemyHandlingState;

	m_pStates.push_back(exploringState);
	m_pStates.push_back(houseState);
	m_pStates.push_back(itemState);
	m_pStates.push_back(healingState);
	m_pStates.push_back(purgeZoneState);
	m_pStates.push_back(fightingState);

	// conditions
	Elite::FSMCondition* foundHouseCondition = new FSMConditions::FoundHouseCondition;
	Elite::FSMCondition* houseSearchDoneCondition = new FSMConditions::HouseSearchDoneCondition;
	Elite::FSMCondition* itemFoundCondition = new FSMConditions::ItemFoundCondition;
	Elite::FSMCondition* itemHandlingDoneCondition = new FSMConditions::ItemHandlingDoneCondition;
	Elite::FSMCondition* needHealingCondition = new FSMConditions::NeedHealingCondition;
	Elite::FSMCondition* noNeedHealingCondition = new FSMConditions::NoNeedHealingCondition;
	Elite::FSMCondition* purgeZoneCondition = new FSMConditions::PurgeZoneCondition;
	Elite::FSMCondition* noPurgeZoneCondition = new FSMConditions::NoPurgeZoneCondition;
	Elite::FSMCondition* wasAttackedCondition = new FSMConditions::WasAttackedCondition;
	Elite::FSMCondition* enemyGoneCondition = new FSMConditions::EnemyGoneCondition;

	// make state machine
	m_pStateMachine = new Elite::FiniteStateMachine(exploringState, pBlackBoard);

	// add transitions
	m_pStateMachine->AddTransition(exploringState, houseState, foundHouseCondition);
	m_pStateMachine->AddTransition(exploringState, itemState, itemFoundCondition);
	m_pStateMachine->AddTransition(exploringState, healingState, needHealingCondition);
	m_pStateMachine->AddTransition(exploringState, purgeZoneState, purgeZoneCondition);
	m_pStateMachine->AddTransition(exploringState, fightingState, wasAttackedCondition);

	m_pStateMachine->AddTransition(houseState, exploringState, houseSearchDoneCondition);
	m_pStateMachine->AddTransition(houseState, itemState, itemFoundCondition);
	m_pStateMachine->AddTransition(houseState, healingState, needHealingCondition);
	m_pStateMachine->AddTransition(houseState, purgeZoneState, purgeZoneCondition);
	m_pStateMachine->AddTransition(houseState, fightingState, wasAttackedCondition);

	m_pStateMachine->AddTransition(itemState, exploringState, itemHandlingDoneCondition);
	m_pStateMachine->AddTransition(itemState, healingState, needHealingCondition);
	m_pStateMachine->AddTransition(itemState, purgeZoneState, purgeZoneCondition);
	m_pStateMachine->AddTransition(itemState, fightingState, wasAttackedCondition);

	m_pStateMachine->AddTransition(healingState, exploringState, noNeedHealingCondition);
	m_pStateMachine->AddTransition(healingState, fightingState, wasAttackedCondition);

	m_pStateMachine->AddTransition(fightingState, exploringState, enemyGoneCondition);
	m_pStateMachine->AddTransition(fightingState, purgeZoneState, purgeZoneCondition);

	m_pStateMachine->AddTransition(purgeZoneState, exploringState, noPurgeZoneCondition);
}

//Called only once
void Plugin::DllInit()
{
	// Called when the plugin is loaded

	// DEBUG Steering
	m_pSeek = new Steering::Seek();
	m_pFlee = new Steering::Flee();
	m_pFace = new Steering::Face();
	m_pWander = new Steering::Wander();
	m_pEvade = new Steering::Evade();
	
	m_IdealInventory.push_back(eItemType::PISTOL);
	m_IdealInventory.push_back(eItemType::SHOTGUN);
	m_IdealInventory.push_back(eItemType::MEDKIT);
	m_IdealInventory.push_back(eItemType::FOOD);
	m_IdealInventory.push_back(eItemType::FOOD);
}

//Called only once
void Plugin::DllShutdown()
{
	// Called when the plugin gets unloaded

	// DEBUG Steering
	delete m_pSeek;
	delete m_pFlee;
	delete m_pFace;
	delete m_pWander;
	delete m_pEvade;

	// DecisionMaking
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pConditions)
	{
		SAFE_DELETE(t);
	}
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	srand(unsigned int(time(nullptr)));

	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = false; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 0; //How many enemies? (Default = 20)
	params.GodMode = true; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = true;
	params.SpawnDebugShotgun = true;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = rand();
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	////Demo Event Code
	////In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	{
		m_ShowExploringCheckpoints = !m_ShowExploringCheckpoints;
	}

	const WorldInfo world{ m_pInterface->World_GetInfo() };
	Elite::Vector2 worldCenter{ world.Center };
	Elite::Vector2 worldDimentions{ world.Dimensions / 2 };

	Elite::Vector2 worldBounds[4]{	{worldCenter.x - worldDimentions.x, worldCenter.y + worldDimentions.y},
									{worldCenter.x + worldDimentions.x, worldCenter.y + worldDimentions.y},
									{worldCenter.x + worldDimentions.x, worldCenter.y - worldDimentions.y},
									{worldCenter.x - worldDimentions.x, worldCenter.y - worldDimentions.y}, };
	m_pInterface->Draw_Polygon(worldBounds, 4, { 1, 0, 0 });

	if (m_ShowExploringCheckpoints)
	{
		// checkpoints for exploring
		float diameter{ 25 };
		int dotsPerCircle{ 4 };
		const float nrCircles{ 7 };

		float angle{ static_cast<float>(2 * M_PI / dotsPerCircle) };

		for (int cirlce{ 0 }; cirlce < nrCircles; ++cirlce)
		{
			for (int dot{ 0 }; dot < dotsPerCircle; ++dot)
			{
				Elite::Vector2 circleCenter{ worldCenter.x + diameter * cosf(dot * angle), worldCenter.y + diameter * sinf(dot * angle) };
				m_pInterface->Draw_SolidCircle(circleCenter, 1, { 0,0 }, { 0, 0, 1 });
			}

			if (cirlce < 2)
			{
				diameter *= 2;
				dotsPerCircle *= 2;
				angle /= 2;
			}
			else
			{
				diameter += 50;
				dotsPerCircle += 4;
				angle = static_cast<float>(2 * M_PI / dotsPerCircle);
			}
		}
	}

	if (m_ShowHouseCheckpoints)
	{
		Elite::Blackboard* pBlackBoard{ m_pStateMachine->GetBlackboard() };
		bool currentHouseExists{ true };

		std::vector<AgentMemoryHouse>* pHouses;
		if (pBlackBoard->GetData("MemoryHouses", pHouses) == false || pHouses == nullptr)
		{
			currentHouseExists = false;
		}
		else
		{
			if (pHouses->empty())
				currentHouseExists = false;
		}

		int houseIndex;
		if (pBlackBoard->GetData("HouseIndex", houseIndex) == false)
		{
			currentHouseExists = false;
		}


		if (currentHouseExists)
		{
			const AgentMemoryHouse& searchHouse{ (*pHouses)[houseIndex] };

			Elite::Vector2 houseCenter{ searchHouse.Center };
			Elite::Vector2 searchSize{ searchHouse.Size / 4 };

			// calculate checkpoints
			Elite::Vector2 housePoint1{ houseCenter.x - searchSize.x, houseCenter.y + searchSize.y };
			Elite::Vector2 housePoint2{ houseCenter.x + searchSize.x, houseCenter.y + searchSize.y };
			Elite::Vector2 housePoint3{ houseCenter.x - searchSize.x, houseCenter.y - searchSize.y };
			Elite::Vector2 housePoint4{ houseCenter.x + searchSize.x, houseCenter.y - searchSize.y };

			m_pInterface->Draw_SolidCircle(housePoint1, 1, { 0,0 }, { 0, 1, 1 });
			m_pInterface->Draw_SolidCircle(housePoint2, 1, { 0,0 }, { 0, 1, 1 });
			m_pInterface->Draw_SolidCircle(housePoint3, 1, { 0,0 }, { 0, 1, 1 });
			m_pInterface->Draw_SolidCircle(housePoint4, 1, { 0,0 }, { 0, 1, 1 });

			m_pInterface->Draw_Segment(housePoint1, housePoint2, { 0, 1, 1 });
			m_pInterface->Draw_Segment(housePoint2, housePoint3, { 0, 1, 1 });
			m_pInterface->Draw_Segment(housePoint3, housePoint4, { 0, 1, 1 });
		}
	}

}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();
	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	std::vector<EntityInfo> vItems{};
	std::vector<EntityInfo> vEnemies{};
	std::vector<EntityInfo> vPurgeZones{};
	SortEntities(vEntitiesInFOV, vItems, vEnemies, vPurgeZones);

	auto pBlackBoard{ m_pStateMachine->GetBlackboard() };
	
	pBlackBoard->ChangeData("AgentInfo", agentInfo);
	pBlackBoard->ChangeData("FOVHouseInfo", vHousesInFOV);
	pBlackBoard->ChangeData("FOVEntityInfo", vEntitiesInFOV);

	pBlackBoard->ChangeData("FOVItemInfo", vItems);
	pBlackBoard->ChangeData("FOVEnemyInfo", vEnemies);
	pBlackBoard->ChangeData("FOVPurgeZoneInfo", vPurgeZones);

	m_pStateMachine->Update(dt);

	SteeringPlugin_Output steering;
	steering.AutoOrient = false;
	if (pBlackBoard->GetData("SteeringOutput", steering) == false)
	{
		steering.AngularVelocity = 0;
		steering.AutoOrient = false;
		steering.LinearVelocity = { 0,0 };
		steering.RunMode = false;
	}

	bool shouldRun{ false };
	if (pBlackBoard->GetData("ShouldRun", shouldRun) == true)
	{
		steering.RunMode = shouldRun;
	}

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	//m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void Plugin::SortEntities(const std::vector<EntityInfo>& entities, std::vector<EntityInfo>& items, std::vector<EntityInfo>& enemies, std::vector<EntityInfo>& purgeZones) const
{
	for (const EntityInfo& currEntity : entities)
	{
		switch (currEntity.Type)
		{
		case eEntityType::ITEM:
			items.push_back(currEntity);
			break;
		case eEntityType::ENEMY:
			enemies.push_back(currEntity);
			break;
		case eEntityType::PURGEZONE:
			purgeZones.push_back(currEntity);
			break;
		}
	}
}

Elite::Blackboard* Plugin::CreateBlackBoard()
{
	Elite::Blackboard* pBlackBoard = new Elite::Blackboard();

	// general info
	pBlackBoard->AddData("AgentInfo", AgentInfo{});
	pBlackBoard->AddData("FOVHouseInfo", std::vector<HouseInfo>{});
	pBlackBoard->AddData("FOVEntityInfo", std::vector<EntityInfo>{});

	pBlackBoard->AddData("FOVItemInfo", std::vector<EntityInfo>{});
	pBlackBoard->AddData("FOVEnemyInfo", std::vector<EntityInfo>{});
	pBlackBoard->AddData("FOVPurgeZoneInfo", std::vector<EntityInfo>{});

	pBlackBoard->AddData("SteeringOutput", SteeringPlugin_Output{});
	pBlackBoard->AddData("Interface", m_pInterface);

	// GOAP info
	pBlackBoard->AddData("MoveInfo", Elite::MoveInfo{});
	pBlackBoard->AddData("HouseIndex", int{});	// index in memory house
	pBlackBoard->AddData("DoneDefending", bool{});	// done defending
	pBlackBoard->AddData("ShouldRun", bool{});	// should run

	// memory
	pBlackBoard->AddData("MemoryHouses", m_pMemoryHouses);
	pBlackBoard->AddData("MemoryInventory", m_pMemoryInventory);
	pBlackBoard->AddData("IdealInventory", m_IdealInventory);
	pBlackBoard->AddData("ItemsToGrab", std::vector<EntityInfo>{});
	

	return pBlackBoard;
}
