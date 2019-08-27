#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Chip8.h"

#define PIXEL_SIZE 10

class EmuRender : public olc::PixelGameEngine
{
public:
	EmuRender()
	{
		sAppName = "Chip 8 Emulator";
	}

	bool OnUserCreate() override
	{
		chip8.initialize();
		if (!chip8.loadProgram("../roms/INVADERS")) // TODO: better rom selection
			return false;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		/*
		 * The chip 8 has a ~500Hz CPU and has a refresh rate of 60Hz
		 * 500Hz / 60Hz = 8.33 cycles/frame --> 8 cycles/frame
		*/
		for (int cycles = 0; cycles <= 8; cycles++)
			chip8.emulateCycle();

		// If the draw flag is set, update the screen
		if (chip8.drawFlag)
		{
			for (int j = 0; j < HEIGHT; j++)
				for (int i = 0; i < WIDTH; i++)
				{
					if (chip8.gfx[(j * 64) + i] == 0)
						Draw(i, j, olc::BLACK);
					else
						Draw(i, j, olc::Pixel(191, 0, 0));
				}
			chip8.drawFlag = false; // The screen has been updated, disable the flag
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Sleep thread for 16ms (limit to ~60fps)
		return true;
	}

private:
	Chip8 chip8 = Chip8();
};

int main(int argc, char* argv[])
{
	EmuRender rend;
	if (rend.Construct(WIDTH, HEIGHT, PIXEL_SIZE, PIXEL_SIZE))
		rend.Start();

	return 0;
}