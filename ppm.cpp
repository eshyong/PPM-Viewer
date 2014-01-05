// standard headers
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <string>

// graphics headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// constants
#define NUM_LENGTH 11
#define X_POS      100
#define Y_POS      100

unsigned int* decodePPM (FILE* stream, int* w, int* h, int* d) {
	/* parse ASCII header */

	// c is used for P3, ch for P6
	char c;
	int ch;

	// width, height, and pixel depth of image
	int width, height, depth;
	width = -1;
	height = -1;
	depth = -1;

	// true for P3, false for P6
	bool ascii;

	// magic number P3 or P6
	c = (char)getc(stream);
	if (c != 'P') {
		printf("file is not in ppm format.\n");
		return NULL;
	}
	c = (char)getc(stream);
	if (c == '3') {
		ascii = true;
	} else if (c == '6') {
		ascii = false;
	} else {
		printf("greyscale formats not supported.\n");
		return NULL;
	}

	// num buffer for numbers in ASCII form
	char num[NUM_LENGTH];
	int val;
	int index = 0;
	memset(num, 0, sizeof(num));

	// find width, height, and max color value
	do {
		c = (char)getc(stream);
		switch (c) {
			case '#':
				// found a comment, ignore and skip to next line
				while ((char)getc(stream) != '\n');
				break;
			case '\n':
			case ' ':
				// no numbers found
				if (index == 0) break;
				else if (index >= NUM_LENGTH) {
					// should never happen
					printf("found an impossibly large number.\n");
					return NULL;
				}

				// end of a number
				val = atoi(num);
				if (val != 0) {
					// set any unset fields
					if (width == -1) 
						width = val;
					else if (height == -1) 
						height = val;
					else 
						depth = val;
					// reset num
					memset(num, 0, sizeof(num));
					index = 0;
				}
				break;
			default:
				// add digit to number
				if (isdigit(c)) {
					num[index] = c;
					index++;
				}
				break;
		}
	} while ((width == -1 || height == -1 || depth == -1) && !feof(stream));

	/* debug */
	printf("width: %d, height: %d, depth: %d\n", width, height, depth);

	// somehow we've reached the end of the file without seeing any values
	if (width == -1 || height == -1 || depth == -1) {
		printf("error parsing width/height/depth.\n");
		return NULL;
	}
	*w = width;
	*h = height;
	*d = depth;

	// allocate new pixmap
	unsigned int* pixmap = new unsigned int[width * height];
	memset(pixmap, 0, width * height * sizeof(unsigned int));
	int r, g, b;
	r = -1;
	g = -1;
	b = -1;
	// index for width and height
	int i = 0, j = height - 1;

	// ppm has to be decoded from bottom to top (OpenGL idiosyncrasy)
	do {
		if (ascii) {
			// P3 files (ASCII data)
			c = (char)getc(stream);
			if (isdigit(c)) {
				num[index] = c;
				index++;
			} else {
				if (index != 0) {
					if (index >= NUM_LENGTH) {
						// should never happen
						printf("found an impossibly large number.\n");
						delete[] pixmap;
						return NULL;
					}
					val = atoi(num);
					if (val > depth) {
						// also should never happen
						printf("invalid ppm file: found a value greater than prescribed depth.\n");
						delete[] pixmap;
						return NULL;
					}
					if (r == -1) 
						r = val;
					else if (g == -1)
						g = val;
					else if (b == -1) {
						// all values are now found
						// alpha is always 255 since ppms only encode RGB
						b = val;

						// little endian: must pack numbers in ABGR format
						pixmap[width * j + i] = (((255 << 24) | (b << 16)) | (g << 8)) | r;

						// decrement pidx and reset values
						if (i < (width - 1)) {
							i++;
						} else {
							j--;
							i = 0;
						}
						r = -1;
						g = -1;
						b = -1;
					}
					memset(num, 0, sizeof(num) * sizeof(char));
					index = 0;
				}
			}
		} else {
			/* P6 files (binary data) */

			// same as P3, only with binary data (no need to use a string)
			ch = getc(stream);
			if (ch > depth) {
				printf("invalid ppm file: found a value greater than prescribed depth.\n");
				delete[] pixmap;
				return NULL;
			}
			if (r == -1) 
				r = ch;
			else if (g == -1)
				g = ch;
			else if (b == -1) {
				// all values are now found
				// alpha is always 255 since ppms only encode RGB
				b = ch;
				pixmap[width * j + i] = (((255 << 24) | (b << 16)) | (g << 8)) | r;

				if (i < (width - 1)) {
					i++;
				} else {
					j--;
					i = 0;
				}
				r = -1;
				g = -1;
				b = -1;
			}
		}
	} while (j >= 0 && !feof(stream));

	// check index of pixmap
	if (j != -1) {
		printf("error parsing rgb values.\n");
		delete[] pixmap;
		return NULL;
	}
	return pixmap;
}

int main(int argc, char* argv[]) {
	/* open ppm file and decode contents */

	// must have a filename argument
	if (argc < 2) {
		printf("Usage: ppm filename.ppm\n");
		printf("add files to \"pics\" folder\n");
		return 0;
	}

	// get path to file
	std::string s = "pics";
	s.append("/");
	s.append(argv[1]);

	// open file stream in binary mode
	FILE* stream = fopen(s.c_str(), "rb");
	if (stream == NULL) {
		printf("Error opening file.\n");
		exit(1);
	}

	// pixel map containing data from ppm
	int width, height, depth;
	unsigned int* pixmap = decodePPM(stream, &width, &height, &depth);
	if (pixmap == NULL) {
		printf("aborting...\n");
		return 1;
	}
	
	/* debug */
	// int length = width * height;
	// for (int i = 0; i < length; i++) {
	// 	printf("0x%x ", pixmap[i]);
	// }
	// printf("\n");

	// clean up file stream
	fclose(stream);

	/* draw image */

	// create an SDL context
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("OpenGL", X_POS, Y_POS, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	SDL_Event event;
	
	// event loop
	while (true) {
		if (SDL_PollEvent(&event)) {
			// break if esc key is pressed or window is closed
			if (event.type == SDL_QUIT) 
				break;
			if (
				event.type == SDL_KEYUP && 
				event.key.keysym.sym == SDLK_ESCAPE
			)
    			break;
		}

		// draw calls here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glRasterPos2i(-1.0f, -1.0f);
		glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixmap);
		SDL_GL_SwapWindow(window);
	}

	// clean up SDL context
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}