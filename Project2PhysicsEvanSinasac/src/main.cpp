//Evan Sinasac - 1081418
//INFO6019 Physics and Simulation (Project 2)
//main.cpp description:
//			        The purpose of project 2 is to familiarize ourselves with physics simulations rendered with 3D graphics,
//					by making some fireworks!  Whenever '1', '2', or '3' is pressed, a stage 1 firework is launched with an initial position
//					randomized within a -5,5 square on the ground.  Each firework object needs to have a distinct 'look' to stage 1 motion
//					(play with initial velocities, damping and applied forces), and then a distinct boom (the number of stage 2 particles, the
//					explosion pattern, maybe even colours).  Motion needs to be distinctive enough that each stage 1 firework could be the same colour
//					and an observer can tell which firework is which by its motion and explosion.  Should be able to trigger as many fireworks
//					as possible by pressing 1, 2, and 3 as much as you want.  Harder marks: at least one firework has a third stage

#include "../GLCommon.h"
#include "../GLMCommon.h"
//standard includes
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
//graphics includes
#include "../shaderManager/cVAOManager.h"
#include "../shaderManager/cMesh.h"
#include "../shaderManager/cShaderManager.h"
//physics includes
#include <physics/cParticleWorld.h>
#include <physics/random_Helpers.h>


//Definitions
#define SCREEN_WIDTH 1000.0f
#define SCREEN_HEIGHT 1200.0f


//Globals
glm::vec3 cameraEye = glm::vec3(40.0f, 10.0f, -40.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);		//default upVector for camera

float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;
float yaw = 90.0f;
float pitch = 0.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

cVAOManager		gVAOManager;
cShaderManager	gShaderManager;
//vector of meshes in the scene to draw
std::vector<cMesh> g_vecMeshesToDraw;
//vector of string of where the models are located
std::vector<std::string> modelLocations;
//Particle world that updates particle positions, velocity, acceleration, etc.
nPhysics::cParticleWorld* world;
//vector of Particles being used as fireworks
std::vector<nPhysics::cParticle*> fireworks;

//callbacks
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %s\n", description);
}

