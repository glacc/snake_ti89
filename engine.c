// Glacc Snake Game Engine Code

#include <tigcclib.h>
#include "texture.h"

#define GetTileOfs(X, Y) ((YTable[(short)Y] + X) << 1)
#define GetBufPtr(X, Y) (Buffer + (Y << 3)*30 + X)

static char *Buffer;

//1 Tile = 4 bytes
//
// Structure:
// short |   byte    |    byte   
// Count | Direction | Texture ID
//
static short *TileTable = 0;

//LUT for 68k, Z80 etc. to save performance.
static short *YTable = 0;

static short TableSize = 0;

static char Width, Height;

static char SnakeX, SnakeY;
static char SnakeDir, SnakeDirOld;
static short SnakeLeng;

static char FoodX, FoodY;

static char EngineInited = 0;
static char GameOver = 0;

static short Difficulty = 0;

//Head1 Head2 Tail1 Tail2
static char TileToUpdate[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};

//Scoring
static long Score = 0;

static short XYLeng = 0;
static short ScoreMultiplier[10] = {100, 140, 180, 220, 260, 300, 350, 400, 450, 500};
static long MaxScore = 0;

static long ScoreBase = 0;
static long ScoreDecreasement = 0;

static long ScoreDecPerStep = 0;
static long ScoreDecFac = 0;

//0x00 - 0x0F New/old direction to texture ID look-up table calculated by hand
//0x10 - 0x13 Head
//0x14 - 0x17 Tail
static short TextureLUT[0x18] = {5, 1, 0, 2, 3, 6, 2, 0, 0, 4, 5, 3, 4, 0, 1, 6, 7, 8, 9, 10, 11, 12, 13, 14};

void ScoreInit()
{
	ScoreBase = 0;
	ScoreDecreasement = 0;

	XYLeng = Width + Height;
	MaxScore = 10000 * (long)ScoreMultiplier[Difficulty];
	
	ScoreDecFac = ((MaxScore / TableSize / XYLeng) << 1) / 3;
}

short EngineRand()
{
	return rand();
}

void EngineSetSeed(short NewSeed)
{
	srand((long)NewSeed * 64525);
}

void EngineSetDir(char Dir)
{
	switch (SnakeDirOld)
	{
		case 0:
			if (Dir == 2) return;
			break;
		case 1:
			if (Dir == 3) return;
			break;
		case 2:
			if (Dir == 0) return;
			break;
		case 3:
			if (Dir == 1) return;
			break;
	}
	SnakeDir = Dir;
}

long EngineGetScore()
{
	return Score;
}

void EngineDispose()
{
	EngineInited = 0;

	if (TileTable != 0) free(TileTable);
	if (YTable != 0) free(YTable);
	
	TileTable = YTable = 0;
}

char EngineInit(char InitDifficulty, char InitWidth, char InitHeight, char *InitBuffer)
{
	if (EngineInited) EngineDispose();
	
	Buffer = InitBuffer;
	
	//Limitations
	if (InitWidth < 8) InitWidth = 8;
	if (InitHeight < 8) InitHeight = 8;
	
	if (InitDifficulty > 9) InitDifficulty = 9;
	if (InitDifficulty < 0) InitDifficulty = 0;
	
	Width = InitWidth;
	Height = InitHeight;
	
	TableSize = Width * Height;
	
	YTable = malloc(Height << 1);
	TileTable = calloc((Width * Height) << 2, 1);
	
	if ((!TileTable) || (!YTable)) return 0;
	
	short i = 0;
	while (i < Height)
	{
		YTable[i] = i * Width;
		i ++ ;
	}
	
	//Initalize variables
	GameOver = 0;
	Difficulty = InitDifficulty;
	Score = 0;
	
	SnakeX = SnakeY = 3;
	SnakeDirOld =	SnakeDir = 1;
	SnakeLeng = 3;
	
	FoodX = (rand() % (Width - 4)) + 4;
	FoodY = (rand() % (Height - 4)) + 4;
	
	ScoreInit();
	
	EngineInited = 1;
	
	return 1;
}

