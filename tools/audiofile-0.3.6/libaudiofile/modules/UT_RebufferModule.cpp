/*
	Audio File Library
	Copyright (C) 2012, Michael Pruett <michael@68k.org>

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

#include <audiofile.h>
#include <gtest/gtest.h>
#include <limits>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "byteorder.h"
#include "RebufferModule.h"

static void setChunkData(Chunk &chunk, int frameOffset)
{
	int16_t *data = static_cast<int16_t *>(chunk.buffer);
	int channels = chunk.f.channelCount;
	for (unsigned i=0; i<chunk.frameCount; i++)
		for (int c=0; c<channels; c++)
			data[i*channels + c] = (frameOffset + i) * channels + c;
}

static void validateChunkData(const Chunk &chunk, int frameOffset)
{
	const int16_t *data = static_cast<const int16_t *>(chunk.buffer);
	int channels = chunk.f.channelCount;
	for (unsigned i=0; i < chunk.frameCount; i++)
		for (int c=0; c < channels; c++)
			EXPECT_EQ(data[i * channels + c],
				int(frameOffset + i) * channels + c);
}

class TestSourceModule : public Module
{
public:
	TestSourceModule() :
		m_startFrame(0),
		m_frameCount(0),
		m_expectedRequestLength(0)
	{
	}
	unsigned startFrame() const { return m_startFrame; }
	void setStartFrame(unsigned startFrame) { m_startFrame = startFrame; }
	unsigned frameCount() const { return m_frameCount; }
	void setFrameCount(unsigned frameCount) { m_frameCount = frameCount; }
	unsigned expectedRequestLength() const { return m_expectedRequestLength; }
	void setExpectedRequestLength(unsigned expectedRequestLength)
	{
		m_expectedRequestLength = expectedRequestLength;
	}

	void runPull()
	{
		EXPECT_EQ(m_outChunk->frameCount, m_expectedRequestLength);
		unsigned frameCount = std::min<unsigned>(m_outChunk->frameCount, m_frameCount);
		m_outChunk->frameCount = frameCount;
		setChunkData(*m_outChunk, m_startFrame);
		m_startFrame += frameCount;
		m_frameCount -= frameCount;
	}

private:
	unsigned m_startFrame;
	unsigned m_frameCount;
	unsigned m_expectedRequestLength;
};

class TestSinkModule : public Module
{
public:
	TestSinkModule() :
		m_startFrame(0),
		m_expectedRequestLength(0)
	{
	}
	unsigned startFrame() const { return m_startFrame; }
	void setStartFrame(unsigned startFrame) { m_startFrame = startFrame; }
	unsigned expectedRequestLength() const { return m_expectedRequestLength; }
	void setExpectedRequestLength(unsigned expectedRequestLength)
	{
		m_expectedRequestLength = expectedRequestLength;
	}
	void runPush()
	{
		EXPECT_EQ(m_inChunk->frameCount, m_expectedRequestLength);
		validateChunkData(*m_inChunk, m_startFrame);
		m_startFrame += m_inChunk->frameCount;
	}

private:
	unsigned m_startFrame;
	unsigned m_expectedRequestLength;
};

static AudioFormat createAudioFormat(int channels)
{
	AudioFormat f =
	{
		44100,
		AF_SAMPFMT_TWOSCOMP,
		16,
		_AF_BYTEORDER_NATIVE,
		{ 0, 0, 0, 0 },
		channels,
		AF_COMPRESSION_NONE,
		NULL,
		false
	};

	return f;
}

static void testFixedToVariable(bool multiple)
{
	const int channels = 2;
	AudioFormat f = createAudioFormat(channels);

	SharedPtr<RebufferModule> rebuffer =
		new RebufferModule(RebufferModule::FixedToVariable, f.bytesPerFrame(),
			10, multiple);

	SharedPtr<TestSourceModule> source = new TestSourceModule();
	rebuffer->setSource(source.get());

	SharedPtr<Chunk> fixedChunk(new Chunk());
	SharedPtr<Chunk> variableChunk(new Chunk());

	const int maxFrameCount = 50;
	fixedChunk->f = f;
	fixedChunk->allocate(maxFrameCount * f.bytesPerFrame());

	variableChunk->f = f;
	variableChunk->allocate(maxFrameCount * f.bytesPerFrame());

	rebuffer->setInChunk(fixedChunk.get());
	rebuffer->setOutChunk(variableChunk.get());

	source->setOutChunk(fixedChunk.get());

	// Initialize source to contain 100 frames.
	source->setFrameCount(100);

	// Request 22 frames from rebuffer module.
	variableChunk->frameCount = 22;
	source->setExpectedRequestLength(multiple ? 30 : 10);
	rebuffer->runPull();
	// Check that 30 frames have been pulled from source module.
	EXPECT_EQ(30u, source->startFrame());
	// Check that rebuffer module has fulfilled request of 22 frames.
	EXPECT_EQ(22u, variableChunk->frameCount);
	// Validate output data.
	validateChunkData(*variableChunk, 0);

	// Request 30 frames from rebuffer module.
	variableChunk->frameCount = 30;
	source->setExpectedRequestLength(multiple ? 30 : 10);
	rebuffer->runPull();
	// Check that 60 frames have been pulled from source module.
	EXPECT_EQ(60u, source->startFrame());
	// Check that rebuffer module has fulfilled request of 28 frames.
	EXPECT_EQ(30u, variableChunk->frameCount);
	// Validate output data.
	validateChunkData(*variableChunk, 22);

	// Request 50 frames from rebuffer module.
	variableChunk->frameCount = 50;
	source->setExpectedRequestLength(multiple ? 50 : 10);
	rebuffer->runPull();
	// Check that 100 frames have been pulled from source module.
	EXPECT_EQ(100u, source->startFrame());
	// Check that rebuffer module has filled 48 of 50 frames requested.
	EXPECT_EQ(48u, variableChunk->frameCount);
	// Validate output data.
	validateChunkData(*variableChunk, 52);
}

TEST(RebufferModule, FixedToVariable)
{
	testFixedToVariable(false);
}

TEST(RebufferModule, FixedToVariable_Multiple)
{
	testFixedToVariable(true);
}

/*
	Make a request to the rebuffer module which is large enough
	to pull the final short chunk from the test source but which
	doesn't consume all the frames of that chunk.

	Verify that a subsequent request to the rebuffer module correctly
	produces the remaining frames of that chunk.
*/
static void testBufferingAfterShortChunk(bool multiple)
{
	const int channels = 2;
	AudioFormat f = createAudioFormat(channels);

	SharedPtr<RebufferModule> rebuffer =
		new RebufferModule(RebufferModule::FixedToVariable, f.bytesPerFrame(),
			10, multiple);

	SharedPtr<TestSourceModule> source = new TestSourceModule();
	rebuffer->setSource(source.get());

	SharedPtr<Chunk> fixedChunk(new Chunk());
	SharedPtr<Chunk> variableChunk(new Chunk());

	const int maxFrameCount = 30;
	fixedChunk->f = f;
	fixedChunk->allocate(maxFrameCount * f.bytesPerFrame());

	variableChunk->f = f;
	variableChunk->allocate(maxFrameCount * f.bytesPerFrame());

	rebuffer->setInChunk(fixedChunk.get());
	rebuffer->setOutChunk(variableChunk.get());

	source->setOutChunk(fixedChunk.get());

	// Initialize source to contain 23 frames.
	source->setFrameCount(23);

	// Request 21 frames from rebuffer module.
	variableChunk->frameCount = 21;
	source->setExpectedRequestLength(multiple ? 30 : 10);
	rebuffer->runPull();
	// Check that all 23 frames have been pulled from source module.
	EXPECT_EQ(23u, source->startFrame());
	// Check that rebuffer module has fulfilled request of 21 frames.
	EXPECT_EQ(21u, variableChunk->frameCount);
	// Validate output data.
	validateChunkData(*variableChunk, 0);

	// Request 5 frames from rebuffer module.
	variableChunk->frameCount = 5;
	source->setExpectedRequestLength(-1);
	rebuffer->runPull();
	// Check that rebuffer module has delivered remaining 2 frames.
	EXPECT_EQ(2u, variableChunk->frameCount);
	// Validate output data.
	validateChunkData(*variableChunk, 21);
}

