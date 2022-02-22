#include "Chip8.h"
#include <cstdio>
#include <iostream>

#define APP_DATA 512 // 0x200 in memory
#define BITS_ROW 8

void Chip8::initialize()
{
	pc = 0x200; // Application starts loading at 0x200
	opcode = 0; // Reset current opcode
	I = 0; // Reset index register
	sp = 0; // Reset stack pointer

	// Clear display
	for (int i = 0; i < TOTAL_PIXELS; i++)
		gfx[i] = 0;

	// Clear stack and registers V0-VF
	for (int i = 0; i < V_LENGTH; i++)
	{
		stack[i] = 0;
		V[i] = 0;
	}

	// Clear memory
	for (int i = 0; i < MEM; i++)
		memory[i] = 0;

	// Load fontset
	for (int i = 0; i < 80; i++)
		memory[i] = chip8_fontset[i];

	// Reset timers
	delay_timer = 0;
	sound_timer = 0;
}

bool Chip8::loadProgram(const char* nROM)
{
	FILE* pROM;
	errno_t err;
	if ((err = fopen_s(&pROM, nROM, "rb")) != 0) {
		std::cout << "Couldn't open the ROM";
		return false;
	}

	// Get the size of the ROM
	int size = getFileSize(nROM);
	if (size == -1) // Check if the ROM exists
	{
		std::cout << "Couldn't open the ROM";
		return false;
	}
	else if ((MEM - APP_DATA) < size) {
		std::cout << "ROM is too big for the Chip8 memory";
		return false;
	}

	// Allocate memory to contain the whole file
	char* buffer = (char*)malloc(sizeof(char) * size);
	if (buffer == NULL)
	{
		std::cout << "Memory error";
		return false;
	}

	// Copy the file into the buffer
	auto result = fread(buffer, sizeof(char), size, pROM);
	if (result != size)
	{
		std::cout << "Error while reading the ROM";
		return false;
	}

	// Copy buffer to Chip8 memory
	for (int i = 0; i < size; i++)
		memory[APP_DATA + i] = buffer[i];

	free(buffer); // Free the memory used by the buffer
	fclose(pROM); // Close the file
	return true;
}

