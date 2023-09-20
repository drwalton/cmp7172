#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_FontCache.h>
#include <chrono>
#include <thread>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

/* Start of a Space Invaders clone implementation, using SDL for input handling & rendering.
 * Currently has rendering logic, invader movement and player movement.
 * 
 * Tasks:
 * 1. Implement allowing the player to shoot the missile with the SPACE key. Reset the missile if it reaches
 *    the top of the screen, and allow the player to shoot again. You might want to use the keydown event rather than
 *    checking the key state.
 * 2. Implement collision between the missile and the invaders (SDL_HasIntersection() should come in handy).
 *    This should remove the invader if hit and increment the score by 10 points.
 * 3. Reimplement with mouse-based input (the horizontal position of the launcher should track the mouse's x location)
 *    Use left click to shoot a missile.
 * 4. Reimplement with controller-based input, using analog stick input to move the launcher. Make sure to add a "dead zone"
 *    so the launcher doesn't drift if the stick is released.
 */

const int clearColor[] = { 50, 50, 50 };

const int winWidth = 1024;
const int winHeight = 768;

// Invader properties
const int 
	invaderNRows = 4, invaderNCols = 5, 
	invaderSize = 50, invaderSeparation = 20, 
	invaderInitX = 50, invaderInitY = 50,
	invaderFrameStep = 30, invaderStepSize = 25,
	invaderHorzSteps = 
		(winWidth - 2 * invaderInitX - invaderNCols * (invaderSize + invaderSeparation)) / invaderStepSize;

int invaderDir = 1;

// Launcher properties
const int launcherSize = 50,
	launcherY = winHeight - 50 - launcherSize,
	launcherMinX = 50, launcherMaxX = winWidth - launcherMinX - launcherSize,
	launcherMoveSpeed = 4;

// Missile properties
const int missileSize = 50;

// Determines whether to draw the missile. You should probably change this to false and 
// change it according to user input.
bool missileActive = true;

/// <summary>
/// Loads a texture from an image file using OpenCV into an SDL texture. 
/// </summary>
/// <param name="renderer">The SDL_Renderer to create the texture for.</param>
/// <param name="path">Path to the image file.</param>
/// <param name="loadAlpha">Determines whether the texture will be created with an alpha channel.
/// If true, your image file should also have transparency (and be of appropriate format e.g. PNG not JPG)</param>
/// <returns>An SDL_Texture pointer to the created texture. Remember to free the texture later.</returns>
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path, bool loadAlpha = false)
{
	cv::Mat image;
	if (loadAlpha) {
		image = cv::imread(path, cv::IMREAD_UNCHANGED);
	}
	else {
		image = cv::imread(path);
	}
	SDL_PixelFormatEnum pixelFormat;
	if (loadAlpha) {
		pixelFormat = SDL_PIXELFORMAT_ABGR8888;
	}
	else {
		pixelFormat = SDL_PIXELFORMAT_BGR888;
	}
	SDL_Texture* texture = SDL_CreateTexture(
		renderer,
		pixelFormat,
		SDL_TEXTUREACCESS_STATIC,
		image.cols, image.rows);

	// Update the texture with the image data.
	// This update function is slow - fine here as we load textures once at the start and don't change them.
	// If you want to update textures every frame, use SDL_LockTexture/SDL_UnlockTexture.
	SDL_UpdateTexture(texture,
		nullptr,
		image.data,
		image.step
		);

	if (loadAlpha) {
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	}
	return texture;
}

