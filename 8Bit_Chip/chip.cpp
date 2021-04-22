#include "chip.h"
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>

unsigned char chipFontset[80] =
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

Chip8Bit::Chip8Bit() {

};
Chip8Bit::~Chip8Bit() {

};

void Chip8Bit::init() {
	programCounter = 0x200;
	opcode = 0;
	indexRegister = 0;
	stackPointer = 0;

	for (int i = 0; i < 2048; i++)
		gfx[i] = 0;
	for (int i = 0; i < 16; i++)
		stack[i] = key[i] = v[i] = 0;
	for (int i = 0; i < 4096; i++)
		memory[i] = 0;
	for (int i = 0; i < 80; ++i)
		memory[i] = chipFontset[i];
		
	delayTime = 0;
	soundTimer = 0;

	drawFlag = true;
	
	srand(time(NULL));



}

void Chip8Bit::emulatorCycle() {
	opcode = memory[programCounter] << 8 | memory[programCounter + 1];
//0xAF000
	switch (opcode & 0xF000)
	{

		case 0x0000://clear screen
			switch (opcode & 0x000F)
			{
			case 0x0000: // Clears the screen
				for (int i = 0; i < 2048; ++i)
					gfx[i] = 0x0;
				drawFlag = true;
				programCounter += 2;
				break;

			case 0x000E: // Returns from subroutine
				--stackPointer;			
				programCounter = stack[stackPointer];						
				programCounter += 2;		
				break;

			default:
				std::cout << "Unknown upcode: " << opcode << std::endl;
				break;
			}
			break;

		case 0x1000:// jumps to address NNN
			programCounter = opcode & 0x0FFF;
			break;

		case 0x2000://calls subroutine to NNN
			stack[stackPointer] = programCounter;
			++stackPointer;
			programCounter = opcode & 0x0FFF;
			break;

		case 0x3000://skip next instruction if VX == NN
			if (v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				programCounter += 4;
			else
				programCounter += 2;
			break;

		case 0x4000://skip next instrcution if VX != NN
			if (v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				programCounter += 4;
			else
				programCounter += 2;
			break;

		case 0x5000://skip next instruction if VX == NN
			if (v[(opcode & 0x0F00) >> 8] == v[(opcode & 0x00F0)>> 4])
				programCounter += 4;
			else
				programCounter += 2;
			break;

		case 0x6000://set VX = NN
			v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			programCounter += 2;
			break;

		case 0x7000://VX + NN
			v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			programCounter += 2;
			break;
		
		case 0x8000:
			switch (opcode & 0x000F)
			{
			case 0x0000://set VX = VY
				v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4];
				programCounter += 2;
				break;			
			case 0x0001://set VX = VY | VX
				v[(opcode & 0x0F00) >> 8] |= v[(opcode & 0x00F0) >> 4];
				programCounter += 2;
				break;
			case 0x0002://set VX = VY & VX
				v[(opcode & 0x0F00) >> 8] &= v[(opcode & 0x00F0) >> 4];
				programCounter += 2;
				break;
			case 0x0003://set VX = VY ^(XOR) VX
				v[(opcode & 0x0F00) >> 8] ^= v[(opcode & 0x00F0) >> 4];
				programCounter += 2;
				break;

			case 0x0004://add VY to VX. VF = 1 when there is carry and 0 when there isnt
				if (v[(opcode & 0x00F0) >> 4] > (0xFF - v[(opcode & 0x0F00) >> 8]))
					v[0xF] = 1;
				else
					v[0xF] = 0;	
				programCounter += 2;
				break;

			case 0x0005://subtract VY to VX. VF = 0 when there is a borrow and 1 when there isnt
				if (v[(opcode & 0x00F0) >> 4] > v[(opcode & 0x0F00) >> 8])
					v[0xF] = 0;
				else
					v[0xF] = 1;
				programCounter += 2;
				break;

			case 0x0006://shift VX rigth by one. VF is set to the values of the last bit of VX before shifts
				v[0xF] = v[(opcode & 0x0F00) >> 8] & 0x1;
				v[(opcode & 0x0F00) >> 8] >>= 1;
				programCounter += 2;
				break;

			case 0x0007://set VX - VY. VF is set to 0 when there's a borrow and 1 there isnt
				if (v[(opcode & 0x0F00) >> 8] > v[(opcode & 0x00F0) >> 4]) 
					v[0xF] = 0;
				else
					v[0xF] = 1;
				v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4] - v[(opcode & 0x0F00) >> 8];
				programCounter += 2;
				break;

			case 0x000E://shift VX left by one and set VF to most siginificant bit of VX before shift
				v[0xF] = v[(opcode & 0x0F00) >> 8] >> 7;
				v[(opcode & 0x0F00) >> 8] <<= 1;
				default:
					std::cout << "Unknown opcode: " << opcode << std::endl;
					break; 
			}
			break;

		case 0x9000://skip next instruction if VX != VY
			if (v[(opcode & 0x0F00) >> 8] != v[(opcode & 0x00F0) >> 4])
				programCounter += 4;
			else
				programCounter += 2;
			break;

		case 0xA000:// ANNN sets indexRegister to the address NNN
			indexRegister = opcode & 0x0FFF;
			programCounter += 2;
			break;

		case 0xB000:// BNNN jumps to address NNN + V0
			programCounter = (opcode & 0x0FFF) + v[0];
			break;

		case 0xC000:// CNNN sets VX to a random number and NN
			v[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			programCounter += 2;
			break;

		case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
					 // Each row of 8 pixels is read as bit-coded starting from memory location I; 
					 // I value doesn't change after the execution of this instruction. 
					 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, 
					 // and to 0 if that doesn't happen
		{
			unsigned short x = v[(opcode & 0x0F00) >> 8];
			unsigned short y = v[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			v[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[indexRegister + yline];
				for (int xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
						{
							v[0xF] = 1;
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			drawFlag = true;
			programCounter += 2;
		}
		break;
		
		case 0xE000:
			switch (opcode & 0x00FF)
			{
			case 0x009E:
				if (key[v[(opcode & 0x0F00) >> 8]] != 0)
					programCounter += 4;
				else
					programCounter += 2;
				break;

			case 0x00A1:
				if (key[v[(opcode & 0x0F00) >> 8]] == 0)
					programCounter += 4;
				else
					programCounter += 2;
				break;
			default:
				std::cout << "Unkown upcode: " << opcode << std::endl;
				break;
			}
			break;

		case 0xF000:
			switch (opcode & 0x00FF)
			{
			case 0x0007:
				v[(opcode & 0x0F00) >> 8] = delayTime;
				programCounter += 2;
				break;

			case 0x000A: // FX0A: A key press is awaited, and then stored in VX		
			{
				bool keyPress = false;

				for (int i = 0; i < 16; ++i)
				{
					if (key[i] != 0)
					{
						v[(opcode & 0x0F00) >> 8] = i;
						keyPress = true;
					}
				}

				// If we didn't received a keypress, skip this cycle and try again.
				if (!keyPress)
					return;

				programCounter += 2;
			}
			break;

			case 0x0015:
				delayTime = v[(opcode & 0x0F00) >> 8];
				programCounter += 2;
				break;
			
			case 0x0018:
				soundTimer = v[(opcode & 0x0F00) >> 8];
				programCounter += 2;
				break;


			case 0x001E: // FX1E: Adds VX to I
				if (indexRegister + v[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
					v[0xF] = 1;
				else
					v[0xF] = 0;
				indexRegister += v[(opcode & 0x0F00) >> 8];
				programCounter += 2;
				break;

			case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
				indexRegister = v[(opcode & 0x0F00) >> 8] * 0x5;
				programCounter+= 2;
				break;

			case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
				memory[indexRegister] = v[(opcode & 0x0F00) >> 8] / 100;
				memory[indexRegister + 1] = (v[(opcode & 0x0F00) >> 8] / 10) % 10;
				memory[indexRegister + 2] = (v[(opcode & 0x0F00) >> 8] % 100) % 10;
				programCounter += 2;
				break;

			case 0x0055: // FX55: Stores V0 to VX in memory starting at address I					
				for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
					memory[indexRegister + i] = v[i];

				// On the original interpreter, when the operation is done, I = I + X + 1.
				indexRegister += ((opcode & 0x0F00) >> 8) + 1;
				programCounter += 2;
				break;

			case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I					
				for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
					v[i] = memory[indexRegister + i];

				// On the original interpreter, when the operation is done, I = I + X + 1.
				indexRegister += ((opcode & 0x0F00) >> 8) + 1;
				programCounter += 2;
				break;
			default:
				std::cout << "Unkown upcode: " << opcode << std::endl;
				break;
			}
		default:
			std::cout << "Unkown upcode: " << opcode << std::endl;
			break; 
	}

	if (delayTime > 0)
		--delayTime;
	if (soundTimer > 0)
	{
		if(soundTimer == 1)
			std::cout << "BEEP! " << std::endl;
		--soundTimer;
	}
}

void Chip8Bit::debugRender() {
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			if (gfx[(y * 64) + x] == 0)
				std::cout << "0";
			else
				std::cout << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

bool Chip8Bit::loadApplication(const char* filename) {
	init();
	std::cout << "Loading... " << filename << std::endl;
	FILE* pFile = fopen(filename, "rb");

	if (pFile == NULL)
	{
		fputs("File Error!", stderr);
		return false;
	}

	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	std::cout << "File size: " << (int)lSize << std::endl;

	char* buffer = new char[lSize];
	if (buffer == NULL)
	{
		fputs("Memory Error!", stderr);
		return false;
	}
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error!", stderr);
		return false;
	}

	if ((4096 - 512) > lSize) {
		for (int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		std::cout << "Memory Overflow!!!" << std::endl;
	fclose(pFile);
	delete buffer;
	return true;
}
