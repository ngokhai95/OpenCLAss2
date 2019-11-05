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
float time_used;
float velocity, velocityH, velocityL, velocityV, alpha, gamma, angle;
float shootTime;
int numShoot;

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device(int type) {

	char device_string[1024];
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
			clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
			printf("OpenCL Supported CPU: %s\n", device_string);
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
			clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
			printf("OpenCL Supported GPU: %s\n", device_string);
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
			break;
		default:
			perror("Couldn't access any devices");
			exit(1);
			break;
	}
	/*err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
	if (err == CL_DEVICE_NOT_FOUND) {
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
	}
	if (err < 0) {
		perror("Couldn't access any devices");
		exit(1);
	}*/

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
	program_handle = fopen(filename, "rb");
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
	//printf("Bullet position:{%f,%f,%f}\n", positionBullet.x, positionBullet.y, positionBullet.z);
}

int print(int n, float v, float alpha, float gamma, float x, float y, float z)
{
	printf("Shooting bullet with velocity %f for %d times\n", v, n);
	printf("Vertical angle: %f degree, Horizontal angle: %f degree\n", alpha, gamma);
	printf("Original Position:{%f,%f,%f}\n", x, y, z);
}

int printResult(int n, float x, float y, float z)
{
	printf("%d bullets hit the ground!\n", n);
	printf("Final Bullet Position:{%f,%f,%f}\n", x, y, z);
}

int main() 
{

	/* OpenCL structures */
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue queue;
	cl_int i, j, err;
	size_t local_size, global_size;

	/* Data and buffers */
	float input[7];
	float output[6];
	cl_mem input_buffer, output_buffer;

	/* Initialize data */
	srand(time(NULL));
	positionBullet.x = 10;
	positionBullet.y = 0.01;
	positionBullet.z = 10;
	velocity = 10;
	alpha = 70;
	gamma = 50;
	numShoot = 100000000;
	input[0] = 10;
	input[1] = 0.01;
	input[2] = 10;
	input[3] = velocity;
	input[4] = alpha;
	input[5] = gamma;
	input[6] = numShoot;
	velocityH = velocity * sin(alpha * PI / 180) * cos(gamma * PI / 180);
	velocityV = velocity * cos(alpha * PI / 180);
	velocityL = velocity * sin(alpha * PI / 180) * sin(gamma * PI / 180);
	start = clock();
	print(numShoot, velocity, alpha, gamma, positionBullet.x, positionBullet.y, positionBullet.z);
	for (int i = 0; i < numShoot; i++)
	{
		while (positionBullet.y > 0)
		{
			Shoot();
		}
	}
	printResult(numShoot,positionBullet.x,positionBullet.y,positionBullet.z);
	end = clock();
	time_used = ((float)(end - start));
	
	printf("Time Used: %f\n", time_used);
	/* Create device and context */
	device = create_device(0); //0 = cpu, 1 = gpu, 2 = both.
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

	/* Build program */
	start = clock();
	program = build_program(context, device, PROGRAM_FILE);

	/* Create data buffer */
	print(input[6], input[3], input[4], input[5], input[0], input[1], input[2]);

	global_size = 8;
	local_size = 4;
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
	printResult(numShoot, output[0], output[1], output[2]);
	printf("Time Used: %f\n", time_used);

	clReleaseKernel(kernel);
	clReleaseMemObject(output_buffer);
	clReleaseMemObject(input_buffer);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
return 0;
}