/// <summary>
/// Updates the locations of the invaders, marching them down the screen in lines.
/// </summary>
/// <param name="invaderRects">Vector of the current invader rects.</param>
/// <param name="currFrameIdx">Index of the current frame.</param>
void updateInvaders(std::vector<SDL_Rect>& invaderRects, Uint64 currFrameIdx) {
	if (currFrameIdx % invaderFrameStep == invaderFrameStep-1) {
		Uint64 stepIdx = currFrameIdx / invaderFrameStep;
		if (stepIdx % (invaderHorzSteps + 1) == invaderHorzSteps) {
			// Take a vertical step
			for (auto& rect : invaderRects) {
				rect.y += invaderStepSize;
			}
			invaderDir = -invaderDir;
		}
		else {
			// Take a horizontal step in the current direction
			for (auto& rect : invaderRects) {
				rect.x += invaderDir * invaderStepSize;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

	// Prepare window
	SDL_Window* window = SDL_CreateWindow("A test window", 50, 50, winWidth, winHeight, 0);

	// Prepare renderer. Uses default settings, should create an accelerated renderer if possible.
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	// Load font to draw the score.
	FC_Font* font = FC_CreateFont();
	FC_LoadFont(font, renderer, "../font/opensans.ttf", 24, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);

	// Load the necessary textures.
	SDL_Texture* bgTexture = loadTexture(renderer, "../images/background.png");
	SDL_Texture* invaderTexture = loadTexture(renderer, "../images/Invader.png", true);
	SDL_Texture* missileTexture = loadTexture(renderer, "../images/Missile.png", true);
	SDL_Texture* launcherTexture = loadTexture(renderer, "../images/RocketLauncher.png", true);

	// Where to draw the bg texture. Note since I initialised with {} the x and y are set to 0.
	SDL_Rect bgTextureRect{}; bgTextureRect.w = winWidth; bgTextureRect.h = winHeight;

	// Create the invaders at their starting locations (in a regular grid).
	std::vector<SDL_Rect> invaderRects(invaderNRows * invaderNCols);
	for (int r = 0; r < invaderNRows; ++r) {
		for (int c = 0; c < invaderNCols; ++c) {
			int idx = r * invaderNCols + c;
			invaderRects[idx].w = invaderSize;
			invaderRects[idx].h = invaderSize;
			invaderRects[idx].x = invaderInitX + c*(invaderSize + invaderSeparation);
			invaderRects[idx].y = invaderInitY + r*(invaderSize + invaderSeparation);
		}
	}

	SDL_Rect launcherRect; launcherRect.w = launcherSize, launcherRect.h = launcherSize, 
		launcherRect.x = launcherMinX, launcherRect.y = launcherY;

	SDL_Rect missileRect; missileRect.w = missileSize, missileRect.h = missileSize,
		missileRect.x = winWidth / 2, missileRect.y = winHeight / 2;

	bool shouldQuit = false;
	SDL_Event event;

	const Uint64 targetFrameTime = 16; // Set target frame time to 16ms (1000ms/16ms is about 60fps)
	Uint64 currFrameIdx = 0;

	int score = 0;

	while (!shouldQuit) {
		Uint64 frameStartTime = SDL_GetTicks64();

		while (SDL_PollEvent(&event)) {
			// Check for X of window being clicked, or ALT+F4
			if (event.type == SDL_QUIT) {
				shouldQuit = true;
			}
		}

		// Handle player movement. Uses GetKeyboardState rather than events, 
		// as we want the launcher to move continuously every frame as long as 
		// L or R is held down.
		const Uint8* keyState = SDL_GetKeyboardState(nullptr);
		if (keyState[SDL_SCANCODE_LEFT]) {
			launcherRect.x -= launcherMoveSpeed;
			launcherRect.x = std::max(launcherRect.x, launcherMinX);
		}
		if (keyState[SDL_SCANCODE_RIGHT]) {
			launcherRect.x += launcherMoveSpeed;
			launcherRect.x = std::min(launcherRect.x, launcherMaxX);
		}

		updateInvaders(invaderRects, currFrameIdx);

		// *** Rendering ***

		// Clear the screen.
		SDL_SetRenderDrawColor(renderer, clearColor[0], clearColor[1], clearColor[2], 255);
		SDL_RenderClear(renderer);

		// Draw the starry background.
		SDL_RenderCopy(renderer, bgTexture, nullptr, &bgTextureRect);

		// Draw the game objects.
		for (SDL_Rect& invaderRect : invaderRects) {
			SDL_RenderCopy(renderer, invaderTexture, nullptr, &invaderRect);
		}
		SDL_RenderCopy(renderer, launcherTexture, nullptr, &launcherRect);

		if (missileActive) {
			SDL_RenderCopy(renderer, missileTexture, nullptr, &missileRect);
		}

		// Draw the score.
		FC_Draw(font, renderer, 10.f, 10.f, "Score: %d", score);

		// Show result.
		SDL_RenderPresent(renderer);

		// Limit the framerate - if we've reached this point before
		// the desired frame time has passed, wait with SDL_Delay.
		Uint64 frameDuration = SDL_GetTicks64() - frameStartTime;
		if (frameDuration < targetFrameTime) {
			SDL_Delay(targetFrameTime - frameDuration);
		}

		++currFrameIdx;
	}

	// Cleanup.
	FC_FreeFont(font);
	SDL_DestroyTexture(bgTexture);
	SDL_DestroyTexture(invaderTexture);
	SDL_DestroyTexture(missileTexture);
	SDL_DestroyTexture(launcherTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

