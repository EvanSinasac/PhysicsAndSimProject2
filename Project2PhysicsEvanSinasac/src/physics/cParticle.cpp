#include <physics/cParticle.h>
#include <../../Project2PhysicsEvanSinasac/GLMCommon.h>
#include <iostream>

namespace nPhysics
{
	cParticle::cParticle(float mass, const glm::vec3& position)
		: mPosition(position)
		, mVelocity(0.f)
		, mAcceleration(0.f)
		, mDamping(0.995f)
		, mAppliedForce(0.f)
		, timeAlive (0.0f)
	{
		if (mass <= 0.f)
		{
			mInverseMass = 0.f;
		}
		else
		{
			mInverseMass = 1.f / mass;
		}
	}
	cParticle::~cParticle()
	{

	}

	void cParticle::SetDamping(float damping)
	{
		mDamping = glm::clamp(damping, 0.f, 1.f);
	}
	float cParticle::GetDamping() const
	{
		return mDamping;
	}

	float cParticle::GetMass() const
	{
		if (mInverseMass == 0.f)
		{
			return mInverseMass;
		}
		return 1.0f / mInverseMass;
	}

	float cParticle::GetInverseMass() const
	{
		return mInverseMass;
	}

	bool cParticle::IsStaticObject() const
	{
		return mInverseMass == 0.f;
	}

	glm::vec3 cParticle::GetPosition() const
	{
		return mPosition;
	}
	void cParticle::GetPosition(glm::vec3& position)
	{
		position = mPosition;
	}
	void cParticle::SetPosition(const glm::vec3& position)
	{
		mPosition = position;
	}

	glm::vec3 cParticle::GetVelocity() const
	{
		return mVelocity;
	}
	void cParticle::GetVelocity(glm::vec3& velocity)
	{
		velocity = mVelocity;
	}
	void cParticle::SetVelocity(const glm::vec3& velocity)
	{
		mVelocity = velocity;
	}

	glm::vec3 cParticle::GetAcceleration() const
	{
		return mAcceleration;
	}
	void cParticle::GetAcceleration(glm::vec3& acceleration)
	{
		acceleration = mAcceleration;
	}
	void cParticle::SetAcceleration(const glm::vec3& acceleration)
	{
		mAcceleration = acceleration;
	}

	glm::vec3 cParticle::GetAppliedForce() const
	{
		return mAppliedForce;
	}

	void cParticle::ApplyForce(const glm::vec3& force)
	{
		mAppliedForce += force;
	}
	void cParticle::ClearAppliedForces()
	{
		glm::set(mAppliedForce, 0.f, 0.f, 0.f);
	}

	void cParticle::Integrate(float deltaTime)
	{
		if (mInverseMass == 0.f)
		{
			return; // static things don't move!
		}

		if (fireworkType == 3 && fireworkStage == 1)
		{
			this->mAcceleration = glm::cross(this->mVelocity, glm::vec3(0, -1.0f, 0));
			this->mAcceleration = glm::normalize(this->mAcceleration);
			this->mAcceleration *= glm::length2(this->mVelocity) / 5.0f;
			this->mAcceleration.y = -3.2f;
		}


		mPosition += mVelocity * deltaTime;
		// F*(1/m) = a
		mVelocity += (mAcceleration + mAppliedForce * mInverseMass) * deltaTime;

		// apply damping
		mVelocity *= glm::pow(mDamping, deltaTime);

		AddToTimeAlive(deltaTime);
		if (timeAlive >= timeToDie)
		{
			isAlive = false;
		}

		// clear applied forces
		ClearAppliedForces();
	}

	//Project 2 stuff
	void cParticle::AddToTimeAlive(float deltaTime)
	{
		timeAlive += deltaTime;
	}
	float cParticle::GetTimeAlive()
	{
		return timeAlive;
	}

	void cParticle::SetTimeToDie(float nTimeToDie)
	{
		timeToDie = nTimeToDie;
	}
	float cParticle::GetTimeToDie()
	{
		return timeToDie;
	}

	void cParticle::SetIsAlive(bool nIsAlive)
	{
		isAlive = nIsAlive;
	}
	bool cParticle::GetIsAlive()
	{
		return isAlive;
	}

	void cParticle::SetFireworkType(int nType)
	{
		fireworkType = nType;
	}
	int cParticle::GetFireworkType()
	{
		return fireworkType;
	}

	void cParticle::SetFireworkStage(int nStage)
	{
		fireworkStage = nStage;
	}
	int cParticle::GetFireworkStage()
	{
		return fireworkStage;
	}
}