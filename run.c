/*
 * @author Ashish K
 */
/*
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/*						ACQUIRED PACKET  REPRENTATION

	------------------------------------------------------------------------------------------------------------
	| SAMPLE 1 | SAMPLE 2 | SAMPLE 3 | ... | SAMPLE 99 | SAMPLE 100|     ....    ..... | SAMPLE 99 | SAMPLE 200 |
	------------------------------------------------------------------------------------------------------------ 

	|<- -- -- -- -- -- -- -- -- FRAME 1 -- -- -- -- -- -- -- -- -->|< -- -- -- -- -- FRAME 2 -- -- -- -- -- -- ->|

 	* All samples are 32 bit Float and 100 samples make a FRAME.
	* The samples can be typecasted to any datatype.
 	* Each frames can be piped to another process for real-time processing.

*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "portaudio.h"

#define TOTAL_NUM_OF_SAMPLES 200
#define FILE_WRITE_RECORDED (1)

typedef float SAMPLE;

PaStream *stream;
const int samplesPerFrame = 100;	// 100 samples make a Frame, this frame will be sent to pipe one by one

int sampleRate = 8000;
int numChannels = 1;
int totalFrames;

PaSampleFormat sampleFormat;

int finished, i;
int bytesPerSample;

PaError err = paNoError;

typedef struct{

	SAMPLE *recordedSample;
	unsigned int sampleIndex;
	unsigned int frameIndex;
}
paTestData;

static int paStreamCallback( const void* input, void* output,
	unsigned long samplesPerFrame,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData ) {

	paTestData *data = (paTestData*) userData;
	const SAMPLE *readPtr = (const SAMPLE*)input;	// Casting input read to valid input type SAMPLE (float)
//	SAMPLE *writePtr = &data->recordedSample[data->sampleIndex];
	unsigned long totalSamples = TOTAL_NUM_OF_SAMPLES ;	// Using maxFrameIndex and frameIndex pointers of the structure data

	(void) output;		// Preventing unused variable warnings
	(void) timeInfo;
	(void) statusFlags;

	static int count = 1;	// Count: here mechanism for frame detection

	/* Case which acquires frame */
	if(data->sampleIndex < totalSamples && data->sampleIndex != (count * samplesPerFrame)){
		printf("Read Pointer : %.8f\n", *readPtr);	// Or do whatever you want to do man
		printf("SAMPLE INDEX: %d\n", data->sampleIndex++);
	}

	/* Running PaContinue after each frame acquired */
	else if(data->sampleIndex == (count++ * samplesPerFrame) && data->sampleIndex != totalSamples){
		
		printf("Frame index number: %d\n", ++data->frameIndex);
		finished = paContinue;
	}

	/* PaComplete after 200 samples (This will be infinite) */
	else if(data->sampleIndex == totalSamples){
		printf("\n---------Streaming Finished!---------\n");
		finished = paComplete;
	}

	return finished;
}		 

int main(void);
int main(void){

	int numBytes;

	paTestData data;	// Object of paTestData structure

	fflush(stdout);

	data.sampleIndex = 0;
	data.frameIndex = 1;

	numBytes = TOTAL_NUM_OF_SAMPLES * sizeof(SAMPLE);
	data.recordedSample = (SAMPLE*)malloc(numBytes);	// Memory allocation here (This has to be changed for all time recording)

	if(data.recordedSample == NULL){
		printf("Malloc failed to allocate recordedSample\n");
		goto done;
	}

	for(i=0; i < TOTAL_NUM_OF_SAMPLES; i++)
		data.recordedSample[i] = 0;	// Initialising recordedSample

	err = Pa_Initialize();
	if( err!= paNoError)
		goto done;

        PaStreamParameters inputParameter;

        inputParameter.device = Pa_GetDefaultInputDevice();
        if(inputParameter.device == paNoDevice){
		fprintf(stderr, "ERROR: No input device found.\n");
		goto done;
	}

        /* Input Parameter structure definition here */
        inputParameter.channelCount = numChannels;
        inputParameter.sampleFormat = paFloat32;
        inputParameter.suggestedLatency = Pa_GetDeviceInfo(inputParameter.device)->defaultHighOutputLatency;
        inputParameter.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(

		&stream,
		&inputParameter,
		NULL,   			// *outputParameter Not needed here
		sampleRate,
		samplesPerFrame,
		paClipOff,                      // All flags OFF
		paStreamCallback,
		&data
	);

	if(err != paNoError)
		goto done;

	err = Pa_StartStream(stream);
	if(err != paNoError)
		goto done;

	/* Recording audio here..Speak into the MIC */
	printf("\nRecording...\n");
	fflush(stdout);

	while((err = Pa_IsStreamActive(stream)) == 1)
		Pa_Sleep(1000);

	if(err < 0)
                goto done;

        err = Pa_CloseStream(stream);
	if(err != paNoError)
		goto done;

	#if FILE_WRITE_RECORDED
		{
			FILE *fid;
			fid = fopen("record.raw", "wb");

			if(fid == NULL)
				printf("\nCould't open file record.raw\n");
			else{
				fwrite(data.recordedSample, sizeof(SAMPLE), totalFrames, fid);
				fclose(fid);
				printf("\nData written successfully to record.wav...\n");
			}
		}
	#endif
	printf("\nTotal number of samples acquired : %d\n", data.sampleIndex);
	printf("\nRecord program finished successfully!\n");

	/* Error handling here */
	done:

		Pa_Terminate();
		if( data.recordedSample )
			free( data.recordedSample );

		if( err != paNoError ){
			fprintf( stderr, "An error occured while using the portaudio stream\n" );
			fprintf( stderr, "Error number: %d\n", err );
			fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
			err = 1;
		}

	return err;
}
