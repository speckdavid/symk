#ifndef REORDER_HEADER
#define REORDER_HEADER

#include<fstream>

using namespace std;

void shuffle(int* array, int start, int end)
{
	for(int i=start; i<end; i++) {
		int swap=i+rand()%(end-i+1);
		int value=array[i];
		array[i]=array[swap];
		array[swap]=value;
	}
}

void readOrderFile(char* elementFileName, int*& order, int numVariable)
{
	assert(order==NULL);

	ifstream in(elementFileName);
	const int MAX_LINE_LENGTH=10000;
	char lineBuffer[MAX_LINE_LENGTH];
	char literalBuffer[10];
	int x=0;
	int y=1;
	while(in.getline(lineBuffer, MAX_LINE_LENGTH)){
		if(lineBuffer[0]=='#'){
			if(lineBuffer[1]=='v'){
				// Number of variables
				int num=0;
				sscanf(&lineBuffer[3], "%d", &num);
				assert(num==numVariable);
			}
			else if(lineBuffer[1]=='s'){
				// Seed
			}
			else {
				exit(0);
			}
		}
		else{
			order=new int[numVariable+1];
			y=1;

			char* linePtr=lineBuffer;
			char* literalPtr=literalBuffer;
			while(*linePtr!='\0'){
				literalPtr = literalBuffer;
				while(*linePtr && isspace(*linePtr)){
					linePtr++;
				}
				while(*linePtr && !isspace(*linePtr)){
					*(literalPtr++) = *(linePtr++); // copy Literal
				}
				*literalPtr = '\0'; // terminate Literal

				if(strlen(literalBuffer)>0){
					order[y++] = atoi(literalBuffer);
				}
			}

			assert(y==numVariable+1);
			x++;
		}
	}

	assert(x==1);
}

#endif
