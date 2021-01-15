#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>

#define WIDTH 96
#define HEIGHT 54
#define SQUARESIZE 1
#define SCALE 10

#define GIMMICK 0
#define AI 1

#define FRAMETIME 25

int moveTick = 0;
int fullscreen = 0;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;
int squares[WIDTH / SQUARESIZE * (HEIGHT / SQUARESIZE)];

int windowx;
int windowy;

struct Vec2 {
	int x, y;
};

struct Player {
	struct Vec2 position;
	struct Vec2 velocity;

	int bodyAmount;
};

struct Vec2 bodySegments[WIDTH / SQUARESIZE * (HEIGHT / SQUARESIZE)];

struct Vec2 possibleMoves[] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

struct Player snake = {{1, 1}, {1, 0}, 0};

struct Vec2 apple = {(WIDTH / SQUARESIZE) / 2, (HEIGHT / SQUARESIZE) / 2};

int
init()
{
	srand(time(NULL));
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Couldn't initialize SDL: %s",
			SDL_GetError());
		return 3;
	}

	if(SDL_CreateWindowAndRenderer(
			 WIDTH * SCALE, HEIGHT * SCALE, 0, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Couldn't create window and renderer: %s",
			SDL_GetError());
		return 3;
	}

	return 0;
}

int
existsBody(int x, int y)
{
	int hittingBody = 0;
	int i;
	for(i = 0; i < snake.bodyAmount; i++) {
		if(bodySegments[i].x == x && bodySegments[i].y == y) {
			hittingBody = 1;
			break;
		}
	}
	return hittingBody;
}

int
AI_update()
{
	int x_dir = 0, y_dir = 0;

	if(apple.y > snake.position.y) {
		y_dir = 1;
	} else if(apple.y < snake.position.y) {
		y_dir = -1;
	}

	if(apple.x > snake.position.x) {
		x_dir = 1;
	} else if(apple.x < snake.position.x) {
		x_dir = -1;
	}

	if(x_dir && y_dir) {
		if(rand() % 2 == 1) {
			y_dir = 0;
		} else {
			x_dir = 0;
		}
	}

	if(x_dir) {
		snake.velocity.x = x_dir;
		snake.velocity.y = 0;
	} else if(y_dir) {
		snake.velocity.x = 0;
		snake.velocity.y = y_dir;
	}

	/*if about to do illegal move, make legal move*/
	int hittingBody = existsBody(
		snake.position.x + snake.velocity.x, snake.position.y + snake.velocity.y);

	if(hittingBody) {
		/*lets find non losing moves!*/
		struct Vec2 legalMoves[4];
		int legalMoveAmount = 0;

		int i, j;
		for(i = 0; i < 4; i++) {
			/*check if possible move is legal*/
			int hittingBody = existsBody(snake.position.x + possibleMoves[i].x,
				snake.position.y + possibleMoves[i].y);

			if(!hittingBody && snake.position.x + possibleMoves[i].x >= 0 &&
				 snake.position.y + possibleMoves[i].y >= 0 &&
				 snake.position.x + possibleMoves[i].x < WIDTH / SQUARESIZE &&
				 snake.position.y + possibleMoves[i].y < HEIGHT / SQUARESIZE) {
				legalMoves[legalMoveAmount] = possibleMoves[i];
				legalMoveAmount++;
			}
		}

		if(legalMoveAmount) {
			int choice = rand() % legalMoveAmount;
			snake.velocity = legalMoves[choice];
		}
	}
}

int
update()
{
	moveTick += 1;
	/*update things*/
	SDL_GetWindowPosition(window, &windowx, &windowy);

	if(GIMMICK) {
		windowx -= snake.velocity.x * SQUARESIZE;
		windowy -= snake.velocity.y * SQUARESIZE;
	}
	snake.position.x += snake.velocity.x;
	snake.position.y += snake.velocity.y;

	if(snake.position.x == apple.x && snake.position.y == apple.y) {
		/* make new apple pos */
		apple.x = rand() % (WIDTH / SQUARESIZE);
		apple.y = rand() % (HEIGHT / SQUARESIZE);
		snake.bodyAmount += 1;
	}

	int prev_x = snake.position.x - snake.velocity.x;
	int prev_y = snake.position.y - snake.velocity.y;

	int i;

	for(i = 0; i < snake.bodyAmount; i++) {
		int temp_x = bodySegments[i].x;
		int temp_y = bodySegments[i].y;

		bodySegments[i].x = prev_x;
		bodySegments[i].y = prev_y;

		prev_x = temp_x;
		prev_y = temp_y;
	}

	if(AI) {
		AI_update();
	}

	if(existsBody(snake.position.x, snake.position.y)) {
		snake.bodyAmount = 0;
		printf("died!\n");
	}
}

int
draw()
{
	SDL_RenderClear(renderer);
	int x, y;
	for(x = 0; x < WIDTH / SQUARESIZE; x++) {
		for(y = 0; y < HEIGHT / SQUARESIZE; y++) {
			int brightness = (x + y) % 2 == 0 ? 0x11 : 0x00;
			SDL_SetRenderDrawColor(renderer, brightness, brightness, brightness, 0);

			int isBody = 0;
			int i;
			for(i = 0; i < snake.bodyAmount; i++) {
				if(bodySegments[i].x == x && bodySegments[i].y == y) {
					isBody = 1;
					break;
				}
			}

			if((x == snake.position.x && y == snake.position.y) || isBody) {
				/*then use snake color*/
				SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0);
			} else if(x == apple.x && y == apple.y) {
				SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0);
			}

			SDL_Rect rect = {x * SQUARESIZE, y * SQUARESIZE, SQUARESIZE, SQUARESIZE};
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

int
main()
{
	init();

	int running = 1;
	unsigned int start;

	while(running) {
		start = SDL_GetTicks();
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_QUIT: running = 0; break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_s:
					snake.velocity.x = 0;
					snake.velocity.y = 1;
					break;
				case SDLK_w:
					snake.velocity.x = 0;
					snake.velocity.y = -1;
					break;
				case SDLK_a:
					snake.velocity.x = -1;
					snake.velocity.y = 0;
					break;
				case SDLK_d:
					snake.velocity.x = 1;
					snake.velocity.y = 0;
					break;
				case SDLK_f:
					fullscreen = fullscreen ? 0 : 1;
					SDL_SetWindowFullscreen(
						window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
					break;
				case SDLK_m:
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
						"Hello world!",
						"This is interesting huh",
						NULL);
					break;
				}
			}
		}

		update();

		draw();

		int renderW, renderH;
		SDL_GetRendererOutputSize(renderer, &renderW, &renderH);

		/*
		SDL_SetWindowSize(window, renderW, renderH);
		*/

		SDL_RenderSetScale(
			renderer, (float)renderW / (float)WIDTH, (float)renderH / (float)HEIGHT);

		SDL_Rect rect = {0, 0, WIDTH, HEIGHT};
		SDL_RenderSetViewport(renderer, &rect);

		if(GIMMICK) {
			SDL_SetWindowPosition(window, windowx, windowy);
		}
		SDL_RenderPresent(renderer);

		if(running) {
			int time = SDL_GetTicks() - start;

			int waitTime = FRAMETIME - time;
			if(waitTime < 19 || waitTime > 30)
				printf("wait time: %d\n", waitTime);
			if(waitTime > 0)
				SDL_Delay(waitTime);
		}
	}

	return 0;
}
