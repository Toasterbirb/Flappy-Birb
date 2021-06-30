#include <iostream>
#include <vector>

// Engine
#include "../../engine/include/RenderWindow.hpp"
#include "../../engine/include/Entity.hpp"
#include "../../engine/include/Utils.hpp"
#include "../../engine/include/Audio.hpp"

// Game
#include "../include/Entry.hpp"


int main(int argc, char* argv[])
{
	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) > 0)
		std::cout << "SDL INIT FAILED: " << SDL_GetError() << "\n";

	// Init SDL_image
	if (!(IMG_Init(IMG_INIT_PNG)))
		std::cout << "IMG_init has failed. Error: " << SDL_GetError() << "\n";

	// Init SDL_ttf
	if (TTF_Init() == -1) {
		std::cout << "TTF_Init has failed. Error: " << TTF_GetError() << "\n";
    	exit(2);
	}

	// Init SDL_mixer
	Audio audio;
	audio.initAudio(MIX_INIT_MP3);


	// Game entry
	Entry entry;
	int errorCode = entry.gameLoop();
	if (errorCode != 0)
	{
		std::cout << "Error running game. Code: " << errorCode << std::endl;
	}

	TTF_Quit();
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();

	return 0;
}
