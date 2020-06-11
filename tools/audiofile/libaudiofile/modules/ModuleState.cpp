/*
	Audio File Library
	Copyright (C) 2000, Silicon Graphics, Inc.
	Copyright (C) 2010-2013, Michael Pruett <michael@68k.org>

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
#include "ModuleState.h"

#include "File.h"
#include "FileHandle.h"
#include "FileModule.h"
#include "RebufferModule.h"
#include "SimpleModule.h"
#include "Track.h"
#include "byteorder.h"
#include "compression.h"
#include "units.h"
#include "util.h"
#include "../pcm.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <stdio.h>

ModuleState::ModuleState() :
	m_isDirty(true)
{
}

ModuleState::~ModuleState()
{
}

status ModuleState::initFileModule(AFfilehandle file, Track *track)
{
	const CompressionUnit *unit = _af_compression_unit_from_id(track->f.compressionType);
	if (!unit)
		return AF_FAIL;

	// Validate compression format and parameters.
	if (!unit->fmtok(&track->f))
		return AF_FAIL;

	if (file->m_seekok &&
		file->m_fh->seek(track->fpos_first_frame, File::SeekFromBeginning) !=
			track->fpos_first_frame)
	{
		_af_error(AF_BAD_LSEEK, "unable to position file handle at beginning of sound data");
		return AF_FAIL;
	}

	AFframecount chunkFrames;
	if (file->m_access == _AF_READ_ACCESS)
		m_fileModule = unit->initdecompress(track, file->m_fh, file->m_seekok,
			file->m_fileFormat == AF_FILE_RAWDATA, &chunkFrames);
	else
		m_fileModule = unit->initcompress(track, file->m_fh, file->m_seekok,
			file->m_fileFormat == AF_FILE_RAWDATA, &chunkFrames);

	if (unit->needsRebuffer)
	{
		assert(unit->nativeSampleFormat == AF_SAMPFMT_TWOSCOMP);

		RebufferModule::Direction direction =
			file->m_access == _AF_WRITE_ACCESS ?
				RebufferModule::VariableToFixed : RebufferModule::FixedToVariable;

		m_fileRebufferModule = new RebufferModule(direction,
			track->f.bytesPerFrame(false), chunkFrames,
			unit->multiple_of);
	}

	track->filemodhappy = true;

	return AF_SUCCEED;
}

status ModuleState::init(AFfilehandle file, Track *track)
{
	if (initFileModule(file, track) == AF_FAIL)
		return AF_FAIL;

	return AF_SUCCEED;
}

bool ModuleState::fileModuleHandlesSeeking() const
{
	return m_fileModule->handlesSeeking();
}

status ModuleState::setup(AFfilehandle file, Track *track)
{
	AFframecount fframepos = llrint(track->nextvframe * track->f.sampleRate / track->v.sampleRate);
	bool isReading = file->m_access == _AF_READ_ACCESS;

	if (!track->v.isUncompressed())
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED,
			"library does not support compression in virtual format yet");
		return AF_FAIL;
	}

	if (arrange(file, track) == AF_FAIL)
		return AF_FAIL;

	track->filemodhappy = true;
	int maxbufsize = 0;
	if (isReading)
	{
		m_chunks.back()->frameCount = _AF_ATOMIC_NVFRAMES;
		for (int i=m_modules.size() - 1; i >= 0; i--)
		{
			SharedPtr<Chunk> inChunk = m_chunks[i];
			SharedPtr<Chunk> outChunk = m_chunks[i+1];
			int bufsize = outChunk->frameCount * outChunk->f.bytesPerFrame(true);
			if (bufsize > maxbufsize)
				maxbufsize = bufsize;
			if (i != 0)
				m_modules[i]->setSource(m_modules[i-1].get());
			m_modules[i]->maxPull();
		}

		if (!track->filemodhappy)
			return AF_FAIL;
		int bufsize = m_fileModule->bufferSize();
		if (bufsize > maxbufsize)
			maxbufsize = bufsize;
	}
	else
	{
		m_chunks.front()->frameCount = _AF_ATOMIC_NVFRAMES;
		for (size_t i=0; i<m_modules.size(); i++)
		{
			SharedPtr<Chunk> inChunk = m_chunks[i];
			SharedPtr<Chunk> outChunk = m_chunks[i+1];
			int bufsize = inChunk->frameCount * inChunk->f.bytesPerFrame(true);
			if (bufsize > maxbufsize)
				maxbufsize = bufsize;
			if (i != m_modules.size() - 1)
				m_modules[i]->setSink(m_modules[i+1].get());
			m_modules[i]->maxPush();
		}

		if (!track->filemodhappy)
			return AF_FAIL;

		int bufsize = m_fileModule->bufferSize();
		if (bufsize > maxbufsize)
			maxbufsize = bufsize;
	}

	for (size_t i=0; i<m_chunks.size(); i++)
	{
		if ((isReading && i==m_chunks.size() - 1) || (!isReading && i==0))
			continue;
		m_chunks[i]->allocate(maxbufsize);
	}

	if (isReading)
	{
		if (track->totalfframes == -1)
			track->totalvframes = -1;
		else
			track->totalvframes = llrint(track->totalfframes *
				(track->v.sampleRate / track->f.sampleRate));

		track->nextfframe = fframepos;
		track->nextvframe = llrint(fframepos * track->v.sampleRate / track->f.sampleRate);

		m_isDirty = false;

		if (reset(file, track) == AF_FAIL)
			return AF_FAIL;
	}
	else
	{
		track->nextvframe = track->totalvframes =
			(AFframecount) (fframepos * track->v.sampleRate / track->f.sampleRate);
		m_isDirty = false;
	}

	return AF_SUCCEED;
}

const std::vector<SharedPtr<Module> > &ModuleState::modules() const
{
	return m_modules;
}

const std::vector<SharedPtr<Chunk> > &ModuleState::chunks() const
{
	return m_chunks;
}

status ModuleState::reset(AFfilehandle file, Track *track)
{
	track->filemodhappy = true;
	for (std::vector<SharedPtr<Module> >::reverse_iterator i=m_modules.rbegin();
			i != m_modules.rend(); ++i)
		(*i)->reset1();
	track->frames2ignore = 0;
	if (!track->filemodhappy)
		return AF_FAIL;
	for (std::vector<SharedPtr<Module> >::iterator i=m_modules.begin();
			i != m_modules.end(); ++i)
		(*i)->reset2();
	if (!track->filemodhappy)
		return AF_FAIL;
	return AF_SUCCEED;
}

status ModuleState::sync(AFfilehandle file, Track *track)
{
	track->filemodhappy = true;
	for (std::vector<SharedPtr<Module> >::reverse_iterator i=m_modules.rbegin();
			i != m_modules.rend(); ++i)
		(*i)->sync1();
	if (!track->filemodhappy)
		return AF_FAIL;
	for (std::vector<SharedPtr<Module> >::iterator i=m_modules.begin();
			i != m_modules.end(); ++i)
		(*i)->sync2();
	return AF_SUCCEED;
}

static const PCMInfo * const intmappings[6] =
{
	&_af_default_signed_integer_pcm_mappings[1],
	&_af_default_signed_integer_pcm_mappings[2],
	&_af_default_signed_integer_pcm_mappings[3],
	&_af_default_signed_integer_pcm_mappings[4],
	NULL,
	NULL
};

static FormatCode getFormatCode(const AudioFormat &format)
{
	if (format.sampleFormat == AF_SAMPFMT_FLOAT)
		return kFloat;
	if (format.sampleFormat == AF_SAMPFMT_DOUBLE)
		return kDouble;
	if (format.isInteger())
	{
		switch (format.bytesPerSample(false))
		{
			case 1: return kInt8;
			case 2: return kInt16;
			case 3: return kInt24;
			case 4: return kInt32;
		}
	}

	/* NOTREACHED */
	assert(false);
	return kUndefined;
}

