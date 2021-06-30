#include <iostream>
#include <vector>
#include <string.h>

// Engine
#include "../../engine/include/RenderWindow.hpp"
#include "../../engine/include/Entity.hpp"
#include "../../engine/include/Utils.hpp"
#include "../../engine/include/Audio.hpp"
#include "../../engine/include/Events.hpp"

// Game
#include "../include/Entry.hpp"
#include "../include/FPS.hpp"

// PrePros vars
#define INPUT_DEBUG
#undef INPUT_DEBUG

int Entry::gameLoop()
{
	RenderWindow window("Game v1.0", 1280, 720);

	int refresh_rate = 240;
	std::cout << "Refresh rate: " << window.getRefreshRate() << " | Custom (" << refresh_rate << ")\n";

	SDL_Texture* birbTexture = window.loadTexture("./res/gfx/birb.png");

	SDL_Texture* skyTexture = window.loadTexture("./res/gfx/sky.png");
	Entity sky("Sky", Vector2f(0, 0), Vector2f(1280, 720), skyTexture);

	SDL_Texture* groundTexture = window.loadTexture("./res/gfx/ground.png");
	Entity ground("Ground", Vector2f(0, 688), Vector2f(1280, 32), groundTexture);

	SDL_Color color = {0, 0, 0};
	TTF_Font* manaspace = window.loadFont("./res/fonts/manaspace/manaspc.ttf", 32);
	Font font(manaspace, color, 32);


	std::vector<Entity> entities = {};

	SDL_Renderer* renderer = window.getRenderer();

	// Create birb entity and destroy it after pushing to the std::vector
	int pipeCount = 4;
	int pipeDistance = 400;
	int pipeStartDistance = 500;
	SDL_Rect scoreCollider;
	scoreCollider.x = pipeDistance + pipeStartDistance - 16;
	scoreCollider.y = 0;
	scoreCollider.w = 32;
	scoreCollider.h = 1280;
	{
		Entity birb("Birb", Vector2f(window.getDimensions().x / 4, window.getDimensions().y / 2), Vector2f(64, 64), birbTexture);
		entities.push_back(birb);

		// Create pipe entities
		SDL_Texture* pipeTexture = window.loadTexture("./res/gfx/pipe.png");

		// Crete bottom pipes
		for (int i = 0; i < pipeCount; i++)
		{
			char pipeName[32];
			std::snprintf(pipeName, 32, "Pipe_bottom %d", i);
			Entity bottomPipe(pipeName, Vector2f(pipeDistance * (i + 1) + pipeStartDistance, 500), 4.5, Vector2f(32, 128), pipeTexture);
			entities.push_back(bottomPipe);
		}

		// Create top pipes
		for (int i = 0; i < pipeCount; i++)
		{
			char pipeName[32];
			std::snprintf(pipeName, 32, "Pipe_top %d", i);
			Entity bottomPipe(pipeName, Vector2f(pipeDistance * (i + 1) + pipeStartDistance, -300), 4.5, Vector2f(32, 128), 180, pipeTexture);
			entities.push_back(bottomPipe);
		}
	}

	//MusicFile mainSong("./res/audio/music/Imaginary_Places.mp3");
	//mainSong.play();
	
	// Load sound effects
	SoundFile jumpSound("./res/audio/sound_fx/jump.wav");
	SoundFile scoreSound("./res/audio/sound_fx/score.wav");
	

	// Framerate counter + score
	int score = 0;
	char counterBuf[32];
	Entity frameText("Frame counter", counterBuf, Vector2f(8, 8), &font);
	SDL_Texture* frameText_tex = window.renderTextEntity(frameText);
	frameText.updateText(counterBuf, frameText_tex);



	bool gameRunning = true;
	
	SDL_Event event;

	const float timeStep = 0.01f;
	float accumulator = 0.01f;
	float currentTime = utils::hireTimeInSeconds();
	bool dragging = true;

	/* Game loop
	 * 1. Timestep vars
	 * 2. Events (keypresses etc.)
	 * 3. Game logic
	 * 4. Rendering
	 * 5. Audio
	 */

	// Game variables
	bool jumping = false;
	bool holdingSpace = false;
	float jumpTime = 0;
	bool birbInScoreCollider = false;
	// -------------
	
	// Classes
	FPS fps_class(refresh_rate);
	// -------
	

	// Print out all of the entities before the game loop starts
	std::cout << "Entity list:" << std::endl;
	for (int i = 0; i < entities.size(); i++)
	{
		std::cout << i << ": " << entities[i].getName() << " ; Pos: " << entities[i].getPos().x << ", " << entities[i].getPos().y << std::endl;
	}

	while (gameRunning)
	{
		int startTick = SDL_GetTicks();

		float newTime = utils::hireTimeInSeconds();
		float frameTime = newTime - currentTime;

		if (frameTime > 0.25f)
			frameTime = 0.25f;

		currentTime = newTime;
		accumulator += frameTime;

		while (accumulator >= timeStep)
		{
			while (SDL_PollEvent(&event) != 0)
			{
				#ifdef INPUT_DEBUG
				std::cout << std::hex << "Event keycode: 0x" << event.key.keysym.sym << std::endl;
				#endif

				// Get our controls and events
				if (event.type == SDL_QUIT)
				{
					gameRunning = false;
					break;
				}

				switch (event.type)
				{
					case (SDL_KEYUP):
						if (event.key.keysym.sym == SDLK_SPACE)
							holdingSpace = false;
						break;
				}

				switch (event.key.keysym.sym)
				{
					case (SDLK_SPACE):
						if (!jumping && event.type == SDL_KEYDOWN && !holdingSpace)
						{
							jumpSound.play();
							jumping = true;
							holdingSpace = true;
							jumpTime = currentTime + 0.1;
						}
						break;

					default:
						break;
				}
			}

			// ------ Game logic -------
			
			// Birb jumping mechanic / gravity
			if (!jumping)
			{
				entities[0].setPos(Vector2f(entities[0].getPos().x, entities[0].getPos().y + 2));
			}
			else
			{
				if (currentTime >= jumpTime)
					jumping = false;
				else
					entities[0].setPos(Vector2f(entities[0].getPos().x, entities[0].getPos().y - 8));
			}

			// Moving pipes
			float lastX = -500;
			int bottomIndex;
			for (int i = 1; i < pipeCount * 2 + 1; i++)
			{
				Vector2f pipePos = entities[i].getPos();

				if (pipePos.x > -150)
				{
					// Pipe is on the screen or off the screen to the right
					entities[i].setPos(Vector2f(entities[i].getPos().x - 2 - (currentTime / 100), entities[i].getPos().y));
				}
				else
				{
					// Pipe has passed off the screen to the left and needs to be recycled
					// Also set the height to a random value
					if (lastX == -500)
					{
						float randomHeight;

						if (pipePos.y + randomHeight > 620)
						{
							randomHeight = utils::randomFloat(-50, 0);
						}
						else if (pipePos.y + randomHeight < 250)
						{
							randomHeight = utils::randomFloat(0, 50);
						}
						else
						{
							randomHeight = utils::randomFloat(-50, 50);
						}

						entities[i].setPos(Vector2f(pipePos.x + (pipeDistance * pipeCount), pipePos.y + randomHeight));
						lastX = round(entities[i].getPos().x);
						bottomIndex = i;
					}
					else if (i == bottomIndex + pipeCount)
					{
						entities[i].setPos(Vector2f(pipePos.x + (pipeDistance * pipeCount), entities[i - pipeCount].getPos().y - 800));
						lastX = -500;
					}
				}
			}

			// Find the pipes that are closest to the birb on the right side
			int closestPipe = 1;
			for (int i = 2; i < pipeCount * 2 + 1; i++)
			{
				if (entities[closestPipe].getPos().x < window.getDimensions().x / 4)
				{
					closestPipe++;
					continue;
				}

				Vector2f pipePos = entities[i].getPos();
				if (pipePos.x > window.getDimensions().x / 4)
				{
					if (pipePos.x < entities[closestPipe].getPos().x)
					{
						closestPipe = i;
					}
				}
			}
			
			// Move score collider to the closest pipe position
			scoreCollider.x = entities[closestPipe].getPos().x + 50;

			if (scoreCollider.x < window.getDimensions().x / 4)
				std::cout << "Invalid score collider position" << std::endl;
			
			// Check if birb has entered to score collider and handle scoring
			if (!birbInScoreCollider)
			{
				if (SDL_HasIntersection(entities[0].getRect(), &scoreCollider))
				{
					birbInScoreCollider = true;
				}
			}
			else if (!SDL_HasIntersection(entities[0].getRect(), &scoreCollider))
			{
				birbInScoreCollider = false;
				
				// Increase score
				score++;

				// Play score sound effect
				scoreSound.play();
			}


			// Collision checking
			// Ground
			if (SDL_HasIntersection(entities[0].getRect(), ground.getRect()))
			{
				std::cout << "Ground collision!" << std::endl;
				gameRunning = false;
			}

			// Pipes
			for (int i = 1; i < pipeCount * 2 + 1; i++)
			{
				if (SDL_HasIntersection(entities[0].getRect(), entities[i].getRect()))
				{
					std::cout << "Pipe collision!" << std::endl;
					gameRunning = false;
				}
			}

			// -------------------------

			accumulator -= timeStep;
		}
		const float alpha = accumulator / timeStep; // 50%??

		window.clear();


		// Render sky before anything else
		window.render(sky);

		// Rendering
		for (Entity& e : entities)
		{
			window.render(e);
		}

		// Render ground
		window.render(ground);

		// Updating framerate text
		int fps = fps_class.calculateFPS(frameTime);

		std::snprintf(counterBuf, 32, "FPS: %d  Score: %d", fps, score);
		frameText_tex = window.renderTextEntity(frameText);
		frameText.updateText(counterBuf, frameText_tex);
		
		// Render framerate text
		window.render(frameText);

		// Score collider debugging
		//SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		//SDL_RenderDrawLine(renderer, scoreCollider.x, 0, scoreCollider.x, 1280);
		//SDL_RenderDrawLine(renderer, scoreCollider.x + scoreCollider.w, 0, scoreCollider.x + scoreCollider.w, 1280);

		window.display();

		int frameTicks = SDL_GetTicks() - startTick;

		if (frameTicks < 1000 / refresh_rate)
			SDL_Delay(1000 / refresh_rate - frameTicks);
	}

	//mainSong.free();
	jumpSound.free();
	scoreSound.free();
	window.cleanUp();
	TTF_CloseFont(manaspace);

	return 0;
}
