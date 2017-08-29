#include <iostream>
#include <Windows.h>
#include <cstdio>
#include <stdio.h>
#include <strsafe.h>
#include <string>
#include <time.h>
#pragma warning(disable:4514)
#define MAX_STRING_LEN 256
using namespace std;
char getRandomChar() {
	char c = 'A' + (rand() % 24);
	return c;
}
typedef struct st
{
	bool bIsProcMatched;
	char Procname[MAX_STRING_LEN];
	int iTempOffset;
	st()
	{
		bIsProcMatched = false;
		strcpy(Procname,"");
		iTempOffset = 0;
	}
}PROCDETAIL;
int _GetName(char szName[])
{
	memset(szName, 0, sizeof(szName));

	int CPUInfo[4];
	__cpuid(CPUInfo, 0x80000002);
	memcpy_s(szName, sizeof(CPUInfo), CPUInfo, sizeof(CPUInfo));

	for (int i = 0; i<MAX_STRING_LEN; i++)
	{
		if (szName[i] == ':')
			szName[i] = ' ';
	}
	__cpuid(CPUInfo, 0x80000003);
	memcpy_s(szName + 16, sizeof(CPUInfo), CPUInfo, sizeof(CPUInfo));

	__cpuid(CPUInfo, 0x80000004);
	memcpy_s(szName + 32, sizeof(CPUInfo), CPUInfo, sizeof(CPUInfo));

	return 0;
}
enum ErrorCode
{
	eFileOpenError = -1,
	eSuccess,
	eFail,
	eInvalidArg,
	eFileDescriptorError
};

int Encode(char *sFileIn,char *sFileOut)
{
	char tmp[MAX_STRING_LEN] = {};
	srand((unsigned)time(0));
	PROCDETAIL stData;
	FILE *fIn, *fOut;
	DWORD dwAttrs;
	TCHAR szNewPath[MAX_STRING_LEN];
	string str;
	size_t j = 0;

	fIn = fopen(sFileIn, "r");
	if (!fIn)
	{
		printf("fopen: failed to open text file\n");
		return eFileOpenError;
	}
	fOut = fopen(sFileOut, "w");
	StringCchPrintf(szNewPath, sizeof(szNewPath) / sizeof(szNewPath[0]), TEXT("%s\\%s"), TEXT("."), TEXT("binfile.bin"));
	if (!fOut)
	{
		dwAttrs = GetFileAttributesA(sFileOut);
		if (dwAttrs == INVALID_FILE_ATTRIBUTES)
		{
			fclose(fIn);
			fclose(fOut);
			return eFileDescriptorError;
		}

		if ((dwAttrs | FILE_ATTRIBUTE_HIDDEN))
		{
			SetFileAttributes(szNewPath,
				FILE_ATTRIBUTE_NORMAL);
		}
		fOut = fopen(sFileOut,"w");
		if (!fOut)
		{
			printf("fopen: failed to open bin file\n");
			return eFileOpenError;
		}
	}
	
	while(EOF != (fscanf(fIn,"%s %d",&stData.Procname,&stData.iTempOffset)))
	{
		strcpy(tmp, stData.Procname);
		str = tmp;
		for (int i = 0; i<str.size(); i++)
		{
			if ((i % 2))
			{
				str.insert(i, 1, getRandomChar());
			}
		}
		strcpy(tmp,str.c_str());
		for (j = 0; j < strlen(tmp); j++)
		{
			stData.Procname[j] = tmp[j] + 3;
		}
		stData.Procname[j] = '\0';
		stData.iTempOffset *= 2;
		stData.iTempOffset += 1;
		fprintf(fOut,"%s %d ",stData.Procname,stData.iTempOffset);
	}	
//	StringCchPrintf(szNewPath, sizeof(szNewPath) / sizeof(szNewPath[0]), TEXT("%s\\%s"), TEXT("."), TEXT("binfile.bin"));

	dwAttrs = GetFileAttributesA(sFileOut);
	if (dwAttrs == INVALID_FILE_ATTRIBUTES)
	{
		fclose(fIn);
		fclose(fOut);
		return eFileDescriptorError;
	}

	if (!(dwAttrs & FILE_ATTRIBUTE_HIDDEN))
	{
		SetFileAttributes(szNewPath,
			dwAttrs | FILE_ATTRIBUTE_HIDDEN);
	}
	
	fclose(fIn);
	fclose(fOut);
	return eSuccess;
}
char* RemoveSpace(char* input)
{
	int i, j;
	char *pOut = input;
	for (i = 0, j = 0; i<strlen(input); i++, j++)
	{
		if (input[i] != ' ')
			pOut[j] = input[i];
		else
			j--;
	}
	pOut[j] = 0;
	return pOut;
}
int Decode(const char *FileName,PROCDETAIL &stResult)
{
	char tmp[MAX_STRING_LEN] = {};
	char ch;
	size_t j = 0;
	PROCDETAIL stData;
	string str, strFinal;
	char szName[MAX_STRING_LEN] = {};
	FILE *fIn = fopen(FileName, "r");
	if (!fIn)
	{
		printf("Failed to open bin file\n");
		return eFileOpenError;
	}
	_GetName(szName);
	char *ptr = RemoveSpace(szName);
	while (EOF != (fscanf(fIn, "%s %d ", &stData.Procname, &stData.iTempOffset)))
	{
		j = 0;
		strcpy(tmp, stData.Procname);
		str = stData.Procname;
		for (size_t i = 0; i < str.size(); i++)
		{
			if (!(i%2))
			{
				strFinal.insert(j,&str.at(i));
				j++;
			}
		}
		strFinal.resize(j);

		strcpy(tmp,strFinal.c_str());
		for (j = 0; j < strlen(tmp); j++)
		{
			stResult.Procname[j] = tmp[j] - 3;
		}
		stResult.Procname[j] = '\0';
		stData.iTempOffset -= 1;
		stData.iTempOffset /= 2;
		
		if (strstr(ptr,stResult.Procname))
		{
			stResult.bIsProcMatched = true;
			stResult.iTempOffset=stData.iTempOffset;
			break;
		}
	}
	fclose(fIn);
	return eSuccess;
}


int main(int argc,char **argv)
{
	PROCDETAIL stResult;
	char *sFileIn = "textfile.txt";
	char *sFileOut = "binfile.bin";
	if (argc!=2)
	{
		printf("Invalid argument!\nUsage :\n\t EncodeDecode.exe.exe -e or <AppName>.exe -d\n ");
		return eInvalidArg;
	}
	if (!strcmp("-e" ,argv[1]))
	{
		if (Encode(sFileIn, sFileOut))
		{
			cout << "Encode API: Failure" << endl;
		}
	}
	else if (!strcmp("-d" ,argv[1]))
	{
		if (!Decode(sFileOut, stResult))
		{
			if (stResult.bIsProcMatched)
			{
				cout << "Match found!! : Offset :" << stResult.iTempOffset << endl;
			}
			else
			{
				cout << "Match not found!!" << endl;
			}
		}
		else
		{
			cout << "Decode API: Failure" << endl;
		}
	}
	else
	{
		printf("Invalid input\n");
	}
	return 0;
}
