#include <vector>
#include <birb2d/Logger.hpp>
#include <birb2d/Renderwindow.hpp>
#include <birb2d/Timestep.hpp>
#include <birb2d/Audio.hpp>
#include <birb2d/Values.hpp>
#include <birb2d/Physics.hpp>

void GeneratePipes(std::vector<Birb::Entity> *topPipes, std::vector<Birb::Entity> *bottomPipes, SDL_Texture* pipeTexture);
void LoopPipes(std::vector<Birb::Entity> *topPipes, std::vector<Birb::Entity> *bottomPipes);

/* Pipe variables */
int topPipeHeight = -300;
int pipeDistance = 400;
float pipeSizeMultiplier = 4.5f;
int pipeHeightVariation = 50;

int main(int argc, char** argv)
{
	Birb::Window window("Flappy Birb", Birb::Vector2int(1280, 720), 75, false);
	Birb::Audio::Init(0);
	Birb::TimeStep timeStep;
	timeStep.Init(&window);
	
	/* Load resources */
	SDL_Texture* skyTexture 	= Birb::Resources::LoadTexture("./res/gfx/sky.png");
	SDL_Texture* groundTexture 	= Birb::Resources::LoadTexture("./res/gfx/ground.png");
	SDL_Texture* birbTexture 	= Birb::Resources::LoadTexture("./res/gfx/birb.png");
	SDL_Texture* pipeTexture 	= Birb::Resources::LoadTexture("./res/gfx/pipe.png");
	TTF_Font* manaspace 		= Birb::Resources::LoadFont("./res/fonts/manaspace/manaspc.ttf", 32);
	Birb::Audio::SoundFile jumpSound("./res/audio/sound_fx/jump.wav");
	Birb::Audio::SoundFile gameOverSound("./res/audio/sound_fx/game_over.wav");
	

	/* --- Entities --- */
	std::vector<Birb::Entity*> entities;

	/* Sky */
	Birb::Entity skyEntity("Sky", Birb::Rect(0, 0, window.window_dimensions.x, window.window_dimensions.y), skyTexture);
	entities.push_back(&skyEntity);

	/* Ground */
	Birb::Entity groundEntity("Ground", Birb::Rect(0, window.window_dimensions.y - 64, 1280, 64), groundTexture);
	entities.push_back(&groundEntity);

	/* Sky ground */
	Birb::Entity skyGroundEntity("Ground", Birb::Rect(0, 0, 1280, 64), groundTexture);
	skyGroundEntity.angle = 180;
	entities.push_back(&skyGroundEntity);

	/* Birb */
	int originalBirbHeight = 300;
	Birb::Entity birbEntity("Birb", Birb::Vector2int(window.window_dimensions.x / 4, originalBirbHeight), birbTexture);
	entities.push_back(&birbEntity);

	/* Game over text */
	Birb::Entity gameOverText("Game over", Birb::Vector2int(window.window_dimensions.x / 2, window.window_dimensions.y / 2 - 50), Birb::EntityComponent::TextComponent("Game Over", manaspace, &Birb::Colors::Black));
	gameOverText.active = false;
	gameOverText.localScale = Birb::Vector2f(2, 2);
	gameOverText.rect.x -= gameOverText.rect.w; // Center the text
	entities.push_back(&gameOverText);

	/* Game restart hint text */
	Birb::Entity restartHintText("Restart hint", Birb::Vector2int(window.window_dimensions.x / 2, window.window_dimensions.y / 2 + 20), Birb::EntityComponent::TextComponent("Hit space to restart", manaspace, &Birb::Colors::Black));
	restartHintText.active = false;
	restartHintText.rect.x -= restartHintText.rect.w / 2; // Center the text
	entities.push_back(&restartHintText);

	/* Setup pipes */
	std::vector<Birb::Entity> topPipes;
	std::vector<Birb::Entity> bottomPipes;
	int pipeMovementSpeed = 150.0f;
	GeneratePipes(&topPipes, &bottomPipes, pipeTexture);


	/* --- Gameloop variables --- */
	bool jumpQueued = false;
	float jumpForce = 70.0f;
	float fallForce = -200.0f;
	float fallSpeedMultiplier = 0;
	bool birbAlive = true;


	bool gameRunning = true;
	while (gameRunning)
	{
		timeStep.Start();
		while (timeStep.Running())
		{
			/* Handle input */
			while (window.PollEvents())
			{
				window.EventTick(window.event, &gameRunning);

				if (window.event.type == SDL_KEYDOWN)
				{
					switch (window.event.key.keysym.scancode)
					{
						case (SDL_SCANCODE_SPACE):
							if (birbAlive)
							{
								jumpQueued = true;
							}
							else // Reset the game
							{
								gameOverText.active = false;
								restartHintText.active = false;
								birbEntity.rect.y = originalBirbHeight;
								GeneratePipes(&topPipes, &bottomPipes, pipeTexture);
								birbAlive = true;
							}

							break;

						default:
							break;
					}
				}
			}

			timeStep.Step();
		}

		/* --- Game Logic --- */

		/* Jumping */
		if (birbAlive)
		{
			if (jumpQueued)
			{
				birbEntity.rect.y -= jumpForce;
				jumpSound.play();
				fallSpeedMultiplier = 0;
				jumpQueued = false;
			}
			else
			{
				/* Drag the birb down */
				birbEntity.rect.y -= timeStep.deltaTime * fallForce * fallSpeedMultiplier;

				if (fallSpeedMultiplier <= 1.0f)
					fallSpeedMultiplier += timeStep.deltaTime * 2;
			}
		}

		/* Move pipes to the left */
		if (birbAlive)
		{
			for (int i = 0; i < 4; i++)
			{
				topPipes[i].rect.x -= pipeMovementSpeed * timeStep.deltaTime;
				bottomPipes[i].rect.x -= pipeMovementSpeed * timeStep.deltaTime;
			}
		}
		LoopPipes(&topPipes, &bottomPipes);


		/* Handle ground collision */
		if (birbAlive && (Birb::Physics::EntityCollision(birbEntity, groundEntity) || Birb::Physics::EntityCollision(birbEntity, skyGroundEntity)))
		{
			birbAlive = false;
			gameOverSound.play();
			gameOverText.active = true;
			restartHintText.active = true;
		}

		/* Pipe collisions */
		if (birbAlive)
		{
			for (int i = 0; i < 4; i++)
			{
				if (Birb::Physics::EntityCollision(birbEntity, topPipes[i]) || Birb::Physics::EntityCollision(birbEntity, bottomPipes[i]))
				{
					birbAlive = false;
					gameOverSound.play();
					gameOverText.active = true;
					restartHintText.active = true;
					break;
				}
			}
		}

		/* --- Rendering --- */
		window.Clear();

		/* Render all entities (except pipes) */
		for (int i = 0; i < entities.size(); i++)
		{
			Birb::Render::DrawEntity(*entities[i]);
		}

		/* Render pipes */
		for (int i = 0; i < topPipes.size(); i++)
		{
			Birb::Render::DrawEntity(topPipes[i]);
			Birb::Render::DrawEntity(bottomPipes[i]);
		}

		window.Display();

		timeStep.End();
	}

	window.Cleanup();
	return 0;
}

