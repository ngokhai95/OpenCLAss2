////////////////////////////////////////////////////////////////////////
//
//  HelloWorld - sample OpenCL application (kernel)
//  Adapted from http://www.drdobbs.com/parallel/a-gentle-introduction-to-opencl/231002854
//      Adds the numbers from 1..N
//  (c) Borna Noureddin
//  British Columbia Institute of Technology
//
////////////////////////////////////////////////////////////////////////


__kernel void add_numbers(__global float4* data,
                          __local float* local_result,
                          __global float* group_result)
{
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

    printf("%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
           global_addr, local_addr, get_local_id(0),
           input1.s0, input1.s1, input1.s2, input1.s3,
           input2.s0, input2.s1, input2.s2, input2.s3,
           sum_vector.s0, sum_vector.s1, sum_vector.s2, sum_vector.s3);
    
    if(get_local_id(0) == 0) {
        sum = 0.0f;
        for(int i=0; i<get_local_size(0); i++) {
            sum += local_result[i];
        }
        group_result[get_group_id(0)] = sum;
    }
}