char EngineStep()
{
	if (!EngineInited) return 2;
	if (GameOver) return 0;

	//New head position
	SnakeDir &= 3;
	
	//Update Head2 tile info for tail drawing
	TileToUpdate[1][0] = SnakeX;
	TileToUpdate[1][1] = SnakeY;
	char OldNew = SnakeDir | (SnakeDirOld << 2);
	TileTable[GetTileOfs(SnakeX, SnakeY) + 1] = TextureLUT[(short)OldNew] | (SnakeDir << 8);

	short NewX = SnakeX;
	short NewY = SnakeY;

	switch (SnakeDir)
	{
		case 0:
			NewY -- ;
			break;
		case 1:
			NewX ++ ;
			break;
		case 2:
			NewY ++ ;
			break;
		case 3:
			NewX -- ;
			break;
	}
	
	if (NewX >= Width) NewX = 0;
  if (NewX < 0) NewX = Width - 1;
  
	if (NewY >= Height) NewY = 0;
  if (NewY < 0) NewY = Height - 1;
  
  SnakeX = NewX;
  SnakeY = NewY;
	
	//Food collision detection
	short FoodFlag = 0;
	
	if (SnakeX == FoodX && SnakeY == FoodY && SnakeLeng < TableSize)
	{
		ScoreBase = MaxScore * (SnakeLeng - 2) / (TableSize - 2);
		
		long RemainLeng = (TableSize - SnakeLeng);
		ScoreDecPerStep = ScoreDecFac * RemainLeng * RemainLeng / TableSize / TableSize;
		
		SnakeLeng ++ ;
		
		FoodFlag = 1;
	}
	
	//Body update
	short *TilePtr = TileTable;
	
	if (!FoodFlag)
	{
		char PY = 0;
		while (PY < Height)
		{
			char PX = 0;
			while (PX < Width)
			{
				if (*TilePtr > 0)
				{
					//Count --
					*TilePtr = *TilePtr - 1;
					
					if (*TilePtr == 0) //Tail1
					{
						TileToUpdate[2][0] = PX;
						TileToUpdate[2][1] = PY;
					}
					if (*TilePtr == 1) //Tail2
					{
						TileToUpdate[3][0] = PX;
						TileToUpdate[3][1] = PY;
						short OrigTile = *(TilePtr + 1);
						*(TilePtr + 1) = (OrigTile & 0xFF00) | TextureLUT[(OrigTile >> 8) + 0x14];
					}
				}
				
				//Clear texture and direction value
				if (*TilePtr == 0) *(TilePtr + 1) = 0;
				
				//Next tile
				TilePtr += 2;
				PX ++ ;
			}
			PY ++ ;
		}
	}
	
	//Head collision detection
	short HeadOfs = GetTileOfs(SnakeX, SnakeY);
	
	if (TileTable[HeadOfs] != 0)
	{
		GameOver = 1;
		return 0;
	}
	
	//Update Head1 position and tile info
  TileToUpdate[0][0] = SnakeX;
  TileToUpdate[0][1] = SnakeY;
  
	TileTable[HeadOfs] = SnakeLeng;
	TileTable[HeadOfs + 1] = (TextureLUT[SnakeDir + 0x10] & 0xFF) | (SnakeDir << 8);
	
	//Food update
	while (TileTable[GetTileOfs(FoodX, FoodY)] != 0 && SnakeLeng < TableSize)
	{
		FoodX = rand() % Width;
		FoodY = rand() % Height;
	}
	
	SnakeDirOld = SnakeDir;
	
	if (SnakeLeng > 3) ScoreDecreasement += ScoreDecPerStep;
	Score = (ScoreBase - ScoreDecreasement) / 25;
	if (Score < 0) Score = 0;
	
	return 1;
}

void EngineDraw(char Entire)
{
	short PX = 0;
	short PY = 0;
	
	void DrawTexture(char TextureID, char *DrawPtr)
	{
		short TexturePtrOfs = TextureID << 3;
		char i = 0;
		while (i < 8)
		{
			*DrawPtr = SnakeTexture[TexturePtrOfs];
			DrawPtr += 30;
			TexturePtrOfs ++ ;
			i ++ ;
		}
	}
	
	if (Entire)
	{
		short *Ptr = TileTable + 1;
		while (PY < Height)
		{
			short YOfs = (PY << 3) * 30;
			PX = 0;
			while (PX < Width)
			{
				//Texture
				short TexturePtrOfs = *Ptr & 0xFF;
				char *DrawPtr = Buffer + YOfs + PX;
				DrawTexture(TexturePtrOfs, DrawPtr);
				
				Ptr += 2;
				PX ++ ;
			}
			PY ++ ;
		}
	}
	else
	{
		short i = 0;
		while (i < 4)
		{
			char PX = TileToUpdate[i][0];
			char PY = TileToUpdate[i][1];
			short TileOfs = GetTileOfs(PX, PY);
			DrawTexture((TileTable[TileOfs + 1] & 0xFF), GetBufPtr(PX, PY));
			
			i ++ ;
		}
	}
	
	/*
	short TexturePtrOfs = 0xF << 3;
	char *DrawPtr = Buffer + (FoodY << 3) * 30 + FoodX;
	char i = 0;
	while (i < 8)
	{
		*DrawPtr = SnakeTexture[TexturePtrOfs];
		DrawPtr += 30;
		TexturePtrOfs ++ ;
		i ++ ;
	}
	*/
	
	if (SnakeLeng < TableSize) DrawTexture(15, GetBufPtr(FoodX, FoodY));
}