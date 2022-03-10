#include <physics/random_Helpers.h>

namespace nPhysics
{
	float getRandom()
	{
		return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}

	float getRandom(float high)
	{
		return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / high);
	}

	float getRandom(float low, float high)
	{
		return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (high - low));
	}

	glm::vec3 getRandomVec3(float length)
	{
		float alpha = getRandom(glm::pi<float>());							//glm::pi<float>();  locks one angle (creates a circle instead of a sphere)
		float delta = getRandom(-glm::pi<float>(), glm::pi<float>());

		float sinAlpha = glm::sin(alpha);
		float cosAlpha = glm::cos(alpha);
		float sinDelta = glm::sin(delta);
		float cosDelta = glm::cos(delta);

		return glm::vec3(length * sinAlpha * cosDelta, length * sinAlpha * sinDelta, length * cosAlpha);	//random vector in any direction
		//google: random vector in any direction
		//google: spherical coordinates to cartesian coordinates
	}

	glm::vec3 getRandomVec3Upwards(float length)
	{
		glm::vec3 randUp(getRandom(-10.0f, 10.0f), getRandom(10.0f, 50.0f), getRandom(-10.0f, 10.0f));
		randUp = glm::normalize(randUp);
		randUp *= length;
		return randUp;			//random vector mostly up
	}

	glm::vec3 getRandomVec3Circle(float length)
	{
		float alpha = getRandom(glm::pi<float>());							
		float delta = getRandom(glm::pi<float>(), glm::two_pi<float>());

		float sinAlpha = glm::sin(alpha);
		float cosAlpha = glm::cos(alpha);
		float sinDelta = glm::sin(delta);
		float cosDelta = glm::cos(delta);

		glm::vec3 circle(sinAlpha * cosDelta, 0.5f, cosAlpha);
		circle = glm::normalize(circle);
		circle *= length;

		return circle;				//random vector in top half of semisphere
	}

}