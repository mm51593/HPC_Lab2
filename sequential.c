#include <stdio.h>
#include <stdlib.h>

#define WIDTH 3840
#define HEIGHT 2160
#define CHANNELS 3
#define FRAMES 60

#define r 0
#define g 1
#define b 2

#define y 0
#define u 1
#define v 2

#define TOTAL_DATA WIDTH * HEIGHT * CHANNELS * FRAMES

void convert_rgb_to_yuv(char *read, char *write, unsigned long frames, unsigned long frame_size)
{
	printf("Converting RGB to YUV...\n");
	char R, G, B;
	for (unsigned long i = 0; i < frames; i++)
	{
		for (unsigned long j = 0; j < frame_size; j++)
		{
			R = *(read + i * (frame_size * CHANNELS) + r * (frame_size) + j);
			G = *(read + i * (frame_size * CHANNELS) + g * (frame_size) + j);
			B = *(read + i * (frame_size * CHANNELS) + b * (frame_size) + j);

			*(write + i * (frame_size * CHANNELS) + y * (frame_size) + j)  =
			       	(char) (0.257 * R + 0.504 * G + 0.098 * B + 16);
			*(write + i * (frame_size * CHANNELS) + u * (frame_size) + j)  =
				(char) (-0.148 * R - 0.291 * G + 0.439 * B + 128);
			*(write + i * (frame_size * CHANNELS) + v * (frame_size) + j)  =
				(char) (0.439 * R - 0.368 * G - 0.071 * B + 128);
		}	
	}
}

void convert_444p_to_420p(char *read, char *write, unsigned long frames, unsigned long frame_size, unsigned long frame_height)
{
	printf("Converting 444p to 420p...\n");
	unsigned long quadrant_counter;
	for (unsigned long i = 0; i < frames; i++)
	{
		quadrant_counter = 0;
		for (unsigned long j = 0; j < frame_size; j++)
		{
			*(write + i * (unsigned long) (frame_size * 1.5) + y * (unsigned long) (frame_size * 1.5) + j) =
				*(read + i * (frame_size * CHANNELS) + y * (frame_size) + j);

			if (j / frame_height % 2 == 1)
			{
				if (j % 2 == 0)
				{
					*(write + i * (unsigned long) (frame_size * 1.5) + (frame_size) + quadrant_counter) =
						*(read + i * (frame_size * CHANNELS) + u * (frame_size) + j);

					*(write + i * (unsigned long) (frame_size * 1.5) + (unsigned long) (frame_size * 1.25) + quadrant_counter) =
						*(read + i * (frame_size * CHANNELS) + v * (frame_size) + j);

					quadrant_counter++;
				}
			}
		}
	}
}

void convert_420p_to_444p(char *read, char *write, unsigned long frames, unsigned long frame_size, unsigned long frame_height)
{
	printf("Converting 420p to 444p...\n");

	unsigned long frame_width = frame_size / frame_height;
	unsigned long quadrant_counter;
	for (unsigned long i = 0; i < frames; i++)
	{
		quadrant_counter = 0;
		for (unsigned long j = 0; j < frame_size; j++)
		{
			*(write + i * (frame_size * CHANNELS) + y * (frame_size) + j) =
				*(read + i * (unsigned long) (frame_size * 1.5) + y * (unsigned long) (frame_size * 1.5) + j);

			*(write + i * (frame_size * CHANNELS) + u * (frame_size) + j) =
				*(read + i * (unsigned long) (frame_size * 1.5) + frame_size + quadrant_counter);

			*(write + i * (frame_size * CHANNELS) + v * (frame_size) + j) =
				*(read + i * (unsigned long) (frame_size * 1.5) + (unsigned long) (frame_size * 1.25) + quadrant_counter);

			if (j / frame_height % 2 == 1 && j % 2 == 0)
			{
				quadrant_counter++;
			}
		}
	}
}

void write_to_file(char *source, FILE *fptr, unsigned long bytes)
{
	// write to file
	printf("Writing to file...\n");
	for (unsigned long i = 0; i < bytes; i++)
	{
		fwrite(source + i, sizeof(char), 1, fptr);
	}
}

void task1(char *source, char *dest, unsigned long bytes)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video1.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_rgb_to_yuv(source, dest, FRAMES, HEIGHT * WIDTH);

	write_to_file(dest, fptr_write, bytes);	
	fclose(fptr_write);
}

void task2(char *source, char *dest, unsigned long bytes)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video2.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_444p_to_420p(source, dest, FRAMES, HEIGHT * WIDTH, HEIGHT);

	write_to_file(dest, fptr_write, FRAMES * (unsigned long) (HEIGHT * WIDTH * 1.5));
	fclose(fptr_write);
}

void task3(char *source, char *dest, unsigned long bytes)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video3.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_420p_to_444p(source, dest, FRAMES, HEIGHT * WIDTH, HEIGHT);

	write_to_file(dest, fptr_write, TOTAL_DATA);
	fclose(fptr_write);
}

int main(void)
{
	// open source
	FILE *fptr_read;
	if ((fptr_read = fopen("./rgb_video.yuv", "rb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	// read file
	printf("Reading file...\n");
	char *read = (char *) malloc(sizeof(char) * TOTAL_DATA);
	for (unsigned long i = 0; i < TOTAL_DATA; i++)
	{
		fread(read + i, sizeof(char), 1, fptr_read);
	}
	fclose(fptr_read);

	// convert RGB to YUV
	char *write = (char *) malloc(sizeof(char) * TOTAL_DATA);
	task1(read, write, TOTAL_DATA);

	// convert yuv444p to yuv420p	
	/*char *write = (char *) malloc(sizeof(char) * FRAMES * (unsigned long) ((HEIGHT * WIDTH) * 1.5));
	task2(read, write, TOTAL_DATA);*/

	// convert yuv420p to yuv444p
	/*char *write = (char *) malloc(sizeof(char) * TOTAL_DATA);
	task3(read, write, TOTAL_DATA);*/

	free(read);
	return 0;
}
