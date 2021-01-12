#include <stdio.h>
#include <SDL2/SDL.h>

#define WIDTH 400
#define HEIGHT 400
#define SQUARESIZE 20

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;
int squares[WIDTH/SQUARESIZE*(WIDTH/SQUARESIZE)];

struct SnakeBody {
	int x;
	int y;
};

struct Player {
	int x;
	int y;
	int x_vel;
	int y_vel;
	int parts;
};

struct Apple {
	int x;
	int y;
};


struct SnakeBody bodySegments[WIDTH/SQUARESIZE*(WIDTH/SQUARESIZE)];

int
init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return 3;
	}

	if (SDL_CreateWindowAndRenderer(WIDTH,HEIGHT,0,&window,&renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		return 3;
	}

	return 0;
}

int
main() 
{
	struct Player snake = {1,1,1,0,0}; 

	struct Apple apple = {
		rand()%(WIDTH/SQUARESIZE),
		rand()%(WIDTH/SQUARESIZE)
	};
	
	init();

	int running = 1;

	unsigned int start;

	while (running) {
		start = SDL_GetTicks();
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_s:
							snake.x_vel=0;
							snake.y_vel=1;
							break;
						case SDLK_w:
							snake.x_vel=0;
							snake.y_vel=-1;
							break;
						case SDLK_a:
							snake.x_vel=-1;
							snake.y_vel=0;
							break;
						case SDLK_d:
							snake.x_vel=1;
							snake.y_vel=0;
					}				
					break;
			}
		}

		/*update things*/
		int windowx;
		int windowy;
		SDL_GetWindowPosition(window, &windowx, &windowy);


		windowx -= snake.x_vel*SQUARESIZE;
		windowy -= snake.y_vel*SQUARESIZE;
		snake.x += snake.x_vel;
		snake.y += snake.y_vel;

		if (snake.x == apple.x && snake.y == apple.y)
		{
			/* make new apple pos */
			apple.x = rand()%(WIDTH/SQUARESIZE);
			apple.y = rand()%(WIDTH/SQUARESIZE);
			snake.parts += 1;
		}

		int prev_x = snake.x-snake.x_vel;
		int prev_y = snake.y-snake.y_vel;

		int i;

		for (i=0;i<snake.parts;i++) {
			int temp_x = bodySegments[i].x;
			int temp_y = bodySegments[i].y;
			
			bodySegments[i].x = prev_x;
			bodySegments[i].y = prev_y;

			prev_x = temp_x;
			prev_y = temp_y;
		}


		/*draw things*/
		SDL_RenderClear(renderer);
		int x,y;
		for (x=0;x<WIDTH/SQUARESIZE;x++) {
			for (y=0;y<HEIGHT/SQUARESIZE;y++) {
				int brightness=(x+y)%2==0?0x11:0x00;
				SDL_SetRenderDrawColor(renderer, brightness, brightness, brightness, 0);

				int isBody = 0;
				int i;
				for (i=0;i<snake.parts;i++) {
					if (bodySegments[i].x == x && bodySegments[i].y == y)
					{
						isBody = 1;
						break;
					}
				}

				if (x==snake.x && y==snake.y || isBody) 
				{
					/*then use snake color*/
					SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0);
				}
				else if(x==apple.x && y==apple.y)
				{
					SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0);
				}

				SDL_Rect rect = {x*SQUARESIZE,y*SQUARESIZE,SQUARESIZE,SQUARESIZE};
				SDL_RenderFillRect(renderer, &rect);
			}
		}
		SDL_SetWindowPosition(window, windowx, windowy);
		SDL_RenderPresent(renderer);

		if (running)
		{
			SDL_Delay(100-(SDL_GetTicks()-start));
		}
	}

	printf("testing!");
	return 0;

}
