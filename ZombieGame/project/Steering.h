
//#include "stdafx.h"
#include "Exam_HelperStructs.h"

namespace Steering
{
	class ISteering
	{
	public:
		ISteering() = default;
		virtual ~ISteering() = default;

		virtual SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) = 0;

		void SetTarget(const Elite::Vector2& target) { m_TargetPosition = target; }
		void SetTargetVelocity(const Elite::Vector2& targetVelocity) { m_TargetVelocity = targetVelocity; }

	protected:
		Elite::Vector2 m_TargetPosition{};
		Elite::Vector2 m_TargetVelocity{};
	};

	///////////////////////////////////////
	//SEEK
	//****
	class Seek : public ISteering
	{
	public:
		Seek() = default;
		virtual ~Seek() = default;

		//Seek Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) override;
	};

	///////////////////////////////////////
	//SEEK & FACE
	//****
	class SeekFace : public ISteering
	{
	public:
		SeekFace() = default;
		virtual ~SeekFace() = default;

		//Seek Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) override;

	private:
		float m_Radius = 0.5f;
	};

	///////////////////////////////////////
	//FLEE
	//****
	class Flee : public ISteering
	{
	public:
		Flee() = default;
		virtual ~Flee() = default;

		//Flee Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) override;
	};

	///////////////////////////////////////
	//FLEE & FACE
	//****
	class FleeFace : public ISteering
	{
	public:
		FleeFace() = default;
		virtual ~FleeFace() = default;

		//Seek Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) override;
	};

	///////////////////////////////////////
	//FACE
	//****
	class Face : public ISteering
	{
	public:
		Face() = default;
		virtual ~Face() = default;

		//Face Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) override;
	};

	///////////////////////////////////////
	//WANDER
	//****
	class Wander : public ISteering
	{
	public:
		Wander() = default;
		virtual ~Wander() = default;

		//Wander Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) override;

		void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
		void SetWanderRadius(float radius) { m_Radius = radius; }
		void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }

	protected:
		float m_OffsetDistance = 6.f; // Offset (Agent Direction)
		float m_Radius = 4.f; // WanderRadius
		float m_MaxAngleChange = Elite::ToRadians(45); // Max Wander Angle change per frame
		float m_WanderAngle = 0.f;
	};

	///////////////////////////////////////
	//EVADE
	//****
	class Evade : public ISteering
	{
	public:
		Evade() = default;
		virtual ~Evade() = default;

		//Evade Behaviour
		SteeringPlugin_Output CalculateSteering(float deltaT, const AgentInfo& agent) override;
	};
}