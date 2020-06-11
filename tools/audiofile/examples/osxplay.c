/*
	Audio File Library

	Copyright (c) 2003, Michael Pruett.  All rights reserved.

	Redistribution and use in source and binary forms, with or
	without modification, are permitted provided that the following
	conditions are met:

	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above
	copyright notice, this list of conditions and the following
	disclaimer in the documentation and/or other materials provided
	with the distribution.

	3. The name of the author may not be used to endorse or promote
	products derived from this software without specific prior
	written permission.

	THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
	osxplay.c

	This program demonstrates audio file playback using the Audio
	File Library and Core Audio.
*/

#include <audiofile.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

#define BUFFER_FRAME_COUNT 1024

int isPlaying = 1;
void *buffer = NULL;

void getASBDForFile (AFfilehandle file, int track,
	AudioStreamBasicDescription *asbd)
{
	int	sampleFormat, sampleWidth, channelCount;
	double	rate;

	afGetVirtualSampleFormat(file, track, &sampleFormat, &sampleWidth);
	channelCount = afGetChannels(file, track);
	rate = afGetRate(file, track);

	asbd->mSampleRate = rate;
	asbd->mFormatID = kAudioFormatLinearPCM;
	switch (sampleFormat)
	{
		case AF_SAMPFMT_TWOSCOMP:
			asbd->mFormatFlags = kAudioFormatFlagIsSignedInteger;
			asbd->mBitsPerChannel = sampleWidth;
			break;
		case AF_SAMPFMT_UNSIGNED:
			asbd->mFormatFlags = 0;
			asbd->mBitsPerChannel = sampleWidth;
			break;
		case AF_SAMPFMT_FLOAT:
			asbd->mFormatFlags = kAudioFormatFlagIsFloat;
			asbd->mBitsPerChannel = 32;
			break;
		case AF_SAMPFMT_DOUBLE:
			asbd->mFormatFlags = kAudioFormatFlagIsFloat;
			asbd->mBitsPerChannel = 64;
			break;
	}

	asbd->mChannelsPerFrame = channelCount;
	asbd->mFramesPerPacket = 1;
	asbd->mBytesPerFrame = ceilf(afGetVirtualFrameSize(file, track, 1));
	asbd->mBytesPerPacket = asbd->mBytesPerFrame;

	if (afGetVirtualByteOrder(file, track) == AF_BYTEORDER_BIGENDIAN)
		asbd->mFormatFlags |= kAudioFormatFlagIsBigEndian;
}

OSStatus openOutput (AudioUnit *outputUnit)
{
	OSStatus		status = noErr;
	ComponentDescription	description;
	Component		component;

	description.componentType = kAudioUnitType_Output;
	description.componentSubType = kAudioUnitSubType_DefaultOutput;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;

	component = FindNextComponent(NULL, &description);
	if (component == NULL)
	{
		fprintf(stderr, "Could not find audio output device.\n");
		exit(EXIT_FAILURE);
	}

	status = OpenAComponent(component, outputUnit);
	if (status != noErr)
	{
		fprintf(stderr, "Could not open audio output device.\n");
		exit(EXIT_FAILURE);
	}

	status = AudioUnitInitialize(*outputUnit);
	if (status != noErr)
	{
		fprintf(stderr, "Could not initialize audio output device.\n");
		exit(EXIT_FAILURE);
	}

	return status;
}

OSStatus fileRenderProc (void *inRefCon,
	AudioUnitRenderActionFlags *inActionFlags,
	const AudioTimeStamp *inTimeStamp,
	UInt32 inBusNumber,
	UInt32 inNumFrames,
	AudioBufferList *ioData)
{
	AFfilehandle	file = (AFfilehandle) inRefCon;
	AFframecount	framesToRead, framesRead;

	framesToRead = inNumFrames;
	if (framesToRead > BUFFER_FRAME_COUNT)
		framesToRead = BUFFER_FRAME_COUNT;

	framesRead = afReadFrames(file, AF_DEFAULT_TRACK,
		buffer, framesToRead);
	if (framesRead > 0)
	{
		ioData->mBuffers[0].mData = buffer;
		ioData->mBuffers[0].mDataByteSize = framesRead *
			afGetVirtualFrameSize(file, AF_DEFAULT_TRACK, 1);
	}
	else
		isPlaying = 0;

	return noErr;
}

OSStatus setupOutput (AudioUnit *outputUnit, AFfilehandle file)
{
	OSStatus	status = noErr;
	UInt32		size;
	Boolean		outWritable;

	AudioStreamBasicDescription	fileASBD, inputASBD, outputASBD;
	AURenderCallbackStruct		renderCallback;

	/* Set virtual sample format to single-precision floating-point. */
	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_FLOAT, 32);

	/* Get ASBD for virtual sample format. */ 
	getASBDForFile(file, AF_DEFAULT_TRACK, &fileASBD);

	status = AudioUnitGetPropertyInfo(*outputUnit,
		kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output,
		0, &size, &outWritable);

	status = AudioUnitGetProperty(*outputUnit,
		kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output,
		0, &outputASBD, &size);

	if (outWritable)
	{
		outputASBD = fileASBD;

		status = AudioUnitSetProperty(*outputUnit,
			kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output,
			0, &outputASBD, size);
	}

	inputASBD = fileASBD;

	status = AudioUnitSetProperty(*outputUnit,
		kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
		0, &inputASBD, size);
	if (status != noErr)
	{
		fprintf(stderr, "Could not set input stream format.\n");
		exit(EXIT_FAILURE);
	}

	/*
		Set the render callback to a procedure which will
		read from the file.
	*/
	renderCallback.inputProc = fileRenderProc;
	renderCallback.inputProcRefCon = file;

	status = AudioUnitSetProperty(*outputUnit,
		kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0,
		&renderCallback, sizeof (AURenderCallbackStruct));
	if (status != noErr)
	{
		fprintf(stderr, "Could not set render callback.\n");
		exit(EXIT_FAILURE);
	}

	return status;
}

int main (int argc, char **argv)
{
	AFfilehandle	file;
	AudioUnit	outputUnit;

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	file = afOpenFile(argv[1], "r", AF_NULL_FILESETUP);
	if (file == AF_NULL_FILEHANDLE)
	{
		fprintf(stderr, "Could not open file '%s' for reading.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	openOutput(&outputUnit);
	setupOutput(&outputUnit, file);
	AudioOutputUnitStart(outputUnit);

	buffer = malloc(BUFFER_FRAME_COUNT *
		afGetVirtualFrameSize(file, AF_DEFAULT_TRACK, 1));

	while (isPlaying)
		usleep(250000);

	AudioOutputUnitStop(outputUnit);
	AudioUnitUninitialize(outputUnit);
	CloseComponent(outputUnit);

	free(buffer);

	afCloseFile(file);
}
