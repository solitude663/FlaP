#include <stdio.h>

#define FLAP_IMPLEMENTATION
#include "flap.h"

int main(int argc, char** argv)
{
	int* threadCount		= FlapInt("tc", "thread_count", "The amount of threads to use. Must be > 0", 1);
	char** buildType		= FlapString("bt", "build_type", "Type of build to be done. Can be [debug], [dev], [release]", "dev");
	int* outputExecutable	= FlapBool("o", "output_exe", "Output a executable file.", 1);
	int* help               = FlapBool("h", "help", "Show possible arguments.", 0);
	
	int passed = FlapParse(argc, argv);	
	
	if(*help)
	{
		FlapPrintInfo();
		return 0;
	}
	
	printf("Build Type: %s\n", *buildType);
	printf("Thread Count: %d\n", *threadCount);
	printf("Output Executable: %s\n", *outputExecutable ? "true" : "false");
	printf("Filename: %s\n", argv[passed]);
	
	return 0;
}
