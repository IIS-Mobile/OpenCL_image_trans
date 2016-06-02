
// System includes
#include <stdio.h>
#include <stdlib.h>

//SDL includes
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

// OpenCL includes
#include <CL/cl.h>

// Defines
#define FRAMES_PER_SECOND 30


SDL_Surface* ekran = NULL;
SDL_Surface* obraz = NULL;
// Signatures
char* readSource(const char *sourceFilename);

int main(int argc, char ** argv)
{
    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );

    printf("Running program\n\n");

   	//obraz = IMG_Load("fraktaler-i3.png");
   	obraz = IMG_Load("image.png");
	ekran = SDL_SetVideoMode( obraz->w, obraz->h, 32, SDL_SWSURFACE );
	int rozmiar_wej = obraz->h * obraz->w;

	SDL_BlitSurface( obraz, NULL, ekran, NULL );
	SDL_Flip(ekran);


    printf("Initializing OpenCL\n\n");
    cl_int status;  // use as return value for most OpenCL functions

    cl_uint numPlatforms = 0;
    cl_platform_id *platforms;

    // Query for the number of recongnized platforms
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(status != CL_SUCCESS) {
      printf("clGetPlatformIDs failed\n");
      getchar();
      exit(-732);
    }

    // Make sure some platforms were found
    if(numPlatforms == 0) {
      printf("No platforms detected.\n");
      getchar();
      exit(-2);
    }

    // Allocate enough space for each platform
    platforms = (cl_platform_id*)malloc(numPlatforms*sizeof(cl_platform_id));
    if(platforms == NULL) {
      perror("malloc");
      getchar();
      exit(-3);
    }

    // Fill in platforms
    clGetPlatformIDs(numPlatforms, platforms, NULL);
    if(status != CL_SUCCESS) {
      printf("clGetPlatformIDs failed\n");
      getchar();
      exit(-4);
    }

    // Print out some basic information about each platform
    printf("%u platforms detected\n", numPlatforms);
    for(unsigned int i = 0; i < numPlatforms; i++) {
      char buf[100];
      printf("Platform %u: \n", i);
      status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR,
                       sizeof(buf), buf, NULL);
      printf("\tVendor: %s\n", buf);
      status |= clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                       sizeof(buf), buf, NULL);
      printf("\tName: %s\n", buf);

      if(status != CL_SUCCESS) {
         printf("clGetPlatformInfo failed\n");
         getchar();
		 exit(-5);
      }
    }
    printf("\n");

    cl_uint numDevices = 0;
    cl_device_id *devices;

    // Retrive the number of devices present
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL,
                           &numDevices);
    if(status != CL_SUCCESS) {
      printf("clGetDeviceIDs failed\n");
      getchar();
      exit(-64);
    }

    // Make sure some devices were found
    if(numDevices == 0) {
      printf("No devices detected.\n");
      getchar();
      exit(-7832);
    }

    // Allocate enough space for each device
    devices = (cl_device_id*)malloc(numDevices*sizeof(cl_device_id));
    if(devices == NULL) {
      perror("malloc");
      getchar();
      exit(-876);
    }

    // Fill in devices
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, numDevices,
                     devices, NULL);
    if(status != CL_SUCCESS) {
      printf("clGetDeviceIDs failed\n");
      getchar();
      exit(-5432);
    }

    // Print out some basic information about each device
    printf("%u devices detected\n", numDevices);
    for(unsigned int i = 0; i < numDevices; i++) {
      char buf[100];
      size_t sizes;
      cl_ulong memSize;
      printf("Device %u: \n", i);
      status = clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR,
                       sizeof(buf), buf, NULL);
      printf("\tDevice: %s\n", buf);
      status |= clGetDeviceInfo(devices[i], CL_DEVICE_NAME,
                       sizeof(buf), buf, NULL);
      printf("\tName: %s\n", buf);

      status |= clGetDeviceInfo(devices[i], CL_DEVICE_LOCAL_MEM_SIZE,
                       sizeof(cl_ulong), &memSize, NULL);
      printf("\tLocal mem size: %d\n",sizes);

      status |= clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE,
                       sizeof(size_t), &sizes, NULL);
      printf("\tLocal workgroup size: %d\n", sizes);

      if(status != CL_SUCCESS) {
         printf("clGetDeviceInfo failed\n");
         getchar();
         exit(-3096);
      }
    }
    printf("\n");
    system("pause");
    cl_context context;

    // Create a context and associate it with the devices
    context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    if(status != CL_SUCCESS || context == NULL) {
      printf("clCreateContext failed\n");
      getchar();
      exit(-5302);
    }

    cl_command_queue cmdQueue;

    // Create a command queue and associate it with the device you
    // want to execute on
    cmdQueue = clCreateCommandQueue(context, devices[0], 0, &status);
    if(status != CL_SUCCESS || cmdQueue == NULL) {
      printf("clCreateCommandQueue failed\n");
      getchar();
      exit(-76230);
    }
    ///int cl_skala = skalowanie; //stworzenie parametrow do przekazania
    //   int cl_szerokosc_wej = szerokosc_wej;
    cl_mem d_A;  // Input buffers on device
    cl_mem d_B;


    printf("Preparing SDL data\n");

    //stworzenie wektorow dla pikseli
    Uint32 *piksele = (Uint32*)ekran->pixels;
    cl_uint4 *cl_piksele = (cl_uint4*)malloc(rozmiar_wej*sizeof(cl_uint4));
    //cl_uint4 *cl_piksele_wyj = (cl_uint4*)malloc(rozmiar_wyj*sizeof(cl_uint4));

    //zaladowanie kolejnych skladowych RGBA do typu wektorowego
    SDL_LockSurface(ekran);
    for(int i = 0; i < rozmiar_wej; i++)
    {
        cl_piksele[i].x = piksele[i] & 0xff; //blue
        cl_piksele[i].y = (piksele[i]&0xff00) >> 8; //green
        cl_piksele[i].z = (piksele[i]&0xff0000) >> 16; //red
        cl_piksele[i].w = (piksele[i]&0xff000000) >> 24; //alpha
    }
    SDL_UnlockSurface(ekran);

    // Stworzenie bufora dla pikseli obrazu wejsciowego
    d_A = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                   rozmiar_wej*sizeof(cl_uint4), cl_piksele, &status);

    if(status != CL_SUCCESS || d_A == NULL) {
      printf("clCreateBuffer failed\n");
      getchar();
      exit(-1702);
    }


    cl_program program;
    char *source;
    const char *sourceFile = "image_rotate.cl";
    source = readSource(sourceFile);
    program = clCreateProgramWithSource(context, 1, (const char**)&source,
                              NULL, &status);
    if(status != CL_SUCCESS) {
      printf("clCreateProgramWithSource failed\n");
      getchar();
      exit(-111);
    }

    cl_int buildErr;
    buildErr = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);

    // If there are build errors, print them to the screen
    if(buildErr != CL_SUCCESS) {
      printf("Program failed to build.\n");
      cl_build_status buildStatus;
      for(unsigned int i = 0; i < numDevices; i++) {
         clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS,
                          sizeof(cl_build_status), &buildStatus, NULL);
         if(buildStatus == CL_SUCCESS) {
            continue;
         }

         char *buildLog;
         size_t buildLogSize;
         clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG,
                          0, NULL, &buildLogSize);
         buildLog = (char*)malloc(buildLogSize);
         if(buildLog == NULL) {
            perror("malloc");
            getchar();
            exit(-237);
         }
         clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG,
                          buildLogSize, buildLog, NULL);
         buildLog[buildLogSize-1] = '\0';
         printf("Device %u Build Log:\n%s\n", i, buildLog);
         free(buildLog);
      }
      getchar();
      exit(0);
    }
    else {
      printf("No build errors\n\n");
    }

    cl_kernel kernel;

    // Create a kernel
    kernel = clCreateKernel(program, "image_rotate", &status);
    if(status != CL_SUCCESS) {
      printf("clCreateKernel failed\n");
      getchar();
      exit(-13);
    }

    int numLocalElements = 16;

                // Associate parameters with the kernel
    status  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_A);
    status |= clSetKernelArg(kernel, 1, sizeof(int), &obraz->w);
    status |= clSetKernelArg(kernel, 2, sizeof(int), &obraz->h);
    status |= clSetKernelArg(kernel, 3, sizeof(cl_uint4)*numLocalElements*numLocalElements, NULL);
    status |= clSetKernelArg(kernel, 4, sizeof(cl_uint4)*numLocalElements*numLocalElements, NULL);

    if(status != CL_SUCCESS)
    {
      printf("clSetKernelArg failed\n");
      getchar();
      exit(-14);
    }


    SDL_Event event;
    bool done = false;
    Uint32 ticks;
    Uint32 piksel;
    unsigned long frameNumber = 0;
    size_t globalWorkSize[2];
    size_t localWorkSize[2];

	while (!done)
        {
            ticks = SDL_GetTicks();
            ++frameNumber;

            if( !(frameNumber % (FRAMES_PER_SECOND / 2) ) )
            {
                globalWorkSize[0] = obraz->w; ///TEMPORARY
                globalWorkSize[1] = obraz->h; ///TEMPORARY
                localWorkSize[0] = numLocalElements;
                localWorkSize[1] = numLocalElements;

                status = clEnqueueNDRangeKernel(cmdQueue, kernel, 2, NULL, globalWorkSize,
                                       localWorkSize, 0, NULL, NULL);
                if(status != CL_SUCCESS)
                {
                  printf("clEnqueueNDRangeKernel failed\n");
                  getchar();
                  exit(-16);
                }

                clEnqueueReadBuffer(cmdQueue, d_A, CL_TRUE, 0, rozmiar_wej*sizeof(cl_uint4), cl_piksele, 0, NULL, NULL);

                SDL_LockSurface(obraz);

                for(int i = 0; i < rozmiar_wej; i++)
                {
                    piksel = cl_piksele[i].s[0] | (cl_piksele[i].s[1] <<8) | (cl_piksele[i].s[2] << 16) | (cl_piksele[i].s[3] << 24);
                    piksele[i] = piksel;
                }

                SDL_UnlockSurface(obraz);

            }

            SDL_Flip(obraz);
            SDL_Flip(ekran);

            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    done = true;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) done = true;
                    break;
                }
            }
            // Message processing END

            printf("klatka %ld\n", frameNumber);
            if (1000/FRAMES_PER_SECOND > SDL_GetTicks() - ticks) SDL_Delay(1000/FRAMES_PER_SECOND-(SDL_GetTicks()-ticks));
        }


    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(d_A);
    clReleaseContext(context);

    free(cl_piksele);

    free(source);
    free(platforms);
    free(devices);

    SDL_FreeSurface(ekran);
    SDL_FreeSurface(obraz);
    SDL_Quit();
    printf("Exiting...\n");
    return 0;


}

char* readSource(const char *sourceFilename) {

   FILE *fp;
   int err;
   int size;

   char *source;

   fp = fopen(sourceFilename, "rb");
   if(fp == NULL) {
      printf("Could not open kernel file: %s\n", sourceFilename);
      exit(-129);
   }

   err = fseek(fp, 0, SEEK_END);
   if(err != 0) {
      printf("Error seeking to end of file\n");
      exit(-2156);
   }

   size = ftell(fp);
   if(size < 0) {
      printf("Error getting file position\n");
      exit(-23832);
   }

   err = fseek(fp, 0, SEEK_SET);
   if(err != 0) {
      printf("Error seeking to start of file\n");
      exit(-2372);
   }

   source = (char*)malloc(size+1);
   if(source == NULL) {
      printf("Error allocating %d bytes for the program source\n", size+1);
      exit(-6532);
   }

   err = fread(source, 1, size, fp);
   if(err != size) {
      printf("only read %d bytes\n", err);
      exit(0);
   }

   source[size] = '\0';

   return source;
}
