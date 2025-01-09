#include <cuda.h>
#include <iostream>

__global__ void foo()
{

}

int main()
{
    foo<<<256, 256>>>();
    auto err = cudaGetLastError();
    auto str = cudaGetErrorString(err);
    std::cout << str << std::endl;
    return 0;
}
