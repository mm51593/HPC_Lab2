#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

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

void convert_rgb_to_yuv(unsigned char *read, unsigned char *write, unsigned long frames, unsigned long frame_size)
{
	printf("Converting RGB to YUV...\n");
	unsigned char R, G, B, Y, U, V;
	#pragma omp parallel for private (R, G, B, Y, U, V) schedule(dynamic)
	for (unsigned long i = 0; i < frames; i++)
	{
		for (unsigned long j = 0; j < frame_size; j++)
		{
			R = *(read + i * (frame_size * CHANNELS) + r * (frame_size) + j);
			G = *(read + i * (frame_size * CHANNELS) + g * (frame_size) + j);
			B = *(read + i * (frame_size * CHANNELS) + b * (frame_size) + j);

			Y =  (unsigned char)(0.257 * R + 0.504 * G + 0.098 * B + 16);
			U =  (unsigned char)(-0.148 * R - 0.291 * G + 0.439 * B + 128);
			V =  (unsigned char)(0.439 * R - 0.368 * G - 0.071 * B + 128);

			*(write + i * (frame_size * CHANNELS) + y * (frame_size) + j) = Y;
			*(write + i * (frame_size * CHANNELS) + u * (frame_size) + j) = U;
			*(write + i * (frame_size * CHANNELS) + v * (frame_size) + j) = V;
		}
	}
	printf("Done converting.\n");
}

void convert_444p_to_420p(unsigned char *read, unsigned char *write, unsigned long frames, unsigned long frame_size, unsigned long frame_width)
{
	printf("Converting 444p to 420p...\n");
	unsigned long quadrant_num;
	#pragma omp parallel for private (quadrant_num) schedule(dynamic)
	for (unsigned long i = 0; i < frames; i++)
	{
		for (unsigned long j = 0; j < frame_size; j++)
		{
			*(write + i * (unsigned long) (frame_size * 1.5) + y * (unsigned long) (frame_size * 1.5) + j) =
				*(read + i * (frame_size * CHANNELS) + y * (frame_size) + j);

			if (j / frame_width % 2 == 1)
			{
				if (j % 2 == 0)
				{
					quadrant_num = (j / (frame_width * 2)) * (frame_width / 2) + j % frame_width / 2;
					*(write + i * (unsigned long) (frame_size * 1.5) + (frame_size) + quadrant_num) =
						*(read + i * (frame_size * CHANNELS) + u * (frame_size) + j);

					*(write + i * (unsigned long) (frame_size * 1.5) + (unsigned long) (frame_size * 1.25) + quadrant_num) =
						*(read + i * (frame_size * CHANNELS) + v * (frame_size) + j);

				}
			}
		}
	}
}

void convert_420p_to_444p(unsigned char *read, unsigned char *write, unsigned long frames, unsigned long frame_size, unsigned long frame_width)
{
	printf("Converting 420p to 444p...\n");

	unsigned long quadrant_num;
	unsigned char *U;
	unsigned char *V;
	#pragma omp parallel for private (quadrant num, U, V) schedule(dynamic) collapse(2)
	for (unsigned long i = 0; i < frames; i++)
	{
		for (unsigned long j = 0; j < frame_size; j++)
		{
			*(write + i * (frame_size * CHANNELS) + y * (frame_size) + j) =
				*(read + i * (unsigned long) (frame_size * 1.5) + y * (unsigned long) (frame_size * 1.5) + j);

			if (j / frame_width % 2 == 1 && j % 2 == 0)
			{
				quadrant_num = (j / (frame_width * 2)) * (frame_width / 2) + j % frame_width / 2;
				U = write + i * (frame_size * CHANNELS) + u * (frame_size) + j;
				*(U) = *(U + 1) = *(U - frame_width) = *(U - frame_width + 1) =
					*(read + i * (unsigned long) (frame_size * 1.5) + frame_size + quadrant_num);

				V = write + i * (frame_size * CHANNELS) + v * (frame_size) + j;
				*(V) = *(V + 1) = *(V - frame_width) = *(V - frame_width + 1) =
					*(read + i * (unsigned long) (frame_size * 1.5) + (unsigned long) (frame_size * 1.25) + quadrant_num);
			}
		}
	}
}

void write_to_file(unsigned char *source, FILE *fptr, unsigned long bytes)
{
	// write to file
	printf("Writing to file...\n");
	fwrite(source, sizeof(unsigned char), bytes, fptr);
}

void task1(unsigned char *source, unsigned char *dest, unsigned long bytes)
{
    convert_rgb_to_yuv(source, dest, FRAMES, HEIGHT * WIDTH);

    // open file
    FILE *fptr_write;
    if ((fptr_write = fopen("./yuv_video1.yuv", "wb")) == NULL)
    {
	    printf("Error opening file.\n");
	    exit(1);
    }
    write_to_file(dest, fptr_write, bytes);
    fclose(fptr_write);
}

void task2(unsigned char *source, unsigned char *dest, unsigned long bytes)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video2.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_444p_to_420p(source, dest, FRAMES, HEIGHT * WIDTH, WIDTH);

	write_to_file(dest, fptr_write, FRAMES * (unsigned long) (HEIGHT * WIDTH * 1.5));
	fclose(fptr_write);
}

void task3(unsigned char *source, unsigned char *dest, unsigned long bytes)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video3.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_420p_to_444p(source, dest, FRAMES, HEIGHT * WIDTH, WIDTH);

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
	unsigned char *read = (unsigned char *) malloc(sizeof(char) * TOTAL_DATA);
	for (unsigned long i = 0; i < TOTAL_DATA; i += WIDTH * HEIGHT)
	{
		fread(read + i, sizeof(unsigned char), WIDTH * HEIGHT, fptr_read);
	}
	//fread(read, sizeof(unsigned char), TOTAL_DATA, fptr_read);
	fclose(fptr_read);

	// convert RGB to YUV
	unsigned char *write1 = (unsigned char *) malloc(sizeof(unsigned char) * TOTAL_DATA);
	task1(read, write1, TOTAL_DATA);

	// convert yuv444p to yuv420p	
	unsigned char *write2 = (unsigned char *) malloc(sizeof(unsigned char) * FRAMES * (unsigned long) ((HEIGHT * WIDTH) * 1.5));
	task2(write1, write2, TOTAL_DATA);

	// convert yuv420p to yuv444p
	unsigned char *write3 = (unsigned char *) malloc(sizeof(unsigned char) * TOTAL_DATA);
	task3(write2, write3, TOTAL_DATA);

	free(read);
	free(write1);
	free(write2);
	free(write3);
	return 0;
}