TEST(RebufferModule, FixedToVariable_BufferingAfterShortChunk)
{
	testBufferingAfterShortChunk(false);
}

TEST(RebufferModule, FixedToVariable_BufferingAfterShortChunk_Multiple)
{
	testBufferingAfterShortChunk(true);
}

static void testVariableToFixed(bool multiple)
{
	const int channels = 2;
	AudioFormat f = createAudioFormat(channels);

	SharedPtr<RebufferModule> rebuffer =
		new RebufferModule(RebufferModule::VariableToFixed, f.bytesPerFrame(),
			10, multiple);
	SharedPtr<TestSinkModule> sink = new TestSinkModule();
	rebuffer->setSink(sink.get());

	SharedPtr<Chunk> variableChunk(new Chunk());
	SharedPtr<Chunk> fixedChunk(new Chunk());

	const int maxFrameCount = 40;
	variableChunk->f = f;
	variableChunk->allocate(maxFrameCount * f.bytesPerFrame());

	fixedChunk->f = f;
	fixedChunk->allocate(maxFrameCount * f.bytesPerFrame());

	rebuffer->setInChunk(variableChunk.get());
	rebuffer->setOutChunk(fixedChunk.get());

	sink->setInChunk(fixedChunk.get());

	// Push 23 frames to the rebuffer module.
	variableChunk->frameCount = 23;
	setChunkData(*variableChunk, 0);
	sink->setExpectedRequestLength(multiple ? 20 : 10);
	rebuffer->runPush();
	// Check that 20 frames have been pushed to the sink module.
	EXPECT_EQ(sink->startFrame(), 20u);
	// Check that the last push contained 20 (multiple) or 10 (single) frames.
	EXPECT_EQ(fixedChunk->frameCount, multiple ? 20u : 10u);

	// Push another 37 frames to the rebuffer module.
	variableChunk->frameCount = 37;
	setChunkData(*variableChunk, 23);
	sink->setExpectedRequestLength(multiple ? 40 : 10);
	rebuffer->runPush();
	// Check that 60 frames have been pushed to the sink module.
	EXPECT_EQ(sink->startFrame(), 60u);
	// Check that the last push contained 40 (multiple) or 10 (single) frames.
	EXPECT_EQ(fixedChunk->frameCount, multiple ? 40u : 10u);
}

TEST(RebufferModule, VariableToFixed)
{
	testVariableToFixed(false);
}

TEST(RebufferModule, VariableToFixed_Multiple)
{
	testVariableToFixed(true);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