static void key_callback   (GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouse_callback (GLFWwindow* window, double xpos, double ypos);

//Used to add models to VAO Manager
bool loadModelsFromFile(GLuint& progam);


//Identifiers
void AddMeshes(glm::vec3 position, float scale, glm::vec3 colour);
void AddParticles(int fireworkType, int fireworkStage, glm::vec3 position);
void LaunchType1Stage2(glm::vec3 position);
void LaunchType2Stage2(glm::vec3 position);
void LaunchType3Stage2(glm::vec3 position);
void LaunchType3Stage3(glm::vec3 position);

int main(int argc, char** argv)
{
	GLFWwindow* window;

	GLuint program = 0;		//0 means "no shader program"

	GLint mvp_location = -1;
	GLint vpos_location = -1;
	GLint vcol_location = -1;

	float fpsFrameCount = 0.0f;
	float fpsTimeElapsed = 0.0f;

	world = new nPhysics::cParticleWorld();

	float timeElapsed = 0.0f;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fireworks Show!", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	//OpenGL error checks omitted for brevity
	std::stringstream ss;

	cShaderManager::cShader vertShader;
	ss << SOLUTION_DIR << "common\\assets\\shaders\\vertShader_01.glsl";
	vertShader.fileName = ss.str().c_str();
	ss.str("");

	cShaderManager::cShader fragShader;
	ss << SOLUTION_DIR << "common\\assets\\shaders\\fragShader_01.glsl";
	fragShader.fileName = ss.str().c_str();
	ss.str("");

	if (gShaderManager.createProgramFromFile("Shader#1", vertShader, fragShader))
	{
		std::cout << "Shader compuled OK" << std::endl;
		program = gShaderManager.getIDFromFriendlyName("Shader#1");
	}
	else
	{
		std::cout << "Error making shader program: " << std::endl;
		std::cout << gShaderManager.getLastError() << std::endl;
	}

	//Getting the locations from the shaders
	GLint matModel_Location = glGetUniformLocation(program, "matModel");
	GLint matView_Location = glGetUniformLocation(program, "matView");
	GLint matProjection_Location = glGetUniformLocation(program, "matProjection");

	GLint matModelInverseTranspose_Location = glGetUniformLocation(program, "matModelInversetranspose");

	//Will remove colour from models, will be assigned by World file
	GLint bUseVertexColour_Location = glGetUniformLocation(program, "bUseVertexColour");
	GLint vertexColourOverride_Location = glGetUniformLocation(program, "vertexColourOverride");

	if (loadModelsFromFile(program))
	{
		std::cout << "loadModelsFromFile finished ok" << std::endl;
	}
	else
	{
		std::cout << "loadModelsFromFile did not finish ok, aborting" << std::endl;
		return -1;
	}
	
	//Meshes added as a kind of reference to see the fireworks, if you want the bunnies to be a bit more obvious/bigger change their scale here

	//Make meshes
	cMesh platform;
	platform.meshName = modelLocations[1];
	platform.positionXYZ = glm::vec3(5.0f, 0.025f, -5.0f);
	platform.orientationXYZ = glm::vec3(3.0f * glm::pi<float>() / 2.0f, 0.0f, 0.0f);
	platform.scale = 1.675f;
	platform.bOverrideColour = true;
	platform.vertexColourOverride = glm::vec3(96.0f/255.0f, 96.0f/255.0f, 96.0f/255.0f);

	cMesh bunny1;
	bunny1.meshName = modelLocations[0];
	bunny1.positionXYZ = glm::vec3(5.0f, 0.0f, 5.0f);
	bunny1.bOverrideColour = true;
	bunny1.vertexColourOverride = glm::vec3(1.0f, 0.0f, 0.0f);
	bunny1.bIsWireframe = true;
	//bunny1.scale = 5;

	cMesh bunny2;
	bunny2.meshName = modelLocations[0];
	bunny2.positionXYZ = glm::vec3(5.0f, 0.0f, -5.0f);
	bunny2.bOverrideColour = true;
	bunny2.vertexColourOverride = glm::vec3(1.0f, 0.0f, 0.0f);
	bunny2.bIsWireframe = true;
	//bunny2.scale = 5;

	cMesh bunny3;
	bunny3.meshName = modelLocations[0];
	bunny3.positionXYZ = glm::vec3(-5.0f, 0.0f, -5.0f);
	bunny3.bOverrideColour = true;
	bunny3.vertexColourOverride = glm::vec3(1.0f, 0.0f, 0.0f);
	bunny3.bIsWireframe = true;
	//bunny3.scale = 5;

	cMesh bunny4;
	bunny4.meshName = modelLocations[0];
	bunny4.positionXYZ = glm::vec3(-5.0f, 0.0f, 5.0f);
	bunny4.bOverrideColour = true;
	bunny4.vertexColourOverride = glm::vec3(1.0f, 0.0f, 0.0f);
	bunny4.bIsWireframe = true;
	//bunny4.scale = 5;

	cMesh bunny5;
	bunny5.meshName = modelLocations[0];
	bunny5.positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	bunny5.bOverrideColour = true;
	bunny5.vertexColourOverride = glm::vec3(1.0f, 0.0f, 0.0f);
	bunny5.bIsWireframe = true;
	//bunny5.scale = 5;

	g_vecMeshesToDraw.push_back(platform);		//[0]
	g_vecMeshesToDraw.push_back(bunny1);		//[1]
	g_vecMeshesToDraw.push_back(bunny2);		//[2]
	g_vecMeshesToDraw.push_back(bunny3);		//[3]
	g_vecMeshesToDraw.push_back(bunny4);		//[4]
	g_vecMeshesToDraw.push_back(bunny5);		//[5]

	//Face the camera towards the origin
	if (cameraEye.x > 0 && cameraEye.z > 0)
	{
		yaw = 180.f + (atan(cameraEye.z / cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (cameraEye.x > 0 && cameraEye.z < 0)
	{
		yaw = 90.f - (atan(cameraEye.z / cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (cameraEye.x < 0 && cameraEye.z > 0)
	{
		yaw = (atan(cameraEye.z / cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (cameraEye.x < 0 && cameraEye.z < 0)
	{
		yaw = (atan(cameraEye.z / cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (cameraEye.x == 0.f)
	{
		if (cameraEye.z >= 0.f)
		{
			yaw = 270.f;
		}
		else
		{
			yaw = 90.f;
		}
	}
	else if (cameraEye.z == 0.f)
	{
		if (cameraEye.x <= 0)
		{
			yaw = 0.f;
		}
		else
		{
			yaw = 180.f;
		}
	}
	cameraTarget = glm::vec3(-1.f * cameraEye.x, 0, -1.f * cameraEye.z);
	glm::normalize(cameraTarget);

	//Main loop
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		glm::mat4 matModel;
		glm::mat4 p;
		glm::mat4 v;
		glm::mat4 mvp;

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		//Turn on the depth buffer
		glEnable(GL_DEPTH);			//Turns on the depth buffer
		glEnable(GL_DEPTH_TEST);	//Check if the pixel is already closer

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//FPS Title stuff		//Copied from SimpleGameApp
		{
			fpsTimeElapsed += deltaTime;
			fpsFrameCount += 1.0f;
			if (fpsTimeElapsed >= 0.03f)
			{
				std::string fps = std::to_string(fpsFrameCount / fpsTimeElapsed);
				std::string ms = std::to_string(1000.0f * fpsTimeElapsed / fpsFrameCount);
				std::string newTitle = "Fireworks Show!   FPS: " + fps + "   MS: " + ms;
				glfwSetWindowTitle(window, newTitle.c_str());
				//reset times and counts
				fpsTimeElapsed = 0.0f;
				fpsFrameCount = 0.0f;
			}
		}
		
		//Keep track of meshes equivalent to existing particles
		//When a particle is dead, remove it from the world, remove it from the vector of fireworks, and remove its mesh from g_vecMeshesToDraw
		
		//Integrate all the particles in the world
		world->TimeStep(deltaTime);

		//Update mesh positions after updating particle position
		for (int index = 0; index < fireworks.size(); index++)
		{
			g_vecMeshesToDraw[index + 6].positionXYZ = fireworks[index]->GetPosition();
			//Check if the firework is still alive, if not remove it and it's related stuff from the world
			if (!fireworks[index]->GetIsAlive())
			{
				
				int type = fireworks[index]->GetFireworkType();
				int stage = fireworks[index]->GetFireworkStage();
				glm::vec3 position = fireworks[index]->GetPosition();
				
				world->RemoveParticle(fireworks[index]);
				fireworks.erase(fireworks.begin()+index);
				g_vecMeshesToDraw.erase(g_vecMeshesToDraw.begin() + index + 6);
				index--;

				//Activate Stage 2 or 3 if the particle has it
				//Found that if I did this before removing the particle/mesh, it kept the mesh in
				if (stage == 1)
				{
					switch (type)
					{
					case 1:
						LaunchType1Stage2(position);
						break;
					case 2:
						LaunchType2Stage2(position);
						break;
					case 3:
						LaunchType3Stage2(position);
						break;
					default:
						break;
					}
				}
				else if (stage == 2)
				{
					if (type == 3)
					{
						LaunchType3Stage3(position);
					}
				}
			} //end of if firework is dead
		} //end of for loop


		//*********************************************************************
		//Screen is cleared and we're ready to draw
		//*********************************************************************
		for (unsigned int index = 0; index != g_vecMeshesToDraw.size(); index++)
		{
			cMesh curMesh = g_vecMeshesToDraw[index];

			//         mat4x4_identity(m);
			matModel = glm::mat4(1.0f);     //"Identity" ("do nothing", like x1)
			//*********************************************************************
			//Translate or "move the object
			//*********************************************************************
			glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
				curMesh.positionXYZ);

			//matModel = matModel * matTranslate;
			//*********************************************************************
			//
			//*********************************************************************
			//Rotation around the Z-axis
			//mat4x4_rotate_Z(m, m, (float) glfwGetTime());
			glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
				curMesh.orientationXYZ.z,   //(float)glfwGetTime()
				glm::vec3(0.0f, 0.0, 1.0f));

			//matModel = matModel * rotateZ;
			//*********************************************************************
			//
			//*********************************************************************
			//Rotation around the Y-axis
			glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
				curMesh.orientationXYZ.y,   //(float)glfwGetTime()
				glm::vec3(0.0f, 1.0, 0.0f));

			//matModel = matModel * rotateY;
			//*********************************************************************
			//
			//*********************************************************************
			//Rotation around the X-axis
			glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
				curMesh.orientationXYZ.x,   //(float)glfwGetTime()
				glm::vec3(1.0f, 0.0, 0.0f));

			//matModel = matModel * rotateX;
			//*********************************************************************
			//Scale
			//*********************************************************************
			glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
				glm::vec3(curMesh.scale,  // Scale in X
					curMesh.scale,  // Scale in Y
					curMesh.scale));// Scale in Z

		//matModel = matModel * matScale;
		//*********************************************************************
		// 
		//*********************************************************************
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
			matModel = matModel * matTranslate;
			matModel = matModel * rotateZ;
			matModel = matModel * rotateY;
			matModel = matModel * rotateX;
			matModel = matModel * matScale;


			p = glm::perspective(0.6f,
				ratio,
				0.1f,
				1000.0f);

			v = glm::mat4(1.0f);

			/*glm::vec3 cameraEye = glm::vec3(0.0, 0.0, -4.0f);*/
			/*glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);*/
			/*glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);*/

			v = glm::lookAt(cameraEye,          // "eye"
				cameraEye + cameraTarget,       // "at" //used to be    cameraTarget
				upVector);

			//mat4x4_mul(mvp, p, m);
			mvp = p * v * matModel;
			glUseProgram(program);


			//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

			//Don't need this anymore since it's being done inside the shader
			//mvp = p * v * matModel;
			glUniformMatrix4fv(matModel_Location, 1, GL_FALSE, glm::value_ptr(matModel));
			glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(v));
			glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(p));

			//Inverse transpose of the mdoel matrix
			//(Used to calculate the normal location in vertex space
			glm::mat4 matInvTransposeModel = glm::inverse(glm::transpose(matModel));
			glUniformMatrix4fv(matModelInverseTranspose_Location, 1, GL_FALSE, glm::value_ptr(matInvTransposeModel));

			//Colour Override HACK (will be removed/changed later)
			if (curMesh.bOverrideColour)
			{
				glUniform1f(bUseVertexColour_Location, (float)GL_TRUE);
				glUniform3f(vertexColourOverride_Location,
					curMesh.vertexColourOverride.r,
					curMesh.vertexColourOverride.g,
					curMesh.vertexColourOverride.b);
			}
			else
			{
				glUniform1f(bUseVertexColour_Location, (float)GL_FALSE);
			}

			//Wireframe
			if (curMesh.bIsWireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				//GL_POINT, GL_LINE, and GL_FILL


			sModelDrawInfo modelInfo;
			if (gVAOManager.FindDrawInfoByModelName(g_vecMeshesToDraw[index].meshName, modelInfo))
			{
				glBindVertexArray(modelInfo.VAO_ID);
				glDrawElements(GL_TRIANGLES,
					modelInfo.numberOfIndices,
					GL_UNSIGNED_INT,
					(void*)0);
				glBindVertexArray(0);
			}

		} //end of for
		//Scene is drawn

		//"Present" what we've drawn
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}



bool loadModelsFromFile(GLuint& program)
{
	std::stringstream ss;
	std::stringstream sFile;
	sModelDrawInfo curModel;

	ss << SOLUTION_DIR << "common\\assets\\models\\modelsToLoad.txt";

	std::ifstream theFile(ss.str());
	if (!theFile.is_open())
	{
		fprintf(stderr, "Could not open modelsToLoad.txt");
		return false;
	}
	std::string nextToken;
	ss.str("");

	while (theFile >> nextToken)
	{
		if (nextToken == "end")
		{
			break;
		}

		if (nextToken.find("ply") != std::string::npos)
		{
			sFile << nextToken.c_str();
			ss << SOLUTION_DIR << "common\\assets\\models\\" << sFile.str().c_str();
			modelLocations.push_back(ss.str().c_str());
			if (gVAOManager.LoadModelIntoVAO(ss.str().c_str(), curModel, program))
			{
				std::cout << "Loaded the " << sFile.str().c_str() << " model OK" << std::endl;
			}
			else
			{
				fprintf(stderr, "Error loading model");
			}
			ss.str("");
			sFile.str("");
		}
		else
		{
			sFile << nextToken.c_str() << " ";
		} //end else
	} //end while
	theFile.close();
	return true;
} //end of loadModelsFromFile


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	float cameraSpeed = 50.0f * deltaTime;

	//Basic camera controls
	if (key == GLFW_KEY_A) {
		//Go left
		cameraEye -= glm::normalize(glm::cross(cameraTarget, upVector)) * cameraSpeed;
	}
	if (key == GLFW_KEY_D) {
		//Go right
		cameraEye += glm::normalize(glm::cross(cameraTarget, upVector)) * cameraSpeed;
	}
	if (key == GLFW_KEY_S) {
		//Go back
		cameraEye -= glm::vec3(cameraTarget.x * cameraSpeed, 0.0f, cameraTarget.z * cameraSpeed);
	}
	if (key == GLFW_KEY_W) {
		//Go forward
		cameraEye += glm::vec3(cameraTarget.x * cameraSpeed, 0.0f, cameraTarget.z * cameraSpeed);
	}
	if (key == GLFW_KEY_Q) {
		//Go down
		cameraEye -= upVector * cameraSpeed;
	}
	if (key == GLFW_KEY_E) {
		//Go up
		cameraEye += upVector * cameraSpeed;
	}

	//Physics project 2 fireworks buttons
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		//Launch a basic firework that goes straight up and then explodes in a sphere
		AddParticles(1, 1, glm::vec3(0));
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		//Launch a firework with some wind to it
		AddParticles(2, 1, glm::vec3(0));
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		//Launch a "fountain" firwork with 3 stages
		AddParticles(3, 1, glm::vec3(0));
	}

} //end of key_callback

//By passing in the type of firework and what stage it is, as well as the initial position (for stage 2 and 3) we can make new particles
void AddParticles(int fireworkType, int fireworkStage, glm::vec3 position)
{
	nPhysics::cParticle* newParticleToAdd;				//pointer
	float scale = 1.0f;									//default scale
	glm::vec3 colour = glm::vec3(1.0f, 1.0f, 1.0f);		//default color
	
	if (fireworkType == 1)
	{
		if (fireworkStage == 1)
		{
			//Firework Type 1 Stage 1 (Activated by the user pressing the 1 key)
			newParticleToAdd = new nPhysics::cParticle(1.0f, glm::vec3(nPhysics::getRandom(-5.0f, 5.0f), 0.1f, nPhysics::getRandom(-5.0f, 5.0f)));
			newParticleToAdd->SetVelocity(glm::vec3(0.0f, 10.0f, 0.0f));									//Launches a firework from the base with an initial velocity of straight up with magnitude 10
			newParticleToAdd->SetAcceleration(glm::vec3(0.0f, -2.5f, 0.0f));								//Sets the acceleration of the particle to -2.5f in the y-axis
			newParticleToAdd->SetFireworkType(1);															//Set the type to 1
			newParticleToAdd->SetFireworkStage(1);															//Set the stage to 1
			newParticleToAdd->SetIsAlive(true);																//Set isAlive to true
			newParticleToAdd->SetTimeToDie(5.0f);															//Set how long the firework will be alive for
			scale = 10.0f;																					//Set the scale of the bunny model
			colour = glm::vec3(1.0f, 0.0f, 0.0f);															//Set the colour of the bunny model
		}
		else if (fireworkStage == 2)
		{
			newParticleToAdd = new nPhysics::cParticle(1.0f, position);
			newParticleToAdd->SetVelocity(nPhysics::getRandomVec3(2.0f));									//Launches the particle from the previous stage 1 particle's position with a random vec 3 (sphere) of magnitude 2
			newParticleToAdd->SetAcceleration(glm::vec3(0.0f, 0.0f, 0.0f));									//Sets the acceleration of the particle to 0.0f
			newParticleToAdd->SetFireworkType(1);															//Set the type to 1
			newParticleToAdd->SetFireworkStage(2);															//Set the stage to 2
			newParticleToAdd->SetIsAlive(true);																//Set isAlive to true
			newParticleToAdd->SetTimeToDie(3.0f);															//Set how long the firework will be alive for
			scale = 5.0f;																					//Set the scale of the bunny model
			colour = glm::vec3(0.5f, nPhysics::getRandom(0, 255) / 255, nPhysics::getRandom(0, 255) / 255);	//Set the colour of the bunny model
		}
		else
		{
			//Error check
			std::cout << "Invalid Stage for a Type 1 Firework!" << std::endl;
			return;
		}
	}
	else if (fireworkType == 2)
	{
		if (fireworkStage == 1)
		{
			//Firework Type 2 Stage 1 (Activated by the user pressing the 2 key)
			newParticleToAdd = new nPhysics::cParticle(1.0f, glm::vec3(nPhysics::getRandom(-5.0f, 5.0f), 0.1f, nPhysics::getRandom(-5.0f, 5.0f)));
			newParticleToAdd->SetVelocity(nPhysics::getRandomVec3Upwards(15.0f));							//Launches the particle from the base with a random mostly upwards vector with a magnitude of 15
			newParticleToAdd->SetAcceleration(glm::vec3(-3.2f, -3.2f, 3.2f));								//Set the acceleration of the particle (right and away from camera)
			newParticleToAdd->SetDamping(0.55f);															//Set the Damping to 0.55 (kinda just playing around with it)
			newParticleToAdd->SetFireworkType(2);															//Set the type to 2
			newParticleToAdd->SetFireworkStage(1);															//Set the stage to 1
			newParticleToAdd->SetIsAlive(true);																//Set isAlive to true
			newParticleToAdd->SetTimeToDie(4.0f);															//Set how long the particle is alive for
			scale = 8.0f;																					//Set the scale of the bunny model
			colour = glm::vec3(0.0f, 1.0f, 0.0f);															//Set the colour of the bunny model
		}
		else if (fireworkStage == 2)
		{
			newParticleToAdd = new nPhysics::cParticle(1.0f, position);
			newParticleToAdd->SetVelocity(nPhysics::getRandomVec3Upwards(10.0f));							//Launches the particle from the position of the previous particle with a random mostly upwards vector with a magnitude of 10
			newParticleToAdd->SetAcceleration(glm::vec3(6.4f, -6.4f, -6.4f));								//Set the acceleration of the particle (left and towards the camera)
			newParticleToAdd->SetDamping(0.55f);															//Set the Damping to 0.55 (kinda just playing around with it)
			newParticleToAdd->SetFireworkType(2);															//Set the type to 2
			newParticleToAdd->SetFireworkStage(2);															//Set the type of 2
			newParticleToAdd->SetIsAlive(true);																//Set isAlive to true
			newParticleToAdd->SetTimeToDie(2.0f);															//Set how long the particle is alive for
			scale = 4.0f;																					//Set the scale of the bunny model
			colour = glm::vec3(nPhysics::getRandom(0.0f, 255.0f) / 255.0f, 0.5f, nPhysics::getRandom(0.0f, 255.0f) / 255.0f);	//Set the colour of the bunny model
		}
		else
		{
			//Error check
			std::cout << "Invalid Stage for a Type 2 Firework!" << std::endl;
			return;
		}
	}
	else if (fireworkType == 3)
	{
		if (fireworkStage == 1)
		{
			//Firework Type 3 Stage 1 (Activated by the user pressing the 3 key)
			newParticleToAdd = new nPhysics::cParticle(5.0f, glm::vec3(nPhysics::getRandom(-5.0f, 5.0f), 0.1f, nPhysics::getRandom(-5.0f, 5.0f)));
			//newParticleToAdd->SetPosition(glm::vec3(nPhysics::getRandom(-5.0f, 5.0f), 0.1f, nPhysics::getRandom(-5.0f, 5.0f)));
			newParticleToAdd->SetVelocity(glm::vec3(6.0f, nPhysics::getRandom(20.0f, 50.0f), 6.0f));		//Launch the particle from the base with an upwards magnitude between 20 and 50
			newParticleToAdd->SetAcceleration(glm::vec3(0.0f, -3.2f, 0.0f));								//Set the acceleration of the particle
			newParticleToAdd->SetDamping(0.15f);															//Set the Damping to 0.15 (kinda just playing around with it)
			newParticleToAdd->SetFireworkType(3);															//Set the type to 3
			newParticleToAdd->SetFireworkStage(1);															//Set the stage to 1
			newParticleToAdd->SetIsAlive(true);																//Set isAlive to true
			newParticleToAdd->SetTimeToDie(5.0f);															//Set how long the particle is alive for
			scale = 12.0f;																					//Set the scale of the bunny model
			colour = glm::vec3(0.0f, 0.0f, 1.0f);															//Set the colour of the bunny model
		}
		else if (fireworkStage == 2)
		{
			newParticleToAdd = new nPhysics::cParticle(5.0f, position);
			newParticleToAdd->SetVelocity(nPhysics::getRandomVec3Circle(20));								//Launch the particle from the position of the stage 1 particle with a random angle of the upper half of a semi circle
			newParticleToAdd->SetAcceleration(glm::vec3(0.0f, -4.2f, 0.0f));								//Set the acceleration of the particle
			newParticleToAdd->SetDamping(0.15f);															//Set the Damping to 0.15 (kinda just playing around with it)
			newParticleToAdd->SetFireworkType(3);															//Set the type to 3
			newParticleToAdd->SetFireworkStage(2);															//Set the stage to 2
			newParticleToAdd->SetIsAlive(true);																//Set isAlive to true
			newParticleToAdd->SetTimeToDie(3.0f);															//Set how long the particle is alive for
			scale = 6.0f;																					//Set the scale of the bunny model
			colour = glm::vec3(nPhysics::getRandom(0.0f, 255.0f) / 255.0f, nPhysics::getRandom(0.0f, 255.0f) / 255.0f, 0.5f);	//Set the colour of the bunny model
		}
		else if (fireworkStage == 3)
		{
			newParticleToAdd = new nPhysics::cParticle(5.0f, position);
			newParticleToAdd->SetVelocity(nPhysics::getRandomVec3Circle(10));								//Almost a copy of stage 2, lower magnitude
			newParticleToAdd->SetAcceleration(glm::vec3(0.0f, -5.2f, 0.0f));								//and higher acceleration
			newParticleToAdd->SetDamping(0.15f);															//Set the Damping to 0.15 (kinda just playing around with it)
			newParticleToAdd->SetFireworkType(3);															//Set the type to 3
			newParticleToAdd->SetFireworkStage(3);															//Set the stage to 3
			newParticleToAdd->SetIsAlive(true);																//Set isAlive to true
			newParticleToAdd->SetTimeToDie(1.5f);															//Set how long the particle is alive for
			scale = 3.0f;																					//Set the scale of the bunny model
			colour = glm::vec3(nPhysics::getRandom(0.0f, 255.0f) / 255.0f, nPhysics::getRandom(0.0f, 255.0f) / 255.0f, 1.0f);	//Set the colour of the bunny model
		}
		else
		{
			//Error check
			std::cout << "Invalid Stage for a Type 3 Firework!" << std::endl;
			return;
		}
	}
	else
	{
		//Error check
		std::cout << "Not a firework!" << std::endl;
		return;
	}
	world->AddParticle(newParticleToAdd);							//Add the particle to the world
	fireworks.push_back(newParticleToAdd);							//Add the particle as a firework
	AddMeshes(newParticleToAdd->GetPosition(), scale, colour);		//Make a mesh at the particle's position
} //end of AddParticles
//Only using the bunny model but we need to add a new version of it whenever we make a particle to represent it
void AddMeshes(glm::vec3 position, float scale, glm::vec3 colour)
{
	cMesh newBunny;
	newBunny.meshName = modelLocations[0];
	newBunny.positionXYZ = position;
	newBunny.bOverrideColour = true;
	newBunny.vertexColourOverride = colour;
	newBunny.bIsWireframe = true;
	newBunny.scale = scale;
	//Set everything about the bunny model and then push it to the vector that we're drawing
	g_vecMeshesToDraw.push_back(newBunny);
}

//Functions that call the AddParticles a certain number of times depending ont he type of firework and what stage
void LaunchType1Stage2(glm::vec3 position)
{
	float randomNumberOfParticles = nPhysics::getRandom(10.0f, 100.0f);
	for (int i = 0; i < randomNumberOfParticles; i++)
	{
		AddParticles(1, 2, position);
	}
}
void LaunchType2Stage2(glm::vec3 position)
{
	float randomNumberOfParticles = nPhysics::getRandom(5.0f, 20.0f);
	for (int i = 0; i < randomNumberOfParticles; i++)
	{
		AddParticles(2, 2, position);
	}
	
}
void LaunchType3Stage2(glm::vec3 position)
{
	float randomNumberOfParticles = nPhysics::getRandom(20.0f, 40.0f);
	for (int i = 0; i < randomNumberOfParticles; i++)
	{
		AddParticles(3, 2, position);
	}
	
}
void LaunchType3Stage3(glm::vec3 position)
{
	float randomNumberOfParticles = nPhysics::getRandom(10.0f, 20.0f);
	for (int i = 0; i < randomNumberOfParticles; i++)
	{
		AddParticles(3, 3, position);
	}
}


//Figured out the math for how to do this from https://learnopengl.com/Getting-started/Camera and http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
//Using the mouse position we calculate the direction that the camera will be facing
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//if it's the start of the program this smooths out a potentially glitchy jump
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	//find the offset of where the mouse positions have moved
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	//multiply by sensitivity so that it's not potentially crazy fast
	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;         // The yaw is the rotation around the camera's y-axis (so we want to add the xOffset to it)
	pitch += yOffset;       // The pitch is the rotation around the camera's x-axis (so we want to add the yOffset to it)
	//This limits the pitch so that we can't just spin the camera under/over itself
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	//calculations for the new direction based on the mouse movements
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraTarget = glm::normalize(direction);
} //fly camera