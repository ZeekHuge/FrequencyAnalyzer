//*************************************************************
//
//	NOTE: Only support LINUX os
//
//	This code is written by ZeekHuge, as an exercise to understand
//	and learn about the fft and using fftw library.
//
//	This program have been throughly tested for 
//	wav files on Ubuntu 14.04 os.
//	
//	
//	
//	Limitaions: 
//				1.Only .wav files supported
//				2.Can read only PCM 1 encrypted files
//				3.Only supports LINUX os
//				4.Do not supports more than 2 channels
//				5.Only supports 16 bit samples
//
//	Supported channels :
//				1.Single Channel .wav files
//				2.Dual channel .wav fles	 
//
//
//	To compile the program :
//				Execute the setup file as root.
//				setup is present in the parent directory.
//
//	Example : HOME/ID3Editor:$ sudo ./setup
//
//
// 	To Start the program : 	run the Frequency_Analyzer command as sudo
//				on your terminal/CMD with the file 
//				parameter
//	
//	Example: sudo Frequency_Analyzer fileToRead.wav (LINUX) 
//
//
//	Online Sources that helped:
//	https://en.wikipedia.org/wiki/Discrete_Fourier_transform#Definition
//	http://www.fftw.org/
//
//******************************************************************




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include <stdint.h>

typedef char _1byte;
typedef long int _4byte ;
typedef short _2byte;


//***********************************************************************************

#define here printf("------Here-------\n");

#define freeze printf("FReezed");getchar();getchar()

#define atPoint(value) fprintf(stderr, "At point %d\n",value )

#define read4byte(value,file)\
							value=0;\
							value = fgetc(file);value = value<<8*1;\
							value = value | (fgetc(file));value = value<<8*1;\
							value = value | (fgetc(file));value = value<<8*1;\
							value = value | (fgetc(file));\
							
#define read2byte(value,file)\
							value=0;\
							value = fgetc(file);value = value<<8*1;\
							value = value | (fgetc(file));

#define read1byte(value,file)\
							value = 0;\
							value = fgetc(file);

#define setOffset(value,file) fseek(file, value, SEEK_SET);	
					

//***********************************************************************************

#define _GET_FREQUENCY_LIST				1
#define _GET_PARTICULAR_FREQUENCY_		2
#define _FIRST_CHANNEL					1
#define _SECOND_CHANNEL					2
#define _ALREADY_SINLGE_CHANNEL			3
#define RIFF 							0x52494646
#define TWOcHANNELpCM 					0x01000200
#define SINGLEcHANNELpCM				0x01000100
#define BITSpERsECOND16 				0x1000
#define tempFileName					"others/tempFileDontDelete.txt"

//***********************************************************************************

//temporary global variables 
_4byte temp4byte;
_2byte temp2byte;
_1byte temp1byte;
_1byte waveChannel;



//***********************************************************************************

void clearScreen(){
	#ifdef __linux
	system("clear");
	#else
	system("cls");
	#endif
}

//***********************************************************************************

FILE *openFileSafely(char * addr, char* type){
	

	FILE *fl = fopen (addr,type);
	if (fl == NULL){
		printf("Couldn't open file %s.\n",addr);
		exit(0);
	}
	return (fl);
}

//***********************************************************************************

_1byte isSupported(FILE *fl){


	rewind(fl);
	read4byte(temp4byte,fl); 	if (temp4byte != RIFF) 				return 0;
	
	setOffset(20,fl);
	read4byte(temp4byte,fl);
	if 		(temp4byte == TWOcHANNELpCM    )		waveChannel = 2;
	else if (temp4byte == SINGLEcHANNELpCM )		waveChannel = 1;
	else									return 0;

	setOffset(34,fl);
	read2byte(temp2byte,fl); 	if (temp2byte!= BITSpERsECOND16)	return 0;

	//else
	return 1;
}


//***********************************************************************************


_4byte parseData(char outputFileName[], _4byte channel, FILE * inputFile ){

	FILE *outputFile = openFileSafely(outputFileName,"wb");
	char amplitude[10];
	long long int i = 0;

	setOffset(44,inputFile);

	if (channel == _SECOND_CHANNEL && !feof(inputFile)){
		printf("Parsing data ... please wait\n");
		read2byte(temp2byte, inputFile);
		while(!feof(inputFile)){

			read2byte(temp2byte,inputFile);
			//numberToString(amplitude,temp2byte);
			fprintf(outputFile, "%d\n",temp2byte );
			read2byte(temp2byte,inputFile);
			i++;
		}	

		fclose(outputFile);
		return i;   //returns number of terms
	}
	else if (channel == _FIRST_CHANNEL && !feof(inputFile)){
		printf("Parsing data ... please wait\n");
		while(!feof(inputFile)){
			read2byte(temp2byte,inputFile);
			//numberToString(amplitude,temp2byte);
			fprintf(outputFile, "%d\n",temp2byte );
			read2byte(temp2byte,inputFile);
			i++;	
		}

		fclose(outputFile);
		return i;	//returns number of terms
	}
	else if (channel == _ALREADY_SINLGE_CHANNEL && !feof(inputFile)){
		printf("Parsing data ... please wait\n");
		while(!feof(inputFile)){
			read2byte(temp2byte,inputFile);
			//numberToString(amplitude,temp2byte);
			fprintf(outputFile, "%d\n",temp2byte );
			read2byte(temp2byte,inputFile);
			i++;	
		}
		fclose(outputFile);
		return i;	//returns number of terms
	}
	else{
		printf("Wrong channel or an empty input file encountered.\n");
		fclose(outputFile);
		return 0;
	}
}

