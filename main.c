// Glacc Snake Game Main

#include <tigcclib.h>
//#include <dialogs.h>
#include "glactxt.h"
#include "engine.h"

#define PTIMER *(volatile unsigned char*)0x600017

INT_HANDLER Interrupt1;
INT_HANDLER Interrupt5;
unsigned char LastRate;
unsigned char CurSpd;
char OrigFont;

char *TileBuffer;
char *LCDBuffer;

char Resuming = 0;
char Redraw = 0;

char Difficulty = 4;
char DifficultyTimer[10] = {33, 30, 27, 24, 21, 18, 15, 13, 11, 9};
short Timer = 0;

long Highscore = 0;

short RandSeed;

DEFINE_INT_HANDLER (RedefInt5)
{
	Timer -- ;
	
	if (Timer <= 0)
	{
		Redraw = 1;
		Timer = DifficultyTimer[(short)Difficulty];
	}
}

void SetSpeed(unsigned char Spd) {
	while (PTIMER != 0);
	while (PTIMER == 0);
	CurSpd = PTIMER = Spd;
}

void IntSet()
{
	Interrupt1 = GetIntVec(AUTO_INT_1);
	Interrupt5 = GetIntVec(AUTO_INT_5);
	SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
	SetIntVec(AUTO_INT_5, RedefInt5);
}

void GetRate()
{
	while (PTIMER != 0);
	while (PTIMER == 0);
	LastRate = PTIMER;
}

void RstRate()
{
	SetSpeed(LastRate);
}

void IntRst()
{	
	SetIntVec(AUTO_INT_1, Interrupt1);
	SetIntVec(AUTO_INT_5, Interrupt5);
	GKeyFlush();
}

void GetHighscore()
{
	FILE *ScoreFile = fopen("gsnakehs", "rb");
	
	if (ScoreFile == NULL)
	{
		char Extension[] = {0, 'g', 'l', 'a', 'c', 0, OTH_TAG};
	
		ScoreFile = fopen("gsnakehs", "wb");
		
		fputc(4, ScoreFile); //Difficulty
		int i = 0;
		while (i < 6)
		{
			fputc(0, ScoreFile);	//Highscore & Random seed
			i ++ ;
		}
		
		fwrite(Extension, 7, 1, ScoreFile);
	}
	else
	{
		fseek(ScoreFile, 0, SEEK_SET);
		fread(&Difficulty, 1, 1, ScoreFile);
		fseek(ScoreFile, 1, SEEK_SET);
		fread(&Highscore, 4, 1, ScoreFile);
		fseek(ScoreFile, 5, SEEK_SET);
		fread(&RandSeed, 2, 1, ScoreFile);
		
		EngineSetSeed(RandSeed);
	}
	
	fclose(ScoreFile);
}

char MainMenuScreen()
{
	char DifficultyStr[2] = {'0' + Difficulty};

	HANDLE MenuHandle = DialogNewSimple(140, 45);
	
	DialogAddTitle(MenuHandle, "Snake Game by Glacc Ver 0.0.2", BT_CANCEL, BT_OK);
	
	DialogAddRequest(MenuHandle, 3, 16, "Difficulty (0-9)", 0, 1, 3);
	
	char HighScoreTxt[48];
	sprintf(HighScoreTxt, "Highest score: %ld", Highscore);
	
	DialogAddText(MenuHandle, 3, 24, HighScoreTxt);
	
	short Result = DialogDo(MenuHandle, CENTER, CENTER, DifficultyStr, NULL);
	
	HeapFree(MenuHandle);
	
	Difficulty = DifficultyStr[0] - '0';
	
	if (Difficulty < 0) Difficulty = 0;
	if (Difficulty > 9) Difficulty = 9;
	
	return Result == KEY_ESC ? 0 : 1;
}

/* TODO: NewRecordScreen()
void NewRecordScreen()
{
	char Name[17];
	char DefaultName[17] = "No Name";
	
	HANDLE MenuHandle = DialogNewSimple(140, 38);
	
  DialogAddTitle(MenuHandle, "NEW RECORD", BT_NONE, BT_OK);
  DialogAddRequest(MenuHandle, 3, 16, "Your name", 0, 16, 14);
  
  if (DialogDo(MenuHandle, CENTER, CENTER, Name, NULL) != KEY_ENTER)
  {
  	
  }
  HeapFree(MenuHandle);
}
*/

void MemErrorExit()
{
	DlgMessage("ERROR", "Error while allocating memory.", BT_NONE, BT_OK);
	exit(0);
}