static bool isInteger(FormatCode code) { return code >= kInt8 && code <= kInt32; }
static bool isFloat(FormatCode code) { return code >= kFloat && code <= kDouble; }

static bool isTrivialIntMapping(const AudioFormat &format, FormatCode code)
{
	return intmappings[code] != NULL &&
		format.pcm.slope == intmappings[code]->slope &&
		format.pcm.intercept == intmappings[code]->intercept;
}

static bool isTrivialIntClip(const AudioFormat &format, FormatCode code)
{
	return intmappings[code] != NULL &&
		format.pcm.minClip == intmappings[code]->minClip &&
		format.pcm.maxClip == intmappings[code]->maxClip;
}

status ModuleState::arrange(AFfilehandle file, Track *track)
{
	bool isReading = file->m_access == _AF_READ_ACCESS;
	AudioFormat in, out;
	if (isReading)
	{
		in = track->f;
		out = track->v;
	}
	else
	{
		in = track->v;
		out = track->f;
	}

	FormatCode infc = getFormatCode(in);
	FormatCode outfc = getFormatCode(out);
	if (infc == kUndefined || outfc == kUndefined)
		return AF_FAIL;

	m_chunks.clear();
	m_chunks.push_back(new Chunk());
	m_chunks.back()->f = in;

	m_modules.clear();

	if (isReading)
	{
		addModule(m_fileModule.get());
		addModule(m_fileRebufferModule.get());
	}

	// Convert to native byte order.
	if (in.byteOrder != _AF_BYTEORDER_NATIVE)
	{
		size_t bytesPerSample = in.bytesPerSample(!isReading);
		if (bytesPerSample > 1 && in.compressionType == AF_COMPRESSION_NONE)
			addModule(new SwapModule());
		else
			in.byteOrder = _AF_BYTEORDER_NATIVE;
	}

	// Handle 24-bit integer input format.
	if (in.isInteger() && in.bytesPerSample(false) == 3)
	{
		if (isReading || in.compressionType != AF_COMPRESSION_NONE)
			addModule(new Expand3To4Module(in.isSigned()));
	}

	// Make data signed.
	if (in.isUnsigned())
		addModule(new ConvertSign(infc, false));

	in.pcm = m_chunks.back()->f.pcm;

	// Reverse the unsigned shift for output.
	if (out.isUnsigned())
	{
		const double shift = intmappings[outfc]->minClip;
		out.pcm.intercept += shift;
		out.pcm.minClip += shift;
		out.pcm.maxClip += shift;
	}

	// Clip input samples if necessary.
	if (in.pcm.minClip < in.pcm.maxClip && !isTrivialIntClip(in, infc))
		addModule(new Clip(infc, in.pcm));

	bool alreadyClippedOutput = false;
	bool alreadyTransformedOutput = false;
	// Perform range transformation if input and output PCM mappings differ.
	bool transforming = (in.pcm.slope != out.pcm.slope ||
		in.pcm.intercept != out.pcm.intercept) &&
		!(isTrivialIntMapping(in, infc) &&
		isTrivialIntMapping(out, outfc));

	// Range transformation requires input to be floating-point.
	if (isInteger(infc) && transforming)
	{
		if (infc == kInt32 || outfc == kDouble || outfc == kInt32)
		{
			addConvertIntToFloat(infc, kDouble);
			infc = kDouble;
		}
		else
		{
			addConvertIntToFloat(infc, kFloat);
			infc = kFloat;
		}
	}

	if (transforming && infc == kDouble && isFloat(outfc))
		addModule(new Transform(infc, in.pcm, out.pcm));

	// Add format conversion if needed.
	if (isInteger(infc) && isInteger(outfc))
		addConvertIntToInt(infc, outfc);
	else if (isInteger(infc) && isFloat(outfc))
		addConvertIntToFloat(infc, outfc);
	else if (isFloat(infc) && isInteger(outfc))
	{
		addConvertFloatToInt(infc, outfc, in.pcm, out.pcm);
		alreadyClippedOutput = true;
		alreadyTransformedOutput = true;
	}
	else if (isFloat(infc) && isFloat(outfc))
		addConvertFloatToFloat(infc, outfc);

	if (transforming && !alreadyTransformedOutput && infc != kDouble)
		addModule(new Transform(outfc, in.pcm, out.pcm));

	if (in.channelCount != out.channelCount)
		addModule(new ApplyChannelMatrix(outfc, isReading,
			in.channelCount, out.channelCount,
			in.pcm.minClip, in.pcm.maxClip,
			track->channelMatrix));

	// Perform clipping if necessary.
	if (!alreadyClippedOutput)
	{
		if (out.pcm.minClip < out.pcm.maxClip && !isTrivialIntClip(out, outfc))
			addModule(new Clip(outfc, out.pcm));
	}

	// Make data unsigned if necessary.
	if (out.isUnsigned())
		addModule(new ConvertSign(outfc, true));

	// Handle 24-bit integer output format.
	if (out.isInteger() && out.bytesPerSample(false) == 3)
	{
		if (!isReading || out.compressionType != AF_COMPRESSION_NONE)
			addModule(new Compress4To3Module(out.isSigned()));
	}

	if (out.byteOrder != _AF_BYTEORDER_NATIVE)
	{
		size_t bytesPerSample = out.bytesPerSample(isReading);
		if (bytesPerSample > 1 && out.compressionType == AF_COMPRESSION_NONE)
			addModule(new SwapModule());
		else
			out.byteOrder = _AF_BYTEORDER_NATIVE;
	}

	if (!isReading)
	{
		addModule(m_fileRebufferModule.get());
		addModule(m_fileModule.get());
	}

	return AF_SUCCEED;
}

