__constant float g = 9.8;
__constant float deltaTime = 0.02;
__constant float PI = 3.14;

__kernel void add_numbers(__global float* data, 
      __local float* local_result, __global float* group_result) 
{
   float sum;
   float positionBulletX, positionBulletY, positionBulletZ;
   uint global_addr, local_addr;

   float positionX = 0;
   float positionY = 0;
   float positionZ = 0;
   float velocity, velocityH, velocityL, velocityV, alpha, gamma = 0;
   float shootTime = 0;
   int numShoot = 0;
  
   positionBulletX = data[0];
   positionBulletY = data[1];
   positionBulletZ = data[2];
   velocity = data[3];
   alpha = data[4];
   gamma = data[5];
   numShoot = data[6];
   velocityH = velocity * sin(alpha * PI / 180) * cos(gamma * PI / 180);
   velocityV = velocity * cos(alpha * PI / 180);
   velocityL = velocity * sin(alpha * PI / 180) * sin(gamma * PI / 180);

   for (int i = 0; i < 1000000; i++)
   {
	   while (positionBulletY > 0)
	   {
		   shootTime += deltaTime;

		   positionX += velocityH * shootTime;
		   positionY += velocityV * shootTime - (g * pow(shootTime, 2) * 0.5);
		   positionZ += velocityL * shootTime;

		   velocityV -= g * shootTime;
		   positionBulletX += positionX;
		   positionBulletY += positionY;
		   positionBulletZ += positionZ;
	   }
   }

   barrier(CLK_LOCAL_MEM_FENCE);
   group_result[0] = positionBulletX;
   group_result[1] = positionBulletY;
   group_result[2] = positionBulletZ;
}