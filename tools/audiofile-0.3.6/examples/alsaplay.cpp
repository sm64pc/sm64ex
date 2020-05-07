/*
	Audio File Library

	Copyright (c) 2010, Michael Pruett.  All rights reserved.

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

#include <alsa/asoundlib.h>
#include <audiofile.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	AFfilehandle file = afOpenFile(argv[1], "r", AF_NULL_FILESETUP);
	if (!file)
	{
		fprintf(stderr, "Could not open '%s'.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	int channels = afGetChannels(file, AF_DEFAULT_TRACK);
	double rate = afGetRate(file, AF_DEFAULT_TRACK);
	afSetVirtualSampleFormat(file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);

	int err;
	snd_pcm_t *handle;
	if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		fprintf(stderr, "Could not open audio output: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	if ((err = snd_pcm_set_params(handle, SND_PCM_FORMAT_S16,
		SND_PCM_ACCESS_RW_INTERLEAVED, channels, rate, 1, 500000)) < 0)
	{
		fprintf(stderr, "Could not set audio output parameters: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	const int bufferFrames = 4096;
	int16_t *buffer = new int16_t[bufferFrames * channels];

	while (true)
	{
		AFframecount framesRead = afReadFrames(file, AF_DEFAULT_TRACK, buffer, bufferFrames);
		if (framesRead <= 0)
			break;

		snd_pcm_sframes_t framesWritten = snd_pcm_writei(handle, buffer, bufferFrames);
		if (framesWritten < 0)
			framesWritten = snd_pcm_recover(handle, framesWritten, 0);
		if (framesWritten < 0)
		{
			fprintf(stderr, "Could not write audio data to output device: %s\n",
				snd_strerror(err));
			break;
		}
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	delete [] buffer;

	afCloseFile(file);

	return 0;
}
