#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void convert_rgb_to_yuv(unsigned char *read, unsigned char *write, unsigned long frames, 
	unsigned long frame_size)
{
	printf("Converting RGB to YUV...\n");
	unsigned char R, G, B, Y, U, V;
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
}

void convert_444p_to_420p(unsigned char *read, unsigned char *write, unsigned long frames, 
	unsigned long frame_size, unsigned long frame_width)
{
	printf("Converting 444p to 420p...\n");
	unsigned long quadrant_num;
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

void convert_420p_to_444p(unsigned char *read, unsigned char *write, unsigned long frames, 
	unsigned long frame_size, unsigned long frame_width)
{
	printf("Converting 420p to 444p...\n");

	unsigned long quadrant_num;
	unsigned char *U;
	unsigned char *V;
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

void write_to_file(unsigned char *source, FILE *fptr, unsigned long chunk_size, 
	unsigned long total_data)
{
    // write to file
    printf("Writing to file...\n");
    /*for (unsigned long i = 0; i < total_data; i += chunk_size)
    {
	printf("%u", *(source + i));
        fwrite(source + i, sizeof(char), chunk_size, fptr);
    }*/
    fwrite(source, sizeof(char), total_data, fptr);
}

void task1(unsigned char *source, unsigned char *dest, unsigned long height, 
	unsigned long width, unsigned long frames)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video1.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_rgb_to_yuv(source, dest, frames, height * width);

	write_to_file(dest, fptr_write, 1, TOTAL_DATA);//height * width * frames * CHANNELS);
	fclose(fptr_write);
}

void task2(unsigned char *source, unsigned char *dest, unsigned long height, 
	unsigned long width, unsigned long frames)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video2.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_444p_to_420p(source, dest, frames, height * width, width);

	write_to_file(dest, fptr_write, height * width * 1.5, height * width * frames * 1.5);
	fclose(fptr_write);
}

void task3(unsigned char *source, unsigned char *dest, unsigned long height,
        unsigned long width, unsigned long frames)
{
	// open file
	FILE *fptr_write;
	if ((fptr_write = fopen("./yuv_video3.yuv", "wb")) == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	convert_420p_to_444p(source, dest, frames, height * width, width);

	write_to_file(dest, fptr_write, height * width * CHANNELS, height * width * frames * CHANNELS);
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
	for (unsigned long i = 0; i < TOTAL_DATA; i += HEIGHT * WIDTH * CHANNELS)
	{
		fread(read + i, sizeof(unsigned char), HEIGHT * WIDTH * CHANNELS, fptr_read);
	}
	fclose(fptr_read);

	// convert RGB to YUV
	unsigned char *write1 = (unsigned char *) malloc(sizeof(unsigned char) * TOTAL_DATA);
	task1(read, write1, HEIGHT, WIDTH, FRAMES);

	// convert yuv444p to yuv420p	
	unsigned char *write2 = (unsigned char *) malloc(sizeof(unsigned char) * FRAMES * (unsigned long) ((HEIGHT * WIDTH) * 1.5));
	task2(write1, write2, HEIGHT, WIDTH, FRAMES);

	// convert yuv420p to yuv444p
	unsigned char *write3 = (unsigned char *) malloc(sizeof(unsigned char) * TOTAL_DATA);
	task3(write2, write3, HEIGHT, WIDTH, FRAMES);

	free(read);
	free(write1);
	free(write2);
	free(write3);

	return 0;
}