//***********************************************************************************



void getFrequencyDetails(int option, const unsigned long long int size ){

  	FILE *fl = openFileSafely(tempFileName,"rb");
  	fftw_complex *in = malloc (sizeof(fftw_complex)*size);
  	fftw_complex *in2 = malloc (sizeof(fftw_complex)*size);
  	fftw_complex *out = malloc (sizeof(fftw_complex)*size);
  	fftw_plan p, q;
	long long int i;

	printf("Analyzing ...... please wait\n");
  	for(i=0; i < size; i++){	
    	fscanf(fl,"%lf",&in[i][0]);
    	in[i][1]=0;
  	}

  	p = fftw_plan_dft_1d(size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  	fftw_execute(p);
  	
  	if (option == _GET_FREQUENCY_LIST){
  					
  		printf("The format of data will be this:\n %6s %18s   %18s   %10s     \npress a key to continue.","#freq", "#RealPart", "#ComplexPart", "#decibelValue" );		
  		getchar();getchar();
  		for (i = 0; i < size; i++){
    		printf(" %6lld %+18.5f   %+18.5f   %+10.4f     I\n",i, out[i][0], out[i][1], (10 * log((out[i][0]*out[i][0])+(out[i][1]*out[i][1])))/log(10) );
    	}
	}

	else if (option == _GET_PARTICULAR_FREQUENCY_){

		printf("What frequency shuold I print [0-%lld] ?\n",size);
		scanf(" %lld",&i);
		if (i < size){
			printf("%6s %18s   %18s   %10s     \n","freq", "RealPart", "ComplexPart", "decibelValue" );		
			printf("%6lld %+18.5f   %+18.5f   %+10.4f     \n",i, out[i][0], out[i][1], (10 * log((out[i][0]*out[i][0])+(out[i][1]*out[i][1])))/log(10) );		
		}
		else {
			printf("No such frequency.\n");
		}
	}
  	
  	fftw_destroy_plan(p);
  	fftw_cleanup();
  	free(in);
  	free(in2);
  	free(out);
  	fclose(fl);
}


//***********************************************************************************


int main(int argc, char *argv[]){

	FILE *fileToRead;
	char outputFileName[50];
	unsigned long long int termsInFile;
	freopen("others/logFile.log", "w+",stderr);

	if (argc == 1){
		printf("Specify a file please.\n");
		exit(0);	
	}
	if (strstr(argv[1],	".wav") == NULL){
		printf("Need a WAV file.\n");
		exit(0);		
	}

	fileToRead = openFileSafely(argv[1],"rb");

	if (isSupported(fileToRead)){

		printf("*****The file is supported\n");
		do {
			clearScreen();
			printf("Things that I can do:\n");
			printf("1.Parse one of the channels into an output file.\n");
			printf("2.Take out a particular frequency.\n");
			printf("3.Print out frequency list.\n");
			printf("What do you want me to do? [1-3] :");

			scanf(" %c",&temp1byte);

			switch (temp1byte){
				case '1' : 
						clearScreen();
						strcpy(outputFileName,argv[1]);
						strcpy(strstr(outputFileName,".wav"),"-parsedData.txt");
						if (waveChannel == 2){
							printf("Which channel do you want to parse ? left (1) or right (2)? [1-2] :");
							scanf(" %d",(int *)&temp4byte);	
							termsInFile = parseData(outputFileName, temp4byte, fileToRead);
							if(termsInFile){
								printf("Data parsed, output file name is %s\n",outputFileName);
							}
						}
						else{
							termsInFile = parseData(outputFileName, _ALREADY_SINLGE_CHANNEL, fileToRead);

							if(termsInFile){
								printf("Data parsed, output file name is %s\n",outputFileName);
							}
						}
				break;
				case '2' :
							clearScreen();
							if (waveChannel == 2){
								printf("*****I can analyze only one channel\n");
								printf("So, what channel should i analyze ?\n");
								printf("Left (1) or right (2) ? [1-2] :\n");
								scanf(" %d",(int *)&temp4byte);
							}
							else{
								temp4byte = _ALREADY_SINLGE_CHANNEL;
								/************/
								atPoint(11);
								/************/
							}
							termsInFile = parseData(tempFileName, temp4byte, fileToRead);
							getFrequencyDetails(_GET_PARTICULAR_FREQUENCY_, termsInFile);
				break;
				case '3' : 
							clearScreen();
							if (waveChannel == 2){
								printf("*****I can analyze only one channel\n");
								printf("So, what channel should i analyze ?\n");
								printf("Left (1) or right (2) ? [1-2] :\n");
								scanf(" %d",(int *)&temp4byte);
							}
							else{

								temp4byte = _ALREADY_SINLGE_CHANNEL;
							}
							termsInFile = parseData(tempFileName, temp4byte, fileToRead);
							getFrequencyDetails(_GET_FREQUENCY_LIST, termsInFile);

				break;
			}
			printf("Wanna try something else ? [y/n] :");
			scanf(" %c",&temp1byte);
		}while (temp1byte == 'y' || temp1byte == 'Y');
	}
	else{

		printf("Sorry the file is not supported\n");
	}

	fclose(fileToRead);
	return 0;
}
