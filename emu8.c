#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

unsigned short opcode; // two bytes per opcode
unsigned char memory[4096]; // 4K Memory
unsigned char V[16]; // registers, the last one is used for carry flag
unsigned short I; // index register
unsigned short pc; // program counter 
unsigned short stack[16]; 
unsigned short sp; // stack pointer

unsigned char gfx[64][32]; // screen

unsigned char key[16]; // HEX based keypad, 0x0-0xF

unsigned char delay_timer;
unsigned char sound_timer;

unsigned char drawFlag = 0;

int i; // general purpose counter

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
}; // geezus christ


/*
The memory map of the chip8 VM
0x000-0x1FF: chip 8 interpreter (for font set)
0x050-0x0A0: used for the built in 4x5 pixel font
0x200-0xFFF: program ROM and work RAM
*/

void loadROM();
void clearGFXMemory();
void init();
void drawGFX();
void runCycle();


int main(int argc, char **argv)
{
	
	if(argc < 2)
	{
		printf("How to use: ./emu8 chip8application\n\n");
		return 1;
	}else{
		system("clear");
		init();
		loadROM(argv[1]);
	}

	while(1)
	{
		system("clear");
		runCycle();
		if(drawFlag)
			drawGFX();
		usleep(3333);
	}



 return 0;
}

void loadROM(const char *filename)
{

	init();
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		return;
	}

	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);

	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		return ;
	}

	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		return;
	}

	if((4096-512) > lSize)
	{
		int k;
		for(k = 0; k < lSize; ++k)
			memory[k + 512] = buffer[k];
	}
	else
		printf("ROM too large\n\n");

	fclose(pFile);
	free(buffer);
}

void clearGFXMemory()
{
int i;
int b;
	for(i = 0; i < 64; ++i)
	{
		for(b = 0; b < 32; ++b)
		{
			gfx[i][b] = 0;
		}
	}
}

void init()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	int i;

	for(i = 0; i < 64; ++i)
	{
		int b;
		for(b = 0; b < 32; ++b)
		{
			gfx[i][b] = 0;
		}
	
	}

	for(i = 0; i < 16; ++i)
	{
		stack[i] = 0;
		V[i] = 0;
		key[i] = 0;

	}

	for(i = 0; i < 4096; ++i)
	{
		memory[i] = 0;
	}
	
	for(i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}
}

void drawGFX()
{
	int x;
	int y;
	for(y = 0; y < 32; y++)
	{
		for(x = 0; x < 64; x++)
		{
			if(gfx[x][y] != 0)
			{
				printf("\u25A0");
			}else{
				printf(" ");
			}
		}
		printf("\n");
	}
}

void runCycle()
{
opcode = memory[pc] << 8 | memory[pc + 1];
	
	switch(opcode & 0xF000)
	{		
		case 0x0000:
			switch(opcode & 0x000F)
			{
				case 0x0000: 
					system("clear");					
					drawFlag = 1;
					pc += 2;
				break;

				case 0x000E: 
					--sp;			
					pc = stack[sp];	
					pc += 2;
				break;

				default:
					printf("Something went wrong!");
			}
		break;

		case 0x2000: 
			stack[sp] = pc;    
			++sp;
		case 0x1000: 
			pc = opcode & 0x0FFF;
		   	 break;
		case 0x3000: 
			if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x4000: 
			if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x5000:
			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x6000: 
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
		break;
		
		case 0x7000: 
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
		break;
		
		case 0x8000:
			switch(opcode & 0x000F)
			{
				case 0x0000: 
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0001: 
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0002:
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0003:
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0004: 
					if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) 
						V[0xF] = 1;
					else 
						V[0xF] = 0;					
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;					
				break;

				case 0x0005: 
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) 
						V[0xF] = 0;
					else 
						V[0xF] = 1;					
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0006: 
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
				break;

				case 0x0007: 
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
						V[0xF] = 0; 
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];				
					pc += 2;
				break;

				case 0x000E: 
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
				break;

				default:
					printf("Something went wrong!");
			}
		break;
		
		case 0x9000: 
			if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
		break;

		case 0xA000: 
			I = opcode & 0x0FFF;
			pc += 2;
		break;
		
		case 0xB000:
			pc = (opcode & 0x0FFF) + V[0];
		break;
		
		case 0xC000:
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
		break;
	
		case 0xD000: 
{
				unsigned short x = V[(opcode & 0x0F00) >> 8];
  				unsigned short y = V[(opcode & 0x00F0) >> 4];
				unsigned short height = opcode & 0x000F;
				
				unsigned short pixel;
				// THIS HAS TO DRAW A SPRITE

				for (int yline = 0; yline < height; yline++)
				{
					pixel = memory[I + yline];
					for(int xline = 0; xline < 8; xline++)
					{
						if((pixel & (0x80 >> xline)) != 0)
						{
							if(gfx[x+xline][y+yline == 1])
								V[0xF] = 1;                                 
							gfx[x+xline][y+yline] ^=1;
						}
					}
				}
			drawFlag = 1;			
			pc += 2;
}
		break;
			
		case 0xE000:
			switch(opcode & 0x00FF)
			{
				case 0x009E: 
					if(key[V[(opcode & 0x0F00) >> 8]] != 0)
						pc += 4;
					else
						pc += 2;
				break;
				
				case 0x00A1:
					if(key[V[(opcode & 0x0F00) >> 8]] == 0)
						pc += 4;
					else
						pc += 2;
				break;

				default:
					printf("Something went wrong!");
			}
		break;
		
		case 0xF000:
			switch(opcode & 0x00FF)
			{
				case 0x0007: 
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
				break;
								
				case 0x000A: 
				{
					char keyPress = 1;

					for(int i = 0; i < 16; ++i)
					{
						if(key[i] != 0)
						{
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress = 1;
						}
					}

					if(!keyPress)						
						return;

					pc += 2;					
				}
				break;
				
				case 0x0015: 
					delay_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;

				case 0x0018: 
					sound_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;

				case 0x001E:
					if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					I += V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;

				case 0x0029: 
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
				break;

				case 0x0033: // i borrowed this case, no idea what this is doing 
					memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;					
					pc += 2;
				break;

				case 0x0055:
					for (i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						memory[I + i] = V[i];	

					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;

				case 0x0065:
					for (i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						V[i] = memory[I + i];			
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;

				default:
					printf("Something went wrong!");
			}
		break;

		default:
					printf("Something went wrong!");
	}	

	if(delay_timer > 0)
		--delay_timer;

	if(sound_timer > 0)
	{
		if(sound_timer == 1)
		{

		}
		--sound_timer;
	}	

}
