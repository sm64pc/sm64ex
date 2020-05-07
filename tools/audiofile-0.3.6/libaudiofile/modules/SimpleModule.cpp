/*
	Audio File Library
	Copyright (C) 2010, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA  02110-1301  USA
*/

#include "config.h"
#include "SimpleModule.h"

#include <algorithm>

void SimpleModule::runPull()
{
	pull(m_outChunk->frameCount);
	run(*m_inChunk, *m_outChunk);
}

void SimpleModule::runPush()
{
	m_outChunk->frameCount = m_inChunk->frameCount;
	run(*m_inChunk, *m_outChunk);
	push(m_outChunk->frameCount);
}

ApplyChannelMatrix::ApplyChannelMatrix(FormatCode format, bool isReading,
	int inChannels, int outChannels,
	double minClip, double maxClip, const double *matrix) :
	m_format(format),
	m_inChannels(inChannels),
	m_outChannels(outChannels),
	m_minClip(minClip),
	m_maxClip(maxClip),
	m_matrix(NULL)
{
	m_matrix = new double[m_inChannels * m_outChannels];
	if (matrix)
	{
		if (isReading)
		{
			// Copy channel matrix for reading.
			std::copy(matrix, matrix + m_inChannels * m_outChannels, m_matrix);
		}
		else
		{
			// Transpose channel matrix for writing.
			for (int i=0; i < inChannels; i++)
				for (int j=0; j < outChannels; j++)
					m_matrix[j*inChannels + i] = matrix[i*outChannels + j];
		}
	}
	else
	{
		initDefaultMatrix();
	}
}

ApplyChannelMatrix::~ApplyChannelMatrix()
{
	delete [] m_matrix;
}

const char *ApplyChannelMatrix::name() const { return "channelMatrix"; }

void ApplyChannelMatrix::describe()
{
	m_outChunk->f.channelCount = m_outChannels;
	m_outChunk->f.pcm.minClip = m_minClip;
	m_outChunk->f.pcm.maxClip = m_maxClip;
}

void ApplyChannelMatrix::run(Chunk &inChunk, Chunk &outChunk)
{
	switch (m_format)
	{
		case kInt8:
			run<int8_t>(inChunk.buffer, outChunk.buffer, inChunk.frameCount);
			break;
		case kInt16:
			run<int16_t>(inChunk.buffer, outChunk.buffer, inChunk.frameCount);
			break;
		case kInt24:
		case kInt32:
			run<int32_t>(inChunk.buffer, outChunk.buffer, inChunk.frameCount);
			break;
		case kFloat:
			run<float>(inChunk.buffer, outChunk.buffer, inChunk.frameCount);
			break;
		case kDouble:
			run<double>(inChunk.buffer, outChunk.buffer, inChunk.frameCount);
			break;
		default:
			assert(false);
	}
}

template <typename T>
void ApplyChannelMatrix::run(const void *inputData, void *outputData, int frameCount)
{
	const T *input = reinterpret_cast<const T *>(inputData);
	T *output = reinterpret_cast<T *>(outputData);
	for (int frame=0; frame<frameCount; frame++)
	{
		const T *inputSave = input;
		const double *m = m_matrix;
		for (int outChannel=0; outChannel < m_outChannels; outChannel++)
		{
			input = inputSave;
			double t = 0;
			for (int inChannel=0; inChannel < m_inChannels; inChannel++)
				t += *input++ * *m++;
			*output++ = t;
		}
	}
}

void ApplyChannelMatrix::initDefaultMatrix()
{
	const double *matrix = NULL;
	if (m_inChannels==1 && m_outChannels==2)
	{
		static const double m[]={1,1};
		matrix = m;
	}
	else if (m_inChannels==1 && m_outChannels==4)
	{
		static const double m[]={1,1,0,0};
		matrix = m;
	}
	else if (m_inChannels==2 && m_outChannels==1)
	{
		static const double m[]={.5,.5};
		matrix = m;
	}
	else if (m_inChannels==2 && m_outChannels==4)
	{
		static const double m[]={1,0,0,1,0,0,0,0};
		matrix = m;
	}
	else if (m_inChannels==4 && m_outChannels==1)
	{
		static const double m[]={.5,.5,.5,.5};
		matrix = m;
	}
	else if (m_inChannels==4 && m_outChannels==2)
	{
		static const double m[]={1,0,1,0,0,1,0,1};
		matrix = m;
	}

	if (matrix)
	{
		std::copy(matrix, matrix + m_inChannels * m_outChannels, m_matrix);
	}
	else
	{
		for (int i=0; i < m_inChannels; i++)
			for (int j=0; j < m_outChannels; j++)
				m_matrix[j*m_inChannels + i] = (i==j) ? 1 : 0;
	}
}
