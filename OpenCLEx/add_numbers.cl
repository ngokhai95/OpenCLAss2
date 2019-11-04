

__kernel void add_numbers(__global float* data, 
      __local float* local_result, __global float* group_result) 
{
	float g = 9.8;
	float deltaTime = 0.02;
	float PI = 3.14;
   float sum;
   //float4 input1, input2, sum_vector;
   float positionBulletX, positionBulletY, positionBulletZ;
   uint global_addr, local_addr;

   float positionX;
   float positionY;
   float positionZ;
   float velocity, velocityH, velocityL, velocityV, alpha, gamma, angle;
   float shootTime;

   velocity = 10;
   alpha = 75;
   gamma = 50;

   //global_addr = get_global_id(0) * 3;
   positionBulletX = data[0];
   positionBulletY = data[1];
   positionBulletZ = data[2];
   /*positionBulletX = 10;
   positionBulletY = 0.1;
   positionBulletZ = 10;*/
   //sum_vector = input1 + input2;


   /*while (positionBulletY > 0)
   {
	   velocityH = velocity * sin(alpha * PI / 180) * cos(gamma * PI / 180);
	   velocityV = velocity * cos(alpha * PI / 180);
	   velocityL = velocity * sin(alpha * PI / 180) * sin(gamma * PI / 180);

	   shootTime += deltaTime;

	   positionX += velocityH * shootTime;
	   positionY += velocityV * shootTime - (g * pow(shootTime, 2) * 0.5);
	   positionZ += velocityL * shootTime;

	   velocityV -= g * shootTime;
	   positionBulletX += positionX;
	   positionBulletY += positionY;
	   positionBulletZ += positionZ;
   }*/



  /* local_addr = get_local_id(0);
   local_result[local_addr] = sum_vector.s0 + sum_vector.s1 + 
                              sum_vector.s2 + sum_vector.s3; */
   

   /*if (get_local_id(0) == 0) {
	   sum = 0.0f;
	   for (int i = 0; i < get_local_size(0); i++) {
		   sum += local_result[i];
	   }
   }*/
	barrier(CLK_LOCAL_MEM_FENCE);
    group_result[get_group_id(0)] = positionBulletX;
	group_result[get_group_id(0) + 1] = positionBulletY;
	group_result[get_group_id(0)+ 2] = positionBulletZ;
	//printf("Bullet position:{%f,%f,%f}\n", positionBullet.x, positionBullet.y, positionBullet.z);
}