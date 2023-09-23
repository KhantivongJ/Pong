
#include "Game.h"

const int thickness = 15;
const float paddleH = 100.0f;

Game::Game()
{
	mWindow=nullptr;
	mRenderer=nullptr;
	mTicksCount=0;
	mIsRunning=true;
	mPaddleDir=0;
	hitSFX = NULL;
	mMusic = NULL;
	theFont = NULL;
	scoreTexture = NULL;
	textSurface = NULL;
}

bool Game::Initialize()
{
	// Initialize SDL
	int sdlResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	
	// Create an SDL Window
	mWindow = SDL_CreateWindow(
		"Pong", // Window title
		100,	// Top left x-coordinate of window
		100,	// Top left y-coordinate of window
		1024,	// Width of window
		768,	// Height of window
		0		// Flags (0 for no flags set)
	);

	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}
	
	//// Create SDL renderer
	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for
		-1,		 // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}
	// Ball and Paddle
	mPaddlePos.x = 10.0f;
	mPaddlePos.y = 768.0f/2.0f;
	mBallPos.x = 1024.0f/2.0f;
	mBallPos.y = 768.0f/2.0f;
	mBallVel.x = -300.0f;
	mBallVel.y = 335.0f;
	mContact = 1;
	
	// Music
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	hitSFX = Mix_LoadWAV("hitSFX.wav");
	mMusic = Mix_LoadMUS("bgm.wav");
	Mix_PlayMusic(mMusic, -1);

	// Font
	TTF_Init();
	scoreACC = 0;
	theFont = TTF_OpenFont("font.ttf", 28);
	textColor = { 0, 0, 0 };
	
	// Open file for reading
	pongFile.open("hiScore.txt", fstream::in);
	if (pongFile.is_open())
	{
		pongFile >> hiScoreACC;
	}
	pongFile.close();
	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			// If we get an SDL_QUIT event, end loop
			case SDL_QUIT:
				mIsRunning = false;
				break;
		}
	}
	
	// Get state of keyboard
	const Uint8* state = SDL_GetKeyboardState(NULL);
	// If escape is pressed, also end loop
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
	
	// Update paddle direction based on W/S keys
	mPaddleDir = 0;
	if (state[SDL_SCANCODE_W])
	{
		mPaddleDir -= 1.8;
	}
	if (state[SDL_SCANCODE_S])
	{
		mPaddleDir += 1.8;
	}
}


void Game::UpdateGame()
{
	// Wait until 16ms has elapsed since last frame
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	// Delta time is the difference in ticks from last frame
	// (converted to seconds)
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	
	// Clamp maximum delta time value
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	// Update tick counts (for next frame)
	mTicksCount = SDL_GetTicks();
	
	// Update paddle position based on direction
	if (mPaddleDir != 0)
	{
		mPaddlePos.y += mPaddleDir * 300.0f * deltaTime;
		// Make sure paddle doesn't move off screen!
		if (mPaddlePos.y < (paddleH/2.0f + thickness))
		{
			mPaddlePos.y = paddleH/2.0f + thickness;
		}
		else if (mPaddlePos.y > (768.0f - paddleH/2.0f - thickness))
		{
			mPaddlePos.y = 768.0f - paddleH/2.0f - thickness;
		}
	}
	
	// Score Updater
	bestScoreText = "Best Score: " + to_string(hiScoreACC);
	text2Surface = TTF_RenderText_Solid(theFont, bestScoreText.c_str(), textColor);
	bestScoreTexture = SDL_CreateTextureFromSurface(mRenderer, text2Surface);

	nWidth = text2Surface->w;
	nHeight = text2Surface->h;

	scoreText = "Score: " + to_string(scoreACC);
	textSurface = TTF_RenderText_Solid(theFont, scoreText.c_str(), textColor);
	scoreTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);

	mWidth = textSurface->w;
	mHeight = textSurface->h;

	// Update ball position based on ball velocity
	mBallPos.x += mBallVel.x * deltaTime;
	mBallPos.y += mBallVel.y * deltaTime;
	
	// Bounce if needed
	// Did we intersect with the paddle?
	float diff = mPaddlePos.y - mBallPos.y;
	// Take absolute value of difference
	diff = (diff > 0.0f) ? diff : -diff;
	if (
		// Our y-difference is small enough
		diff <= paddleH / 2.0f &&
		// We are in the correct x-position
		mBallPos.x <= 25.0f && mBallPos.x >= 20.0f &&
		// The ball is moving to the left
		mBallVel.x < 0.0f)
	{
		Mix_PlayChannel(-1, hitSFX, 0);
		mBallVel.x *= -1.0f;
		mContact *= -1;
		scoreACC++;
	}
	// Did the ball go off the screen? (if so, end game)
	else if (mBallPos.x <= 0.0f)
	{
		mIsRunning = false;
	}
	// Did the ball collide with the right wall?
	else if (mBallPos.x >= (1024.0f - thickness) && mBallVel.x > 0.0f)
	{
		mBallVel.x *= -1.0f;
	}
	
	// Did the ball collide with the top wall?
	if (mBallPos.y <= thickness && mBallVel.y < 0.0f)
	{
		mBallVel.y *= -1;
	}
	// Did the ball collide with the bottom wall?
	else if (mBallPos.y >= (768 - thickness) &&
		mBallVel.y > 0.0f)
	{
		mBallVel.y *= -1;
	}
}

