#include "stdafx.h"
#include "Flock.h"
#include "projects/Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 5 }
	, m_NrOfNeighbors{ 0 }
{
	m_Agents.resize(m_FlockSize);
	m_AgentsOldPos.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize - 1);


	// initialize behaviours
	m_pSeekBehavior = new Seek();
	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pWanderBehavior = new Wander();
	m_pEvadeBehavior = new Evade();

	m_pEvadeBehavior->SetEvadeRadius(30);

	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetSteeringBehavior(m_pWanderBehavior);
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetBodyColor({ 1, 0, 0 });
	m_pAgentToEvade->SetMaxLinearSpeed(65.f);
	m_pAgentToEvade->SetMaxAngularSpeed(25.f);
	m_pAgentToEvade->SetPosition(randomVector2(0, m_WorldSize));

	m_pBlendedSteering = new BlendedSteering({  {m_pSeekBehavior, 0}, 
												{m_pCohesionBehavior, 0}, 
												{m_pSeparationBehavior, 0}, 
												{m_pVelMatchBehavior, 0},
												{m_pWanderBehavior, 0} });

	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });

	// initialize the flock and the memory pool
	for (int i{}; i < m_FlockSize; ++i)
	{
		m_Agents[i] = new SteeringAgent();
		m_Agents[i]->SetPosition(randomVector2(0, m_WorldSize));
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);

		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMaxLinearSpeed(55.f);
		m_Agents[i]->SetMaxAngularSpeed(25.f);
		m_Agents[i]->SetMass(1.f);

		if (i == 0)
		{
			//m_Agents[i]->SetRenderBehavior(true);
		}

		m_AgentsOldPos[i] = m_Agents[i]->GetPosition();
	}

	// initialize cellSpace
	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, 25, 25, m_FlockSize);
	for (int i{}; i < m_FlockSize; ++i)
	{
		m_pCellSpace->AddAgent(m_Agents[i]);
	}

	// message debug colors
	std::cout	<< "Debug direction colors: \n"
				<< "Seek: green \n"
				<< "Cohesion: dark blue \n"
				<< "Separation: purple \n"
				<< "Velocity match: light blue \n"
				<< "Wander: yellow \n"
				<< "Evade: red \n";
}

Flock::~Flock()
{
	// clean up any additional data
	delete m_pAgentToEvade;

	delete m_pSeekBehavior;
	delete m_pCohesionBehavior;
	delete m_pSeparationBehavior;
	delete m_pVelMatchBehavior;
	delete m_pWanderBehavior;
	delete m_pEvadeBehavior;

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();

	SAFE_DELETE(m_pCellSpace);
}

void Flock::Update(float deltaT)
{
	if (m_PartitioningToggledOn)		// cellspace not updated when toggled on, update before calculations
	{
		for (int i{}; i < m_FlockSize; ++i)
		{
			m_pCellSpace->UpdateAgentCell(m_Agents[i], m_AgentsOldPos[i]);
		}
	}

	// update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world

	m_pAgentToEvade->Update(deltaT);
	m_pAgentToEvade->TrimToWorld(m_WorldSize, m_TrimWorld);
	m_pEvadeBehavior->SetTarget(TargetData{	m_pAgentToEvade->GetPosition(), 
											m_pAgentToEvade->GetRotation(), 
											m_pAgentToEvade->GetLinearVelocity(), 
											m_pAgentToEvade->GetAngularVelocity()});

	for (int i{}; i < m_FlockSize; ++i)
	{
		if (m_SpacePartitioning)
		{
			m_pCellSpace->RegisterNeighbors(m_Agents[i], m_NeighborhoodRadius);

			m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
			for (int i{}; i < m_NrOfNeighbors; ++i)
			{
				m_Neighbors[i] = m_pCellSpace->GetNeighbors()[i];
			}
		}
		else
		{
			RegisterNeighbors(m_Agents[i]);
		}

		m_Agents[i]->Update(deltaT);
		m_Agents[i]->TrimToWorld(m_WorldSize, m_TrimWorld);
	}

	if (m_SpacePartitioning)
	{
		for (int i{}; i < m_FlockSize; ++i)
		{
			m_pCellSpace->UpdateAgentCell(m_Agents[i], m_AgentsOldPos[i]);
		}

		for (int i{}; i < m_FlockSize; ++i)
		{
			m_AgentsOldPos[i] = m_Agents[i]->GetPosition();	
		}
	}

}

void Flock::Render(float deltaT)
{
	if(m_RenderCells)
		m_pCellSpace->RenderCells();

	m_pAgentToEvade->Render(deltaT);

	for (SteeringAgent* pAgent : m_Agents)
	{
		//pAgent->Render(deltaT);

		if (pAgent->CanRenderBehavior())
		{
			if (m_SpacePartitioning)
			{
				m_pCellSpace->RenderNeighborhood(pAgent, m_NeighborhoodRadius);		
			}
			else
			{
				RegisterNeighbors(pAgent);

				DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, Color({ 1, 0, 0 }), 0);
				for (int i{}; i < m_NrOfNeighbors; ++i)
				{
					DEBUGRENDERER2D->DrawCircle(m_Neighbors[i]->GetPosition(), 1, Color({ 0, 1, 0 }), 0);
				}
			}
		}
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	bool canRender{ m_Agents[0]->CanRenderBehavior() };
	if(ImGui::Checkbox("Debug Rendering", &canRender))
		m_Agents[0]->SetRenderBehavior(canRender);
	ImGui::Spacing();


	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text("Space Partitioning");
	ImGui::Spacing();

	if (ImGui::Checkbox("Space Part.", &m_SpacePartitioning) && m_SpacePartitioning)	// toggled on
		m_PartitioningToggledOn = true;
	ImGui::Checkbox("Render (if using space part.)", &m_RenderCells);
	if (!m_SpacePartitioning)
		m_RenderCells = false;
	ImGui::Spacing();

	auto blendedBehaviours{ m_pBlendedSteering->GetWeightedBehaviorsRef() };

	// Implement checkboxes for debug rendering and weight sliders here
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("VelocityMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	for (SteeringAgent* pOtherAgent : m_Agents)
	{
		if (pAgent != pOtherAgent)	// skip current agent
		{
			Elite::Vector2 distance{ pAgent->GetPosition() - pOtherAgent->GetPosition() };
			if (distance.MagnitudeSquared() <= m_NeighborhoodRadius * m_NeighborhoodRadius)	// in neighborhood, magnitudeSquared == more optimal
			{
				m_Neighbors[m_NrOfNeighbors] = pOtherAgent;
				++m_NrOfNeighbors;
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Vector2 averagePos{};
	
	for (int i{}; i < m_NrOfNeighbors; ++i)
	{
		averagePos.x += m_Neighbors[i]->GetPosition().x;
		averagePos.y += m_Neighbors[i]->GetPosition().y;
	}

	averagePos.x /= m_NrOfNeighbors;
	averagePos.y /= m_NrOfNeighbors;

	return averagePos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Vector2 averageVel{};

	for (int i{}; i < m_NrOfNeighbors; ++i)
	{
		averageVel.x += m_Neighbors[i]->GetLinearVelocity().x;
		averageVel.y += m_Neighbors[i]->GetLinearVelocity().y;
	}

	averageVel.x /= m_NrOfNeighbors;
	averageVel.y /= m_NrOfNeighbors;

	return averageVel;
}

void Flock::SetTarget_Seek(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