void ModuleState::addModule(Module *module)
{
	if (!module)
		return;

	m_modules.push_back(module);
	module->setInChunk(m_chunks.back().get());
	Chunk *chunk = new Chunk();
	chunk->f = m_chunks.back()->f;
	m_chunks.push_back(chunk);
	module->setOutChunk(chunk);
	module->describe();
}

void ModuleState::addConvertIntToInt(FormatCode input, FormatCode output)
{
	if (input == output)
		return;

	assert(isInteger(input));
	assert(isInteger(output));
	addModule(new ConvertInt(input, output));
}

void ModuleState::addConvertIntToFloat(FormatCode input, FormatCode output)
{
	addModule(new ConvertIntToFloat(input, output));
}

void ModuleState::addConvertFloatToInt(FormatCode input, FormatCode output,
	const PCMInfo &inputMapping, const PCMInfo &outputMapping)
{
	addModule(new ConvertFloatToIntClip(input, output, inputMapping, outputMapping));
}

void ModuleState::addConvertFloatToFloat(FormatCode input, FormatCode output)
{
	if (input == output)
		return;

	assert((input == kFloat && output == kDouble) ||
		(input == kDouble && output == kFloat));
	addModule(new ConvertFloat(input, output));
}

void ModuleState::print()
{
	fprintf(stderr, "modules:\n");
	for (size_t i=0; i<m_modules.size(); i++)
		fprintf(stderr, " %s (%p) in %p out %p\n",
			m_modules[i]->name(), m_modules[i].get(),
			m_modules[i]->inChunk(),
			m_modules[i]->outChunk());
	fprintf(stderr, "chunks:\n");
	for (size_t i=0; i<m_chunks.size(); i++)
		fprintf(stderr, " %p %s\n",
			m_chunks[i].get(),
			m_chunks[i]->f.description().c_str());
}
