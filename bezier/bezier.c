#include "bezier.h"

void die() {
	fprintf(stderr, "DISREGARD THAT, I SUCK COCKS.\n");
   exit(9001);
}

void compute_subpoint(short first[2], short second[2], short *sol, double t) {
	short delta[2];
	double psol[2];
	delta[X] = second[X] - first[X];
//	printf("delta[X] (%hd) = second[X] (%hd) - first[X] (%hd)\n", delta[X], second[X], first[X]);
	delta[Y] = second[Y] - first[Y];
	psol[X] = delta[X] * t;
//	printf("psol[X] (%f) = delta[X] (%hd) * t (%f)\n", psol[X], delta[X], t);
	psol[Y] = delta[Y] * t;
	sol[X] = first[X] + psol[X];
//	printf("sol[X] (%hd) = first[X] (%hd) + psol[X] (%f)\n",sol[X], first[X], psol[X]);
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

	while (1) {
		double t;      
		pixelColor(screen, first[X], first[Y], 0xFFFFFFFF);
		pixelColor(screen, second[X], second[Y], 0xFFFFFFFF);
		for(t = 0.0; t < 1.0; t += 0.003) {
			short sol[2];
			compute_subpoint(first, second, (short *) &sol, t);
			pixelColor(screen, sol[X], sol[Y], 0xFFFFFFFF);
		};
		SDL_UpdateRect(screen, 0, 0, 0, 0);
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