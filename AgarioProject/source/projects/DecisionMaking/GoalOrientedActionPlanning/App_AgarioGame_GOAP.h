#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "Actions.h"

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;
class NavigationColliderElement;

class App_AgarioGame_GOAP final : public IApp
{
public:
	App_AgarioGame_GOAP();
	~App_AgarioGame_GOAP();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;
private:
	float m_TrimWorldSize = 150.f;
	const int m_AmountOfAgents{ 20 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pSmartAgent = nullptr;
	const float m_SearchRadius{ 20.f };

	const int m_AmountOfFood{ 40 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;
	bool m_GameWon = false;

	//--Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};
private:	
	template<class T_AgarioType>
	void UpdateAgarioEntities(std::vector<T_AgarioType*>& entities, float deltaTime);

	Elite::Blackboard* CreateBlackboard(AgarioAgent* a);
	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_AgarioGame_GOAP(const App_AgarioGame_GOAP&) {};
	App_AgarioGame_GOAP& operator=(const App_AgarioGame_GOAP&) {};
};

template<class T_AgarioType>
inline void App_AgarioGame_GOAP::UpdateAgarioEntities(std::vector<T_AgarioType*>& entities, float deltaTime)
{
	for (auto& e : entities)
	{
		e->Update(deltaTime);

		if (e->CanBeDestroyed())
			SAFE_DELETE(e);
	}

	auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
		[](T_AgarioType* e) {return e == nullptr; });
	if (toRemoveEntityIt != entities.end())
	{
		entities.erase(toRemoveEntityIt, entities.end());
	}
}
#endif