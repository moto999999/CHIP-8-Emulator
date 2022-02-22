//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <iostream>
#include "Chip8.h"

//Screen dimension constants (10x chip8 resolution)
#define SCREEN_WIDTH WIDTH*10
#define SCREEN_HEIGHT HEIGHT*10

//Starts up SDL and creates window
bool init(SDL_Window** window, SDL_Renderer** renderer);

//Frees media and shuts down SDL
void close(SDL_Window** window, SDL_Renderer** renderer);

// Handles key presses
void handleEvent(SDL_Event* e, Chip8* chip8);

int main(int argc, char* args[])
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The window renderer
	SDL_Renderer* renderer = NULL;

	//Start up SDL and create window
	if (!init(&window, &renderer))
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		// Create chip8 object
		Chip8 chip8 = Chip8();

		// Initialize chip8
		chip8.initialize();

		// Set resolution scale
		SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

		//Clear screen
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);

		if (chip8.loadProgram("../roms/PONG")) // TODO: better rom selection
		{
			//While application is running
			while (!quit)
			{
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					handleEvent(&e, &chip8);
				}

				/*
				 * The chip 8 has a ~500Hz CPU and has a refresh rate of 60Hz
				 * 500Hz / 60Hz = 8.33 cycles/frame --> 8 cycles/frame
				*/
				for (int cycles = 0; cycles < 9; cycles++)
					chip8.emulateCycle();

				// If the draw flag is set, update the screen
				if (chip8.drawFlag)
				{
					for (int j = 0; j < HEIGHT; j++)
						for (int i = 0; i < WIDTH; i++)
						{
							if (chip8.gfx[(j * 64) + i] == 0) {
								SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
								SDL_RenderDrawPoint(renderer, i, j);
							}
							else {
								SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
								SDL_RenderDrawPoint(renderer, i, j);
							}
						}
					chip8.drawFlag = false; // The screen has been updated, disable the flag
				}
				//Update screen
				SDL_RenderPresent(renderer);
			}
		}
	}

	//Free resources and close SDL
	close(&window, &renderer);

	return 0;
}

bool init(SDL_Window** window, SDL_Renderer** renderer)
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		*window = SDL_CreateWindow("CHIP-8 emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (*window == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
		}
	}

	return success;
}

void close(SDL_Window** window, SDL_Renderer** renderer)
{
	//Destroy window	
	SDL_DestroyRenderer(*renderer);
	SDL_DestroyWindow(*window);
	*window = NULL;
	*renderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void handleEvent(SDL_Event* e, Chip8* chip8) {
	// Check if a button is pressed
	if (e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch (e->key.keysym.sym)
		{ // If pressed update key array in chip8 object
		case SDLK_1: chip8->key[0] = 1; break;
		case SDLK_2: chip8->key[1] = 1; break;
		case SDLK_3: chip8->key[2] = 1; break;
		case SDLK_4: chip8->key[3] = 1; break;
		case SDLK_q: chip8->key[4] = 1; break;
		case SDLK_w: chip8->key[5] = 1; break;
		case SDLK_e: chip8->key[6] = 1; break;
		case SDLK_r: chip8->key[7] = 1; break;
		case SDLK_a: chip8->key[8] = 1; break;
		case SDLK_s: chip8->key[9] = 1; break;
		case SDLK_d: chip8->key[10] = 1; break;
		case SDLK_f: chip8->key[11] = 1; break;
		case SDLK_z: chip8->key[12] = 1; break;
		case SDLK_x: chip8->key[13] = 1; break;
		case SDLK_c: chip8->key[14] = 1; break;
		case SDLK_v: chip8->key[15] = 1; break;
		}
	}
	// Check if a button is released
	else if (e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch (e->key.keysym.sym)
		{// If released update key array in chip8 object
		case SDLK_1: chip8->key[0] = 0; break;
		case SDLK_2: chip8->key[1] = 0; break;
		case SDLK_3: chip8->key[2] = 0; break;
		case SDLK_4: chip8->key[3] = 0; break;
		case SDLK_q: chip8->key[4] = 0; break;
		case SDLK_w: chip8->key[5] = 0; break;
		case SDLK_e: chip8->key[6] = 0; break;
		case SDLK_r: chip8->key[7] = 0; break;
		case SDLK_a: chip8->key[8] = 0; break;
		case SDLK_s: chip8->key[9] = 0; break;
		case SDLK_d: chip8->key[10] = 0; break;
		case SDLK_f: chip8->key[11] = 0; break;
		case SDLK_z: chip8->key[12] = 0; break;
		case SDLK_x: chip8->key[13] = 0; break;
		case SDLK_c: chip8->key[14] = 0; break;
		case SDLK_v: chip8->key[15] = 0; break;
		}
	}
}
