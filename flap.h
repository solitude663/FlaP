#ifndef FLAP_H
#define FLAP_H

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#if __cplusplus
extern "C" {
#endif
enum FlapType
{
	Flap_Int,
	Flap_Str,
	Flap_Bool,
};

typedef struct
{	
	int Type;

	char* ShortName;
	char* LongName;
	char* Description;

	intptr_t Val;
	int IsPtr;

} FlapArg;

#define FLAP_ARG_CAP 128
typedef struct
{
	int ArgCount;
	FlapArg Args[FLAP_ARG_CAP];
} FlagParserState;

int*	FlapInt(char* shortName, char* longName, char* description, int defaultVal);
char**	FlapString(char* shortName, char* longName, char* description, char* defaultVal);
int*	FlapBool(char* shortName, char* longName, char* description, int defaultVal);

void	FlapIntVal(int* ptr, char* shortName, char* longName, char* description, int defaultVal);
char**	FlapStringVal(char** ptr, char* shortName, char* longName, char* description, char* defaultVal);
int*	FlapBoolVal(int* ptr, char* shortName, char* longName, char* description, int defaultVal);

int FlapParse(int flagCount, char** flags);

void FlapReportErrors();
void FlapPrintInfo();

#if __cplusplus
}
#endif
 
#ifdef FLAP_IMPLEMENTATION

static FlagParserState FlapState;

static FlapArg* FlapNewArg(int type, char* shortName, char* longName, char* description)
{
	assert(FlapState.ArgCount < FLAP_ARG_CAP);
	
	FlapArg* arg = &(FlapState.Args[FlapState.ArgCount++]);
	arg->Type = type;
	arg->ShortName = shortName;
	arg->LongName = longName;
	arg->Description = description;
	return arg;
}

int* FlapInt(char* shortName, char* longName, char* description, int defaultVal)
{
	FlapArg* arg = FlapNewArg(Flap_Int, shortName, longName, description);
	arg->Val = defaultVal;
	return (int*)&arg->Val;
}

void FlapIntVal(int* ptr, char* shortName, char* longName, char* description, int defaultVal)
{
	FlapArg* arg = FlapNewArg(Flap_Int, shortName, longName, description);
	arg->IsPtr = 1;
	*ptr = defaultVal;
	arg->Val = (intptr_t)ptr;
}

char** FlapString(char* shortName, char* longName, char* description, char* defaultVal)
{
	FlapArg* arg = FlapNewArg(Flap_Str, shortName, longName, description);
	arg->Val = (intptr_t)defaultVal;
	return (char**)&arg->Val;
}

char** FlapStringVal(char** ptr, char* shortName, char* longName, char* description, char* defaultVal)
{
	FlapArg* arg = FlapNewArg(Flap_Str, shortName, longName, description);
	arg->IsPtr = 1;
	*ptr = defaultVal;
	arg->Val = (intptr_t)ptr;
}

int* FlapBool(char* shortName, char* longName, char* description, int defaultVal)
{
	FlapArg* arg = FlapNewArg(Flap_Bool, shortName, longName, description);
	arg->Val = defaultVal;
	return (int*)&arg->Val;
}

int* FlapBoolVal(int* ptr, char* shortName, char* longName, char* description, int defaultVal)
{
	FlapArg* arg = FlapNewArg(Flap_Bool, shortName, longName, description);
	arg->IsPtr = 1;
	*ptr = defaultVal;
	arg->Val = (intptr_t)ptr;
}


void FlapReportErrors()
{
	assert(0 && "Implement");
}

void FlapPrintInfo()
{
	int argCount = FlapState.ArgCount;

	for(int i = 0; i < argCount; i++)
	{
		FlapArg* arg = &FlapState.Args[i];
		printf("-%s\t--%s\n", arg->ShortName, arg->LongName);
		printf("\t\t%s\n\n", arg->Description);
	}
}

static int FlagMatches(FlapArg* arg, char* flag)
{
	int flagLen = strlen(flag);
	if(flagLen < 2) assert(0 && "Flag length has to be > 2");

	if(flag[0] == '-' && flag[1] == '-')
	{		
		return strcmp(arg->LongName, (flag+2)) == 0;
	}
	else if(flag[0] == '-')
	{
		return strcmp(arg->ShortName, (flag+1)) == 0;
	}

	return 0;
}

static int FlapIsBool(char* test, int* value)
{
	if(strcmp(test, "0") == 0  || strcmp(test, "false") == 0 || strcmp(test, "False") == 0||
	   strcmp(test, "FALSE") == 0 || strcmp(test, "F") == 0|| strcmp(test, "f") == 0)
	{
		*value = 0;
		return 1;
	}

	if(strcmp(test, "1") == 0  || strcmp(test, "true") == 0 || strcmp(test, "True") == 0||
	   strcmp(test, "TRUE") == 0 || strcmp(test, "T") == 0|| strcmp(test, "t") == 0)
	{
		*value = 1;
		return 1;
	}

	return 0;
}

// TODO(afb) :: Different bool parsing techniques. -b True | -b=1
int FlapParse(int flagCount, char** flags)
{
	int argCount = FlapState.ArgCount;

	int passed = 1;
	
	for(int flagIndex = 1; flagIndex < flagCount; flagIndex++)
	{
		char* flag = flags[flagIndex];

		passed = flagIndex;
		if(strlen(flag) > 0 && flag[0] != '-')
		{
			break;
		}

		for(int i = 0; i < argCount; i++)
		{
			FlapArg* arg = &FlapState.Args[i];
			
			if(!FlagMatches(arg, flag))
			{
				continue;
			}
			
			switch(arg->Type)
			{
				case(Flap_Bool):
				{
					int value = 1;
					if((flagIndex + 1) < flagCount)
					{
						char* possibleValue = flags[flagIndex+1];
						if(!FlapIsBool(possibleValue, &value)) value = 1;
						else flagIndex++;
					}
					
					if(arg->IsPtr) *(int*)arg->Val = value;
					else *(int*)&arg->Val = value;
				}break;

				case(Flap_Str):
				{
					assert((flagIndex+1 < flagCount) && "No string passed");

					if(arg->IsPtr) *(char**)arg->Val = flags[++flagIndex];
					else *(char**)&arg->Val = flags[++flagIndex];
				}break;

				case(Flap_Int):
				{
					assert((flagIndex+1 < flagCount) && "No int passed");
					char* number = flags[++flagIndex];
					int value = atoi(number);

					if(value == 0 && strlen(number) > 1) assert(0 && "Invalid integer");
					if(value == 0 && number[0] != '0') assert(0 && "Invalid integer");

					if(arg->IsPtr) *(int*)arg->Val = value;
					else *(int*)&arg->Val = value;
				}break;
								
				default:
				{
					assert(0 && "Unhandled flag type");
				}
			}

			break;
		}
	}
		
	return passed;
}

#endif // FLAP_IMPLEMENTATION

#endif // FLAP_H