void Chip8::emulateCycle()
{
	/*
	 * Fetch
	 * Data is stored in an array in which each address contains one byte.
	 * As one opcode is 2 bytes long, we will need to fetch two successive bytes and merge them to get the actual opcode.
	*/
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Decode and execution
	unsigned short regX = (opcode & 0x0F00) >> 8; // regX based on where the register X is usually located (0x3XNN)
	unsigned short regY = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000)
	{

	case 0x0000:
		switch (opcode & 0x00FF)
		{

		case 0x00E0: // Clears the screen.
			for (int i = 0; i < TOTAL_PIXELS; i++)
				gfx[i] = 0;
			drawFlag = true;
			pc += 2; // Increase the program counter by 2
			break;

		case 0x00EE: // Returns from a subroutine.
			pc = stack[--sp]; // Restore the value of the program counter from the stack
			pc += 2; // Increase the program counter
			break;
		}
		break;

	case 0x1000: // 0x1NNN: Jumps to address NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000: // 0x2NNN: Calls subroutine at NNN
		stack[sp++] = pc; // Save the value of the program counter on the stack and increase it
		pc = opcode & 0x0FFF; // Call the subroutine
		break;

	case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
		if (V[regX] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
		if (V[regX] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
		if (V[regX] == V[regY])
			pc += 4;
		else
			pc += 2;
		break;

	case 0x6000: // 0x6XNN: Sets VX to NN
		V[regX] = (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x7000: // 0x7XNN: Adds NN to VX. (Carry flag is not changed)
		V[regX] = V[regX] + (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x8000:
	{
		switch (opcode & 0x000F) {

		case 0x0000: // 8XY0: Sets VX to the value of VY
			V[regX] = V[regY];
			pc += 2;
			break;

		case 0x0001: // 8XY1: Sets VX to VX or VY. (Bitwise OR operation)
			V[regX] |= V[regY];
			pc += 2;
			break;

		case 0x0002: // 8XY2: Sets VX to VX and VY. (Bitwise AND operation)
			V[regX] &= V[regY];
			pc += 2;
			break;

		case 0x0003: // 8XY3: Sets VX to VX xor VY
			V[regX] ^= V[regY];
			pc += 2;
			break;

		case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
			if (V[regY] > (0xFF - V[regX]))
				V[0xF] = 1; // There's a carry
			else
				V[0xF] = 0;
			V[regX] += V[regY];
			pc += 2;
			break;

		case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (V[regY] > V[regX])
				V[0xF] = 0; // There's a borrow
			else
				V[0xF] = 1;
			V[regX] -= V[regY];
			pc += 2;
			break;

		case 0x0006: // 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
			V[0xF] = V[regX] & 0x1;
			V[regX] >>= 1;
			pc += 2;
			break;

		case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (V[regX] > V[regY]) // VY - VX
				V[0xF] = 0; // There's a borrow
			else
				V[0xF] = 1;
			V[regX] = V[regY] - V[regX];
			pc += 2;
			break;

		case 0x000E: // 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1
			V[0xF] = V[regX] >> 7;
			V[regX] <<= 1;
			pc += 2;
			break;

		default:
			std::cout << "Unknown opcode: [0x" << opcode << "]\n";
		}
		break;
	}

	case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
		if (V[regX] != V[regY])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000: // ANNN: Sets I to the address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000: // BNNN: Jumps to the address NNN plus V0
		pc = (opcode & 0x0FFF) + V[0];
		break;

	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
		V[regX] = (rand() % 255) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000:
		/* 0xDXYN:
		 * Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
		 * Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn�t change after the execution of this instruction.
		 * As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn�t happen
		*/

	{
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0; // Reset register VF
		for (int yLine = 0; yLine < height; yLine++) // Loop over each row
		{
			pixel = memory[I + yLine]; // Fetch the pixel value from the memory starting at location I
			for (int xLine = 0; xLine < BITS_ROW; xLine++) // Loop over 8 bits of one row
			{
				if ((pixel & (0x80 >> xLine)) != 0) // Check if the current evaluated pixel is set to 1 (note that 0x80 >> xline scan through the byte, one bit at the time)
				{
					int pixDisp = (V[regX] + xLine + ((V[regY] + yLine) * 64));
					if (gfx[pixDisp] == 1) // Check if the pixel on the display is set to 1. If it is set, we need to register the collision by setting the VF register
						V[0xF] = 1;
					gfx[pixDisp] ^= 1; // Set the pixel value by using XOR
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (key[V[regX]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (key[V[regX]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // FX07: Sets VX to the value of the delay timer
			V[regX] = delay_timer;
			pc += 2;
			break;

		case 0x000A: // FX0A: A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
		{
			bool  pressed = false;
			for (int i = 0; i < KEY_LENGTH && !pressed; i++)
			{
				if (key[i] != 0)
				{
					pressed = true;
					V[regX] = i;
				}
			}
			if (pressed) // If the key was pressed, increase the program counter. Otherwise, skip the cycle
				pc += 2;
		}
		break;

		case 0x0015: // FX15: Sets the delay timer to VX
			delay_timer = V[regX];
			pc += 2;
			break;

		case 0x0018: // FX18: Sets the sound timer to VX
			sound_timer = V[regX];
			pc += 2;
			break;

		case 0x001E: // FX1E: Adds VX to I
			I += V[regX];
			pc += 2;
			break;

		case 0x0029: // FX29: Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
			I = V[regX] * 0x5;
			pc += 2;
			break;

		case 0x0033: // FX33: Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I+1, and I+2
			memory[I] = V[regX] / 100;
			memory[I + 1] = (V[regX] / 10) % 10;
			memory[I + 2] = (V[regX] % 100) % 10;
			pc += 2;
			break;

		case 0x0055: // FX55: Store the values of registers V0 to VX inclusive in memory starting at address I. I is set to I + X + 1 after operation
			for (int i = 0; i <= regX; i++)
				memory[I + i] = V[i];
			/*
			* Modern interpreters (starting with CHIP48 and SUPER-CHIP in the early 90s) used a temporary variable for indexing,
			* so when the instruction was finished, I would still hold the same value as it did before.
			*/
			//I += regX + 1;
			pc += 2;
			break;

		case 0x0065: // FX65: Fill registers V0 to VX inclusive with the values stored in memory starting at address I. I is set to I + X + 1 after operation
			for (int i = 0; i <= regX; i++)
				V[i] = memory[I + i];
			/*
			* Modern interpreters (starting with CHIP48 and SUPER-CHIP in the early 90s) used a temporary variable for indexing,
			* so when the instruction was finished, I would still hold the same value as it did before.
			*/
			//I += regX + 1;
			pc += 2;
			break;

		default:
			std::cout << "Unknown opcode: [0x" << std::hex << opcode << "]\n";
		}
		break;

	default:
		std::cout << "Unknown opcode: [0x" << std::hex << opcode << "]\n";
	}

	// Update timers
	if (delay_timer > 0)
		delay_timer--;

	if (sound_timer > 0)
	{
		if (sound_timer != 0)
			std::cout << "BEEP!\n"; // TODO: add sound support
		sound_timer--;
	}
}
