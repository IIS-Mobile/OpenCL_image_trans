__kernel __attribute__((vec_type_hint(uint4)))
void image_rotate(__global uint4 *A, const int w, const int h, __local uint4 * localTemp,
                  __local uint4 * localTemp2)

{
    //cos(90 degrees) = 0;
    //sin (90 degrees) = 1;
    int idx = get_global_id(0);
    int idy = get_global_id(1);
    int locIdy = get_local_id(0);
    int locIdx = get_local_id(1);
    int locW = get_local_size(0);
    int locH = get_local_size(1);

    localTemp[locIdy * locW + locIdx] = A[idy*w + idx];
    barrier(CLK_LOCAL_MEM_FENCE |  CLK_GLOBAL_MEM_FENCE);

    localTemp2[locIdy * locW + locIdx] = localTemp[locIdx * locH + locIdy];
    barrier(CLK_LOCAL_MEM_FENCE |  CLK_GLOBAL_MEM_FENCE);

    //A[idy*w + idx] = localTemp[locIdx * locH + locIdy];
    A[idy*w + idx] = localTemp2[((locH-1)-locIdy) * locW + locIdx];
    barrier(CLK_LOCAL_MEM_FENCE |  CLK_GLOBAL_MEM_FENCE);



}
