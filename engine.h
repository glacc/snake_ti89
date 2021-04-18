// Glacc Snake Game Engine Code Header File

void EngineDispose();
char EngineInit(char InitDifficulty, char InitWidth, char InitHeight, char *InitBuffer);
char EngineStep();
void EngineDraw(char Entire);

void EngineSetDir(char Dir);

long EngineGetScore();

short EngineRand();
void EngineSetSeed(short NewSeed);