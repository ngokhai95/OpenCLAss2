__kernel void add_numbers(__global float4* data, 
      __local float* local_result, __global float* group_result) {

   float sum;
   float4 input1, input2, sum_vector;
   uint global_addr, local_addr;

   global_addr = get_global_id(0) * 2;
   input1 = data[global_addr];
   input2 = data[global_addr+1];
   sum_vector = input1 + input2;

   local_addr = get_local_id(0);
   local_result[local_addr] = sum_vector.s0 + sum_vector.s1 + 
                              sum_vector.s2 + sum_vector.s3; 
   barrier(CLK_LOCAL_MEM_FENCE);

   if(get_local_id(0) == 0) {
      sum = 0.0f;
      for(int i=0; i<get_local_size(0); i++) {
         sum += local_result[i];
      }
      group_result[get_group_id(0)] = sum;
   }
}

/*private void Shoot()
{
	velocityH = velocity * System.Math.Sin(Mathf.Deg2Rad * alpha) * System.Math.Cos(Mathf.Deg2Rad * gamma);
	velocityV = velocity * System.Math.Cos(Mathf.Deg2Rad * alpha);
	velocityL = velocity * System.Math.Sin(Mathf.Deg2Rad * alpha) * System.Math.Sin(Mathf.Deg2Rad * gamma);
	velocityV = velocity * System.Math.Sin(angle);
	velocityH = velocity * System.Math.Cos(angle);
	shootTime = Time.deltaTime;
	time += shootTime;
	timeUI.GetComponent<Text>().text = "Time: " + time.ToString("0.0000");

	zPos += (float)(velocityH * shootTime);
	yPos += (float)(velocityV * shootTime - (g * System.Math.Pow(shootTime, 2) * 0.5));
	xPos += (float)(velocityL * shootTime);

	velocityV -= g * shootTime;
	bullet.transform.position = gun.transform.position + new Vector3(xPos, yPos, zPos);

	if (bullet.transform.position.y < 0.05)
	{
		Debug.Break();
	}


}*/
