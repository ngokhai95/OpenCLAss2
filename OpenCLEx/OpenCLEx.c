#define _CRT_SECURE_NO_WARNINGS
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


/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device(int type) {

	char device_string[1024];
	cl_platform_id platform;
	cl_device_id dev;
	int cpu, gpu, both;

	/* Identify a platform */
	cpu = clGetPlatformIDs(1, &platform, NULL);
	if (cpu < 0) {
		perror("Couldn't identify a platform");
		exit(1);
	}

	gpu = clGetPlatformIDs(1, &platform, NULL);
	if (gpu < 0) {
		perror("Couldn't identify a platform");
		exit(1);
	}

	both = clGetPlatformIDs(1, &platform, NULL);
	if (both < 0) {
		perror("Couldn't identify a platform");
		exit(1);
	}
	/* Access a device */
	switch(type){
		case 0:
			cpu = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
			if (cpu < 0) {
				perror("Couldn't access CPU");
				exit(1);
			}
			break;
		case 1:
			gpu = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
			clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
			printf("OpenCL Supported GPU: %s\n", device_string);
			if (gpu < 0) {
				perror("Couldn't access GPU");
				exit(1);
			}
			break;
		case 2:
			both = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &dev, NULL);
			if (both < 0) {
				perror("Couldn't access CPU and GPU");
				exit(1);
			}
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
	velocityH = velocity * sin(alpha * PI / 180) * cos(gamma * PI / 180);
	velocityV = velocity * cos(alpha * PI / 180);
	velocityL = velocity * sin(alpha * PI / 180) * sin(gamma * PI / 180);
	//velocityV = velocity * Sin(angle);
	//velocityH = velocity * Cos(angle);
	shootTime += deltaTime;

	position.x += velocityH * shootTime;
	position.y += velocityV * shootTime - (g * pow(shootTime, 2) * 0.5);
	position.z += velocityL * shootTime;

	velocityV -= g * shootTime;
	positionBullet.x += position.x;
	positionBullet.y += position.y;
	positionBullet.z += position.z;
	printf("Bullet position:{%f,%f,%f}\n", positionBullet.x, positionBullet.y, positionBullet.z);
}

int printSolution()
{
	printf("Bullet hit the ground!\n");
	printf("Final Bullet Position:{%f,%f,%f}\n", positionBullet.x, positionBullet.y, positionBullet.z);
	printf("Time till hit the ground:%f\n",shootTime);
}

int main() {

	/* OpenCL structures */
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue queue;
	cl_int i, j, err;
	size_t local_size, global_size;

	/* Data and buffers */
	float data[ARRAY_SIZE];
	float sum[2], total, actual_sum;
	float input[3];
	float output[3];
	cl_mem input_buffer, output_buffer;
	cl_int num_groups;

	/* Initialize data */
	for (i = 0; i < ARRAY_SIZE; i++) {
		data[i] = 1.0f * i;
	}
	positionBullet.x = 10;
	positionBullet.y = 0.01;
	positionBullet.z = 10;
	input[0] = positionBullet.x;
	input[1] = positionBullet.y;
	input[2] = positionBullet.z;
	velocity = 10;
	alpha = 75;
	gamma = 50;

	printf("Shooting bullet with velocity %f\n", velocity);
	printf("Vertical angle: %f degree, Horizontal angle: %f degree\n", alpha, gamma);
	printf("Original Position:{%f,%f,%f}\n", positionBullet.x, positionBullet.y, positionBullet.z);
	/*while (positionBullet.y > 0)
	{
		Shoot();
	}
	printSolution();*/

	/* Create device and context */
	device = create_device(1); //0 = cpu, 1 = gpu, 2 = both.
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

	/* Build program */
	start = clock();

	//dijkstra(graph, 0);
	program = build_program(context, device, PROGRAM_FILE);

	/* Create data buffer */

	global_size = 8;
	local_size = 4;
	num_groups = global_size / local_size;
	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY |
		CL_MEM_COPY_HOST_PTR, 3 * sizeof(float), input, &err);
	output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
		CL_MEM_COPY_HOST_PTR, 3 * sizeof(float), output, &err);
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

	printf("Bullet hit the ground!\n");
	printf("Final Bullet Position:{%f,%f,%f}\n", output[0], output[1], output[2]);
	printf("Time till hit the ground:%f\n", shootTime);
	/* Check result */
	/*total = 0.0f;
	for (j = 0; j < num_groups; j++) {
		total += sum[j];
	}
	actual_sum = 1.0f * ARRAY_SIZE / 2 * (ARRAY_SIZE - 1);
	printf("Computed sum = %.1f.\n", total);
	if (fabs(total - actual_sum) > 0.01 * fabs(actual_sum))
		printf("Check failed.\n");
	else
		printf("Check passed.\n");*/

	/* Deallocate resources */
	clReleaseKernel(kernel);
	clReleaseMemObject(output_buffer);
	clReleaseMemObject(input_buffer);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);

	

	

	end = clock();
	time_used = ((float)(end = start));

	printf("Time Used: %f",time_used);
	return 0;
}
