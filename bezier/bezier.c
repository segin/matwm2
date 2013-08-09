#include "bezier.h"

void die() {
	fprintf(stderr, "DISREGARD THAT, I SUCK COCKS.\n");
   exit(9001);
}

void render_subpoint(short x1, short y1, short x2, short y2, float t) {

}

int main() {
	SDL_Surface *screen;
	if ((SDL_Init(SDL_INIT_VIDEO) > 0)) {
		die();
	}
	atexit(SDL_Quit);
	if (!(screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE))) {
		die();	
	}

	while (1) {
		render();
		
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