void GeneratePipes(std::vector<Birb::Entity> *topPipes, std::vector<Birb::Entity> *bottomPipes, SDL_Texture* pipeTexture)
{
	int pipeStartDistance = 500;

	/* If there are already pipes in the lists, destroy them */
	if (topPipes->size() > 0)
		topPipes->clear();

	if (bottomPipes->size() > 0)
		bottomPipes->clear();

	/* Top pipes */
	int height;
	for (int i = 0; i < 4; i++)
	{
		height = topPipeHeight + Birb::utils::randomInt(-pipeHeightVariation, pipeHeightVariation);
		Birb::Entity topPipe("Top pipe", Birb::Vector2int(pipeDistance * (i + 1) + pipeStartDistance, height), pipeTexture);
		topPipe.localScale = Birb::Vector2f(pipeSizeMultiplier, pipeSizeMultiplier);
		topPipe.angle = 180;
		topPipes->push_back(topPipe);
	}

	/* Bottom pipes */
	for (int i = 0; i < 4; i++)
	{
		height = topPipes->at(i).rect.y + 800;
		Birb::Entity bottomPipe("Bottom pipe", Birb::Vector2int(pipeDistance * (i + 1) + pipeStartDistance, height), pipeTexture);
		bottomPipe.localScale = Birb::Vector2f(pipeSizeMultiplier, pipeSizeMultiplier);
		bottomPipes->push_back(bottomPipe);
	}
}

void LoopPipes(std::vector<Birb::Entity> *topPipes, std::vector<Birb::Entity> *bottomPipes)
{
	/* Check if there's a pipe outside of the screen bounds */
	int index = -1;
	int currentFurthestDistance = -1;
	for (int i = 0; i < 4; i++)
	{
		if (topPipes->at(i).rect.x < -topPipes->at(i).rect.w * pipeSizeMultiplier)
		{
			index = i;
		}

		if (topPipes->at(i).rect.x > currentFurthestDistance)
			currentFurthestDistance = topPipes->at(i).rect.x;
	}

	if (index == -1)
		return;

	/* Move the 2 pipes that are out of bounds to a new position */
	topPipes->at(index).rect.x = currentFurthestDistance + pipeDistance;
	bottomPipes->at(index).rect.x = currentFurthestDistance + pipeDistance;

	/* Randomize the height */
	topPipes->at(index).rect.y = topPipeHeight + Birb::utils::randomInt(-pipeHeightVariation, pipeHeightVariation);
	bottomPipes->at(index).rect.y = topPipes->at(index).rect.y + 800;
}
