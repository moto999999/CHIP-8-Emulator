#pragma once

#include <fstream>

#define WIDTH 64
#define HEIGHT 32
#define MEM 4096
#define V_LENGTH 16
#define STACK_LENGTH 16
#define KEY_LENGTH 16
#define TOTAL_PIXELS WIDTH*HEIGHT

class Chip8
{
public:
	bool drawFlag = false; // Flag to see if it's needed to draw on the screen

	unsigned short opcode; // Operation Code -- 2 bytes

	/*
	 * Memory map
	 * 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	 * 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	 * 0x200-0xFFF - Program ROM and work RAM
	*/
	unsigned char memory[MEM];

	unsigned char V[V_LENGTH]; // 1 byte per register -- V0 -> VE

	unsigned short I; // Index register
	unsigned short pc; // Program Counter

	/*
	 * Graphics for the Chip 8. It has a total of 2048 pixels (64*32).
	 * The array will hold the state of the pixel (0 or 1)
	*/
	unsigned char gfx[TOTAL_PIXELS];

	/*
	 * Two timer register that count at 60 Hz
	*/
	unsigned char delay_timer;
	unsigned char sound_timer;

	/*
	 * Implement a stack to remember the current location before a jump is performed.
	 * When a jump or call a subroutine is performed, store the pc in the stack before proceeding
	*/
	unsigned short stack[STACK_LENGTH]; // 16 levels of stack
	unsigned short sp; // To remember which level is used, a stack pointer is necessary

	/*
	 * HEX based keyboard -> 0x0 - 0xF
	*/
	unsigned char key[KEY_LENGTH];

	/*
	 * Chip 8 fontset
	*/
	unsigned char chip8_fontset[80] =
	{
	  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	  0x20, 0x60, 0x20, 0x20, 0x70, // 1
	  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	/*
	 * Prepare the system state, initialize all to default values of the system
	*/
	void initialize();

	/*
	 * Load the program into the memory
	*/
	bool loadProgram(const char* rom);

	/*
	 * Get the Size of a File in C++
	 * https://www.joelverhagen.com/blog/2011/03/get-the-size-of-a-file-in-c/
	*/
	int getFileSize(const char* fileName)
	{
		std::ifstream file(fileName, std::ifstream::in | std::ifstream::binary);

		if (!file.is_open())
			return -1;

		file.seekg(0, std::ios::end);
		int fileSize = (int)file.tellg();
		file.close();

		return fileSize;
	}

	/*
	 * Execute an opcode.
	 * First fetch the opcode, decode, execute it and update timers.
	*/
	void emulateCycle();

};
