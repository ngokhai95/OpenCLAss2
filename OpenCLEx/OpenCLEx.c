#define _CRT_SECURE_NO_WARNINGS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define PROGRAM_FILE "add_numbers.cl"
#define KERNEL_FUNC "add_numbers"
#define ARRAY_SIZE 64
#define PI 3.14
#define deltaTime 0.02
#define g 9.8

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h> 
#include <stdbool.h>


#ifdef MAC
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

struct Vector3
{
	float x;
	float y;
	float z;
};

clock_t start, end;
struct Vector3 position;
struct Vector3 positionBullet;
struct Vector3 originalPosition;
float time_used;
float velocity, velocityH, velocityL, velocityV, alpha, gama, angle;
float shootTime;
int numBoat;

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device(int type) {

	char device_cpu[1024];
	char device_gpu[1024];
	cl_platform_id platform;
	cl_device_id dev;
	int cpu, gpu, both;

	/* Access a device */
	switch(type)
	{
		case 0:
			cpu = clGetPlatformIDs(1, &platform, NULL);
			if (cpu < 0) {
				perror("Couldn't identify a platform");
				exit(1);
			}
			if (cpu < 0) {
				perror("Couldn't access CPU");
				exit(1);
			}
			cpu = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
			clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(device_cpu), &device_cpu, NULL);
            printf("\n");
			printf("Run using Supported CPU: %s!\n", device_cpu);
			break;
		case 1:
			gpu = clGetPlatformIDs(1, &platform, NULL);
			if (gpu < 0) {
				perror("Couldn't identify a platform");
				exit(1);
			}
			if (gpu < 0) {
				perror("Couldn't access GPU");
				exit(1);
			}
			gpu = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
			clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(device_gpu), &device_gpu, NULL);
            printf("\n");
			printf("Run using Supported GPU: %s!\n", device_gpu);
			break;
		case 2:
			both = clGetPlatformIDs(1, &platform, NULL);
			if (both < 0) {
				perror("Couldn't identify a platform");
				exit(1);
			}
			if (both < 0) {
				perror("Couldn't access CPU and GPU");
				exit(1);
			}
			both = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &dev, NULL);
			cpu = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
			clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(device_cpu), &device_cpu, NULL);
			gpu = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
			clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(device_gpu), &device_gpu, NULL);
			printf("Run using Supported CPU: %s and GPU: %s!\n", device_cpu, device_gpu);
			break;
		default:
			perror("Couldn't access any devices");
			exit(1);
			break;
	}

	return dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

	cl_program program;
	FILE* program_handle;
	char* program_buffer, * program_log;
	size_t program_size, log_size;
	int err;

	/* Read program file and place content into buffer */
	program_handle = fopen(filename, "r");
	if (program_handle == NULL) {
		perror("Couldn't find the program file");
		exit(1);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file */
	program = clCreateProgramWithSource(ctx, 1,
		(const char**)&program_buffer, &program_size, &err);
	if (err < 0) {
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);

	/* Build program */
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = (char*)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}

	return program;
}

void Shoot()
{
	shootTime += deltaTime;

	position.x += velocityH * shootTime;
	position.y += velocityV * shootTime - (g * pow(shootTime, 2) * 0.5);
	position.z += velocityL * shootTime;

	velocityV -= g * shootTime;
	positionBullet.x += position.x;
	positionBullet.y += position.y;
	positionBullet.z += position.z;
}

int print(float v, float alpha, float gama, float x, float y, float z)
{
	printf("Velocity: %f, Vertical angle: %f degree, Horizontal angle: %f degree\n", v, alpha, gama);
	printf("Original Position:{%f,%f,%f}\n", x, y, z);
}

int printResult(int n, float x, float y, float z)
{
	printf("Bullets hit the ground!\n");
	printf("Final Bullet Position:{%f,%f,%f}\n", x, y, z);
}

void runSerially()
{
    printf("\n");
    printf("Run Serially!\n");
	positionBullet = originalPosition;
	velocityH = velocity * sin(alpha * PI / 180) * cos(gama * PI / 180);
	velocityV = velocity * cos(alpha * PI / 180);
	velocityL = velocity * sin(alpha * PI / 180) * sin(gama * PI / 180);
	start = clock();
	print(velocity, alpha, gama, positionBullet.x, positionBullet.y, positionBullet.z);
    numBoat = pow(numBoat,3);
	for (int i = 0; i < numBoat; i++)
	{
		while (positionBullet.y > 0)
		{
			Shoot();
		}
	}
	printResult(numBoat, positionBullet.x, positionBullet.y, positionBullet.z);
	end = clock();
	time_used = ((float)(end - start));

	printf("Time Used: %f\n", time_used);
}

void runOpenCL(int mode, float numBoat, float velocity, float alpha, float gama, float posX, float posY, float posZ)
{
	/* OpenCL structures */
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue queue;
	cl_int err;
	size_t local_size, global_size;

	/* Data and buffers */
	float input[7];
	float output[6];
	cl_mem input_buffer, output_buffer;

	input[0] = posX;
	input[1] = posY;
	input[2] = posZ;
	input[3] = velocity;
	input[4] = alpha;
	input[5] = gama;
	input[6] = numBoat;

	/* Create device and context */
	device = create_device(mode);
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

	/* Build program */

	program = build_program(context, device, PROGRAM_FILE);

	/* Create data buffer */
	print(input[3], input[4], input[5], input[0], input[1], input[2]);
	global_size = 8;
	local_size = 1;
	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
		CL_MEM_COPY_HOST_PTR, 7 * sizeof(float), input, &err);
	output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
		CL_MEM_COPY_HOST_PTR, 6 * sizeof(float), output, &err);
	if (err < 0) {
		perror("Couldn't create a buffer");
		exit(1);
	};
	/* Create a command queue */
	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	};

	/* Create a kernel */
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if (err < 0) {
		perror("Couldn't create a kernel");
		exit(1);
	};

	/* Create kernel arguments */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
	err |= clSetKernelArg(kernel, 1, local_size * sizeof(float), NULL);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_buffer);
	if (err < 0) {
		perror("Couldn't create a kernel argument");
		exit(1);
	}
	/* Enqueue kernel */
	start = clock();
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size,
		&local_size, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't enqueue the kernel");
		exit(1);
	}

	/* Read the kernel's output */
	err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0,
		sizeof(output), output, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't read the buffer");
		exit(1);
	}

	/* Check result */
	end = clock();
	time_used = ((float)(end - start));
	printResult(numBoat, output[0], output[1], output[2]);
	printf("Time Used: %f\n", time_used);

	clReleaseKernel(kernel);
	clReleaseMemObject(output_buffer);
	clReleaseMemObject(input_buffer);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
}

int main() 
{
	srand(time(NULL));
	originalPosition.x = rand() % 100;
	originalPosition.y = (float)(rand() % 10 + 1) / 100;
	originalPosition.z = rand() % 100;
	velocity = rand() % (100 - 50 + 1) + 50;
	alpha = rand() % (170 - 10 + 1) + 10;
	gama = rand() % (170 - 10 + 1) + 10;
	numBoat = rand() % (400 - 200  + 1) + 200;
    printf("%d boats shooting bullets!\n", numBoat);
	runSerially();
	runOpenCL(2, numBoat, velocity, alpha, gama, originalPosition.x, originalPosition.y, originalPosition.z);
	runOpenCL(1, numBoat, velocity, alpha, gama, originalPosition.x, originalPosition.y, originalPosition.z);

return 0;
}