void KeyDetection()
{
	if (_keytest(RR_UP)) EngineSetDir(0);
	else if (_keytest(RR_DOWN)) EngineSetDir(2);
		
	if (_keytest(RR_LEFT)) EngineSetDir(3);
	else if (_keytest(RR_RIGHT)) EngineSetDir(1);
	
	if ((_keytest(RR_UP) || _keytest(RR_DOWN) || _keytest(RR_LEFT) || _keytest(RR_RIGHT))
		&& (Timer > DifficultyTimer[(short)Difficulty]))
		Timer = DifficultyTimer[(short)Difficulty];
}

char ProcessPause()
{	
	while (_keytest(RR_ESC));
			
	IntRst(); 
	RstRate();
	
	memset(LCD_MEM, 0, LCD_SIZE);
	
	/*FontSetSys(F_6x8);
	
	printf_xy(0, 0, "Score: %ld", EngineGetScore());
	
	FontSetSys(OrigFont);*/
	
	short PauseStat = DlgMessage("PAUSED", "Exit?", BT_NO, BT_YES);
	
	if (PauseStat == KEY_ENTER)
	{
		return 1;
	}
	
	memcpy(LCD_MEM, LCDBuffer, LCD_SIZE);
	
	if (!Resuming) Timer = 300;
	Resuming = 1;
	Redraw = 0;
	
	IntSet();
	SetSpeed(240);
	
	while (_keytest(RR_ESC));
	
	return 0;
}

void _main(void)
{	
	GetHighscore();

	if (MainMenuScreen() == 0) exit(0);
	
	//Init & memory allocating
	if ((!(TileBuffer = calloc(LCD_SIZE, 1))) || (!(LCDBuffer = calloc(LCD_SIZE, 1))) || (!EngineInit(Difficulty, 20, 12, TileBuffer))) MemErrorExit();
	//if ((!(TileBuffer = calloc(LCD_SIZE, 1))) || (!EngineInit(Difficulty, 20, 12, TileBuffer))) MemErrorExit();
	
	//Update difficulty settings
	if (DerefSym(SymFind(SYMSTR("gsnakehs")))->flags.bits.archived) 
			EM_moveSymFromExtMem(SYMSTR("gsnakehs"), HS_NULL);
			
	FILE *ScoreFile = fopen("gsnakehs", "r+b");
		
	fwrite(&Difficulty, 1, 1, ScoreFile);
	
	fclose(ScoreFile);
	
	//Start
	OrigFont = FontGetSys();
	
	IntSet();
	GetRate();
	
	memset(LCD_MEM, 0, LCD_SIZE);
	
	SetSpeed(240);
	
	Timer = DifficultyTimer[(short)Difficulty];
	
	char FirstDraw = 1;
	
	while (1)
	{
		KeyDetection();
	
		if (_keytest(RR_ESC))
			if (ProcessPause() == 1) break;
		
		if (Redraw)
		{
			if (EngineStep() != 1) break;
			EngineDraw(FirstDraw);
			FirstDraw = 0;
			
			Resuming = 0;
			Redraw = 0;
			
			memcpy(LCDBuffer, TileBuffer, LCD_SIZE - 120);
			memset(LCDBuffer + (LCD_SIZE - 120), 0, 120);
			
			char ScoreStr[30];
			sprintf(ScoreStr, "Score: %ld", EngineGetScore());
			Text3x5(0, 94, ScoreStr, LCDBuffer);
			
			memcpy(LCD_MEM, LCDBuffer, LCD_SIZE);
		}
	}
	
	while (_keytest(RR_ESC));
	
	EngineDispose();
	
	free(TileBuffer);
	free(LCDBuffer);
	
	RstRate();
	IntRst();
	
	//NewRecordScreen();
	
	//Score
	long Score = EngineGetScore();
	
	char ScoreText[48];
	sprintf(ScoreText, "Score: %ld", Score);
	
	ScoreFile = fopen("gsnakehs", "r+b");
	
	if (DerefSym(SymFind(SYMSTR("gsnakehs")))->flags.bits.archived) 
			EM_moveSymFromExtMem(SYMSTR("gsnakehs"), HS_NULL);
	
	char NewRecord = 0;
	if (Score > Highscore)
	{
		NewRecord = 1;
		Highscore = Score;
		
		fseek(ScoreFile, 1, SEEK_SET);
		fwrite(&Highscore, 4, 1, ScoreFile);
	}
	
	RandSeed = EngineRand();
	fseek(ScoreFile, 5, SEEK_SET);
	fwrite(&RandSeed, 2, 1, ScoreFile);
	
	fclose(ScoreFile);
	
	DlgMessage(NewRecord ? "NEW RECORD" : "GAME OVER", ScoreText, BT_NONE, BT_OK);
	
	exit(0);
}
