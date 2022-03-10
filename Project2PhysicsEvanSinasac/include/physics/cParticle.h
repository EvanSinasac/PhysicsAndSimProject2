#pragma once

#include <../../Project2PhysicsEvanSinasac/GLMCommon.h>

namespace nPhysics
{
	class cParticle
	{
	private:
		// Linear position of the particle in world space
		glm::vec3 mPosition;
		// Linear velocity of the particle in world space
		glm::vec3 mVelocity;
		// Linear acceleration of the particle in world space
		glm::vec3 mAcceleration;
		// Inverse mass
		float mInverseMass;
		// Externally applied forces accumulated during a time step
		glm::vec3 mAppliedForce;
		// Damping is applied to linear motion
		// Required to remove energy added through numerical instability of integration
		// Also generally used as an overall "friction" for the particle
		float mDamping;

		//project 2 stuff
		float timeAlive;
		float timeToDie;
		bool isAlive;
		//I can not for the life of me get a firework class to work pulling from particles, either I'm get this weird linker error about the constructor
		//or it just does not inherit from cParticle and I can't access the Particle specific functions
		int fireworkType;
		int fireworkStage;
	public:
		cParticle(float mass, const glm::vec3& position);
		virtual ~cParticle();

		cParticle() = delete;
		cParticle(cParticle& other) = delete;
		cParticle& operator=(cParticle& other) = delete;

		void SetDamping(float damping);
		float GetDamping() const;

		float GetMass() const;
		float GetInverseMass() const;

		bool IsStaticObject() const;

		glm::vec3 GetPosition() const;
		void GetPosition(glm::vec3& position);
		void SetPosition(const glm::vec3& position);

		glm::vec3 GetVelocity() const;
		void GetVelocity(glm::vec3& velocity);
		void SetVelocity(const glm::vec3& velocity);

		glm::vec3 GetAcceleration() const;
		void GetAcceleration(glm::vec3& acceleration);
		void SetAcceleration(const glm::vec3& acceleration);

		glm::vec3 GetAppliedForce() const;

		void ApplyForce(const glm::vec3& force);
		void ClearAppliedForces();

		virtual void Integrate(float deltaTime);

		//Project 2 stuff
		void AddToTimeAlive(float deltaTime);
		float GetTimeAlive();

		void SetTimeToDie(float nTimeToDie);
		float GetTimeToDie();

		void SetIsAlive(bool nIsAlive);
		bool GetIsAlive();

		void SetFireworkType(int nType);
		int GetFireworkType();

		void SetFireworkStage(int nStage);
		int GetFireworkStage();

	};
}