void Game::GenerateOutput()
{
	// Set draw color to blue
	if (mContact == 1)
		SDL_SetRenderDrawColor(mRenderer, 50, 255, 0, 255);
	else if (mContact == -1)
		SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
	
	// Clear back buffer
	SDL_RenderClear(mRenderer);

	// Draw walls
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 0, 255);
	
	// Draw top wall
	SDL_Rect wall{
		0,			// Top left x
		0,			// Top left y
		1024,		// Width
		thickness	// Height
	};
	SDL_RenderFillRect(mRenderer, &wall);
	
	// Draw bottom wall
	wall.y = 768 - thickness;
	SDL_RenderFillRect(mRenderer, &wall);
	
	// Draw right wall
	wall.x = 1024 - thickness;
	wall.y = 0;
	wall.w = thickness;
	wall.h = 1024;
	SDL_RenderFillRect(mRenderer, &wall);
	
	// Draw paddle
	SDL_Rect paddle{
		static_cast<int>(mPaddlePos.x),
		static_cast<int>(mPaddlePos.y - paddleH/2),
		thickness,
		static_cast<int>(paddleH)
	};
	SDL_RenderFillRect(mRenderer, &paddle);
	
	// Draw ball
	SDL_Rect ball{	
		static_cast<int>(mBallPos.x - thickness/2),
		static_cast<int>(mBallPos.y - thickness/2),
		thickness,
		thickness
	};
	SDL_RenderFillRect(mRenderer, &ball);

	// Score
	int x = (980 - (mWidth + thickness));
	int y = (mHeight);
	SDL_Rect* clip = NULL;

	int a = (980 - (nWidth + thickness));
	int b = nHeight;


	SDL_Rect renderQuad = { x - 15, y, mWidth, mHeight };
	SDL_Rect renderQuad2 = { a - 15, b - 30, nWidth, nHeight};

	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
		renderQuad2.w = clip->w;
		renderQuad2.h = clip->h;
	}
	
	SDL_RenderCopyEx(mRenderer, bestScoreTexture, clip, &renderQuad2, NULL, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mRenderer, scoreTexture, clip, &renderQuad, NULL, NULL, SDL_FLIP_NONE);
	
	// Swap front buffer and back buffer
	SDL_RenderPresent(mRenderer);
}

void Game::Shutdown()
{
	if (scoreACC > hiScoreACC)
	{
		pongFile.open("hiScore.txt", fstream::trunc | fstream::out);
		pongFile << scoreACC;
	}
	pongFile.close();

	SDL_FreeSurface(text2Surface);
	SDL_FreeSurface(textSurface);
	Mix_FreeChunk(hitSFX);
	Mix_FreeMusic(mMusic);
	SDL_DestroyTexture(scoreTexture);
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}