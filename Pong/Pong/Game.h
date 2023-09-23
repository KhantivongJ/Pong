#pragma once
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

struct Vector2
{
	float x;
	float y;
};

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();
private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	Uint32 mTicksCount;
	
	bool mIsRunning;

	// Paddle & Ball
	int mPaddleDir;
	Vector2 mPaddlePos;
	Vector2 mBallPos;
	Vector2 mBallVel;
	int mContact;

	// Music
	Mix_Chunk* hitSFX;
	Mix_Music* mMusic;

	// Text
	TTF_Font* theFont;
	int mWidth;
	int mHeight;
	SDL_Texture* scoreTexture;
	SDL_Texture* bestScoreTexture;
	int scoreACC;
	int hiScoreACC;
	SDL_Color textColor;
	string scoreText;
	SDL_Surface* textSurface;
	SDL_Surface* text2Surface;
	int nWidth;
	int nHeight;

	// File
	fstream pongFile;

	string bestScoreText;



};
