#include "bezier.h"

void die() {
	fprintf(stderr, "DISREGARD THAT, I SUCK COCKS.\n");
   exit(9001);
}

void compute_subpoint(short first[2], short second[2], short *sol, float t) {
	short delta[2], psol[2];
	delta[X] = first[X] - second[X];
	delta[Y] = first[Y] - second[Y];
	psol[X] = delta[X] * t;
	psol[Y] = delta[Y] * t;
	sol[X] = first[X] + psol[X];
	sol[Y] = first[Y] + psol[Y];
}

int main() {
	short first[2] = { 100, 100 };
	short second[2] = { 150, 150 };
	if ((SDL_Init(SDL_INIT_VIDEO) > 0)) {
		die();
	}
	atexit(SDL_Quit);
	if (!(screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE))) {
		die();	
	}

	while (1) {
      pixelColor(screen, 100, 100, 0xFFFFFFFF);
		pixelColor(screen, 150, 150, 0xFFFFFFFF);
		for(float t = 0; t < 1; t += 0.001) {
			short sol[2];
			compute_subpoint(first, second, &sol, t);
			pixelColor(screen, sol[X], sol[Y], 0xFFFFFFFF);
		}
		//render();
		
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					return 0;
				}
				break;
			case SDL_QUIT:
				return(0);
			}
		}
	}
}