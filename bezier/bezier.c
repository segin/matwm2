#include "bezier.h"

void die() {
	fprintf(stderr, "DISREGARD THAT, I SUCK COCKS.\n");
   exit(9001);
}

void compute_subpoint(double first[2], double second[2], short *sol, double t) {
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

void compute_bezier_point(short order, double ipoints[][2], short *sol, double t) {
	double int ***npoints;
	int i, j;
	npoints = malloc(sizeof(double *) * (order + 1));
	for (i = 0; i < (order + 1); i++) {
		npoints[i] = malloc(sizeof(double *) * ((order + 1) - i));
		for(j = 0; j < (order + 1) - i; j++) { 
			npoints[i][j] = malloc(sizeof(double) * 2);
		};
	};
	for (i = 0; i < order + 1; i++) {
		npoints[0][i][X] = ipoints[i][X];
		npoints[0][i][Y] = ipoints[i][Y];
	};
	for (i = 0; i < order; i++) {
		for(j = 0; j < order - i; j++) { 
			compute_subpoint(npoints[i][j], npoints[i][j+1], npoints[i+1][j], t);
		};
	};
	sol[X] = npoints[order][0][X];
	sol[Y] = npoints[order][0][Y];
	for (i = 0; i < (order + 1); i++) {
		for(j = 0; j < (order + 1) - i; j++) { 
			free(npoints[i][j]);
		};
		free(npoints[i]);
	};
	free((void *) npoints);
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

	srand(time(NULL));

	while (1) {
		double t;
		/*
		char r = rand() % 256, g = rand() % 256, b = rand() % 256;  
		pixelColor(screen, first[X], first[Y], 0xFFFFFFFF);
		pixelColor(screen, second[X], second[Y], 0xFFFFFFFF);

		for(t = 0.0; t < 1.0; t += 0.003) {
			short sol[2];
			compute_subpoint(first, second, (short *) &sol, t);
			pixelRGBA(screen, sol[X], sol[Y], r, g, b, 0xFF);
		};
		first[X] = rand() % 640;
		first[Y] = rand() % 480;
		second[X] = rand() % 640;
		second[Y] = rand() % 480; 
		*/		
		double points[6][2] = { { 12, 366 }, { 124, 13 }, { 237, 277 }, { 348, 102 }, {460, 189 }, { 348, 277 } };		
		for(t = 0.0; t < 1.0; t += 0.003) {
			short sol[2];
			compute_bezier_point(5, points, sol, t);
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