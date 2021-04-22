#pragma once

class Chip8Bit {
public:
	Chip8Bit();
	~Chip8Bit();

	bool drawFlag;

	void emulatorCycle();
	void debugRender();
	bool loadApplication(const char* fileName);

	unsigned char gfx[64 * 32];
	unsigned char key[16];
private:
	unsigned short opcode;
	unsigned char memory[4096];
	unsigned char v[16];
	unsigned short indexRegister;
	unsigned short programCounter;
	unsigned char delayTime;
	unsigned char soundTimer;
	unsigned short stack[16];
	unsigned short stackPointer;

	void init();

};
