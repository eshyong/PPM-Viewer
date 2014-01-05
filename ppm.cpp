#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <string>

#define NUM_LENGTH 11

int decodePPM (FILE* stream) {
	// parse header
	char c;
	int width, height, depth;
	width = -1;
	height = -1;
	depth = -1;

	// magic number P3 or P6
	c = (char)getc(stream);
	if (c != 'P') {
		printf("file is not in ppm format.\n");
		return -1;
	}
	c = (char)getc(stream);
	if (c != '3' && c != '6') {
		printf("cannot parse non P3 and P6 files.\n");
		return -2;
	}

	char num[NUM_LENGTH];
	int val;
	int index = 0;
	memset(num, 0, sizeof(num) * sizeof(char));

	// find width, height, and max color value
	do {
		c = (char)getc(stream);
		switch (c) {
			case '#':
				// comment, ignore and skip to next line
				while ((char)getc(stream) != '\n');
				break;
			case '\n':
			case ' ':
				// no numbers found
				if (index == 0) break;

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
					memset(num, 0, sizeof(num) * sizeof(char));
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
	} while (width == -1 || height == -1 || depth == -1);
	printf("width: %d, height: %d, depth: %d\n", width, height, depth);

	return 0;
}

int decodePPMA (FILE* stream) {
	return 0;
}

int main(void) {
	FILE* stream = fopen("feep.ppm", "rb");
	if (stream == NULL) {
		printf("Error opening file.");
		exit(1);
	}
	decodePPM(stream);

	fclose(stream);
	return 0;
}