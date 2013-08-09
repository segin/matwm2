#include "bezier.h"

void die() {
	fprintf(stderr, "DISREGARD THAT, I SUCK COCKS.\n");
   exit(9001);
}

void compute_subpoint(short first[2], short second[2], short *sol, double t) {
	short delta[2], psol[2];
	delta[X] = first[X] - second[X];
	delta[Y] = first[Y] - second[Y];
	psol[X] = delta[X] * t;
	psol[Y] = delta[Y] * t;
	sol[X] = first[X] + psol[X];
	sol[Y] = first[Y] + psol[Y];
}

int main() {
	short first[2] = { 150, 150 };
	short second[2] = { 100, 100 };
	if ((SDL_Init(SDL_INIT_VIDEO) > 0)) {
		die();
	}
	atexit(SDL_Quit);
	if (!(screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE))) {
		die();	
	}

	double t;      
	pixelColor(screen, first[X], first[Y], 0xFFFFFFFF);
	pixelColor(screen, second[X], second[Y], 0xFFFFFFFF);
	for(t = 0; t < 1; t += 0.003) {
		short sol[2];
		compute_subpoint(first, second, (short *) &sol, t);
		pixelColor(screen, sol[X], sol[Y], 0xFFFFFFFF);
	}

	while (1) {
		
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