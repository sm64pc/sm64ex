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

#ifndef SIMPLE_MODULE_H
#define SIMPLE_MODULE_H

#include "config.h"

#include "Compiler.h"
#include "Module.h"
#include "byteorder.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <functional>

class SimpleModule : public Module
{
public:
	virtual void runPull() OVERRIDE;
	virtual void runPush() OVERRIDE;
	virtual void run(Chunk &inChunk, Chunk &outChunk) = 0;
};

struct SwapModule : public SimpleModule
{
public:
	virtual const char *name() const OVERRIDE { return "swap"; }
	virtual void describe() OVERRIDE
	{
		m_outChunk->f.byteOrder = m_inChunk->f.byteOrder == AF_BYTEORDER_BIGENDIAN ?
			AF_BYTEORDER_LITTLEENDIAN : AF_BYTEORDER_BIGENDIAN;
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		switch (m_inChunk->f.bytesPerSample(false))
		{
			case 2:
				run<2, int16_t>(inChunk, outChunk); break;
			case 3:
				run<3, char>(inChunk, outChunk); break;
			case 4:
				run<4, int32_t>(inChunk, outChunk); break;
			case 8:
				run<8, int64_t>(inChunk, outChunk); break;
			default:
				assert(false); break;
		}
	}

private:
	template <int N, typename T>
	void run(Chunk &inChunk, Chunk &outChunk)
	{
		int sampleCount = inChunk.f.channelCount * inChunk.frameCount;
		runSwap<N, T>(reinterpret_cast<const T *>(inChunk.buffer),
			reinterpret_cast<T *>(outChunk.buffer),
			sampleCount);
	}
	template <int N, typename T>
	void runSwap(const T *input, T *output, int sampleCount)
	{
		for (int i=0; i<sampleCount; i++)
			output[i] = byteswap(input[i]);
	}
};

template <>
inline void SwapModule::runSwap<3, char>(const char *input, char *output, int count)
{
	for (int i=0; i<count; i++)
	{
		output[3*i] = input[3*i+2];
		output[3*i+1] = input[3*i+1];
		output[3*i+2] = input[3*i];
	}
}

template <typename UnaryFunction>
void transform(const void *srcData, void *dstData, size_t count)
{
	typedef typename UnaryFunction::argument_type InputType;
	typedef typename UnaryFunction::result_type OutputType;
	const InputType *src = reinterpret_cast<const InputType *>(srcData);
	OutputType *dst = reinterpret_cast<OutputType *>(dstData);
	std::transform(src, src + count, dst, UnaryFunction());
}

template <FormatCode>
struct IntTypes;

template <>
struct IntTypes<kInt8> { typedef int8_t SignedType; typedef uint8_t UnsignedType; };
template <>
struct IntTypes<kInt16> { typedef int16_t SignedType; typedef uint16_t UnsignedType; };
template <>
struct IntTypes<kInt24> { typedef int32_t SignedType; typedef uint32_t UnsignedType; };
template <>
struct IntTypes<kInt32> { typedef int32_t SignedType; typedef uint32_t UnsignedType; };

template <FormatCode Format>
struct signConverter
{
	typedef typename IntTypes<Format>::SignedType SignedType;
	typedef typename IntTypes<Format>::UnsignedType UnsignedType;

	static const int kScaleBits = (Format + 1) * CHAR_BIT - 1;
	static const int kMaxSignedValue = (((1 << (kScaleBits - 1)) - 1) << 1) + 1;
	static const int kMinSignedValue = -kMaxSignedValue - 1;

	struct signedToUnsigned : public std::unary_function<SignedType, UnsignedType>
	{
		UnsignedType operator()(SignedType x) { return x - kMinSignedValue; }
	};

	struct unsignedToSigned : public std::unary_function<SignedType, UnsignedType>
	{
		SignedType operator()(UnsignedType x) { return x + kMinSignedValue; }
	};
};

class ConvertSign : public SimpleModule
{
public:
	ConvertSign(FormatCode format, bool fromSigned) :
		m_format(format),
		m_fromSigned(fromSigned)
	{
	}
	virtual const char *name() const OVERRIDE { return "sign"; }
	virtual void describe() OVERRIDE
	{
		const int scaleBits = m_inChunk->f.bytesPerSample(false) * CHAR_BIT;
		m_outChunk->f.sampleFormat =
			m_fromSigned ? AF_SAMPFMT_UNSIGNED : AF_SAMPFMT_TWOSCOMP;
		double shift = -(1 << (scaleBits - 1));
		if (m_fromSigned)
			shift = -shift;
		m_outChunk->f.pcm.intercept += shift;
		m_outChunk->f.pcm.minClip += shift;
		m_outChunk->f.pcm.maxClip += shift;
	}
	virtual void run(Chunk &input, Chunk &output) OVERRIDE
	{
		size_t count = input.frameCount * m_inChunk->f.channelCount;
		if (m_fromSigned)
			convertSignedToUnsigned(input.buffer, output.buffer, count);
		else
			convertUnsignedToSigned(input.buffer, output.buffer, count);
	}

private:
	FormatCode m_format;
	bool m_fromSigned;

	template <FormatCode Format>
	static void convertSignedToUnsigned(const void *src, void *dst, size_t count)
	{
		transform<typename signConverter<Format>::signedToUnsigned>(src, dst, count);
	}
	void convertSignedToUnsigned(const void *src, void *dst, size_t count)
	{
		switch (m_format)
		{
			case kInt8:
				convertSignedToUnsigned<kInt8>(src, dst, count);
				break;
			case kInt16:
				convertSignedToUnsigned<kInt16>(src, dst, count);
				break;
			case kInt24:
				convertSignedToUnsigned<kInt24>(src, dst, count);
				break;
			case kInt32:
				convertSignedToUnsigned<kInt32>(src, dst, count);
				break;
			default:
				assert(false);
		}
	}

	template <FormatCode Format>
	static void convertUnsignedToSigned(const void *src, void *dst, size_t count)
	{
		transform<typename signConverter<Format>::unsignedToSigned>(src, dst, count);
	}
	void convertUnsignedToSigned(const void *src, void *dst, size_t count)
	{
		switch (m_format)
		{
			case kInt8:
				convertUnsignedToSigned<kInt8>(src, dst, count);
				break;
			case kInt16:
				convertUnsignedToSigned<kInt16>(src, dst, count);
				break;
			case kInt24:
				convertUnsignedToSigned<kInt24>(src, dst, count);
				break;
			case kInt32:
				convertUnsignedToSigned<kInt32>(src, dst, count);
				break;
			default:
				assert(false);
		}
	}
};

struct Expand3To4Module : public SimpleModule
{
public:
	Expand3To4Module(bool isSigned) : m_isSigned(isSigned)
	{
	}
	virtual const char *name() const OVERRIDE { return "expand3to4"; }
	virtual void describe() OVERRIDE
	{
		m_outChunk->f.packed = false;
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		int count = inChunk.f.channelCount * inChunk.frameCount;
		if (m_isSigned)
			run<int32_t>(reinterpret_cast<const uint8_t *>(inChunk.buffer),
				reinterpret_cast<int32_t *>(outChunk.buffer),
				count);
		else
			run<uint32_t>(reinterpret_cast<const uint8_t *>(inChunk.buffer),
				reinterpret_cast<uint32_t *>(outChunk.buffer),
				count);
	}

private:
	bool m_isSigned;

	template <typename T>
	void run(const uint8_t *input, T *output, int sampleCount)
	{
		for (int i=0; i<sampleCount; i++)
		{
			T t =
#ifdef WORDS_BIGENDIAN
				(input[3*i] << 24) |
				(input[3*i+1] << 16) |
				input[3*i+2] << 8;
#else
				(input[3*i+2] << 24) |
				(input[3*i+1] << 16) |
				input[3*i] << 8;
#endif
			output[i] = t >> 8;
		}
	}
};

struct Compress4To3Module : public SimpleModule
{
public:
	Compress4To3Module(bool isSigned) : m_isSigned(isSigned)
	{
	}
	virtual const char *name() const OVERRIDE { return "compress4to3"; }
	virtual void describe() OVERRIDE
	{
		m_outChunk->f.packed = true;
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		int count = inChunk.f.channelCount * inChunk.frameCount;
		if (m_isSigned)
			run<int32_t>(inChunk.buffer, outChunk.buffer, count);
		else
			run<uint32_t>(inChunk.buffer, outChunk.buffer, count);
	}

private:
	bool m_isSigned;

	template <typename T>
	void run(const void *input, void *output, int count)
	{
		const T *in = reinterpret_cast<const T *>(input);
		uint8_t *out = reinterpret_cast<uint8_t *>(output);
		for (int i=0; i<count; i++)
		{
			uint8_t c0, c1, c2;
			extract3(in[i], c0, c1, c2);
			out[3*i] = c0;
			out[3*i+1] = c1;
			out[3*i+2] = c2;
		}
	}
	template <typename T>
	void extract3(T in, uint8_t &c0, uint8_t &c1, uint8_t &c2)
	{
#ifdef WORDS_BIGENDIAN
			c0 = (in >> 16) & 0xff;
			c1 = (in >> 8) & 0xff;
			c2 = in & 0xff;
#else
			c2 = (in >> 16) & 0xff;
			c1 = (in >> 8) & 0xff;
			c0 = in & 0xff;
#endif
	}
};

template <typename Arg, typename Result>
struct intToFloat : public std::unary_function<Arg, Result>
{
	Result operator()(Arg x) const { return x; }
};

struct ConvertIntToFloat : public SimpleModule
{
	ConvertIntToFloat(FormatCode inFormat, FormatCode outFormat) :
		m_inFormat(inFormat), m_outFormat(outFormat)
	{
	}
	virtual const char *name() const OVERRIDE { return "intToFloat"; }
	virtual void describe() OVERRIDE
	{
		m_outChunk->f.sampleFormat = m_outFormat == kDouble ?
			AF_SAMPFMT_DOUBLE : AF_SAMPFMT_FLOAT;
		m_outChunk->f.sampleWidth = m_outFormat == kDouble ? 64 : 32;
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		const void *src = inChunk.buffer;
		void *dst = outChunk.buffer;
		int count = inChunk.frameCount * inChunk.f.channelCount;
		if (m_outFormat == kFloat)
		{
			switch (m_inFormat)
			{
				case kInt8:
					run<int8_t, float>(src, dst, count); break;
				case kInt16:
					run<int16_t, float>(src, dst, count); break;
				case kInt24:
				case kInt32:
					run<int32_t, float>(src, dst, count); break;
				default:
					assert(false);
			}
		}
		else if (m_outFormat == kDouble)
		{
			switch (m_inFormat)
			{
				case kInt8:
					run<int8_t, double>(src, dst, count); break;
				case kInt16:
					run<int16_t, double>(src, dst, count); break;
				case kInt24:
				case kInt32:
					run<int32_t, double>(src, dst, count); break;
				default:
					assert(false);
			}
		}
	}

private:
	FormatCode m_inFormat, m_outFormat;

	template <typename Arg, typename Result>
	static void run(const void *src, void *dst, int count)
	{
		transform<intToFloat<Arg, Result> >(src, dst, count);
	}
};

template <typename Arg, typename Result, unsigned shift>
struct lshift : public std::unary_function<Arg, Result>
{
	Result operator()(const Arg &x) const { return x << shift; }
};

template <typename Arg, typename Result, unsigned shift>
struct rshift : public std::unary_function<Arg, Result>
{
	Result operator()(const Arg &x) const { return x >> shift; }
};

struct ConvertInt : public SimpleModule
{
	ConvertInt(FormatCode inFormat, FormatCode outFormat) :
		m_inFormat(inFormat),
		m_outFormat(outFormat)
	{
		assert(isInteger(m_inFormat));
		assert(isInteger(m_outFormat));
	}
	virtual const char *name() const OVERRIDE { return "convertInt"; }
	virtual void describe() OVERRIDE
	{
		getDefaultPCMMapping(m_outChunk->f.sampleWidth,
			m_outChunk->f.pcm.slope,
			m_outChunk->f.pcm.intercept,
			m_outChunk->f.pcm.minClip,
			m_outChunk->f.pcm.maxClip);
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		const void *src = inChunk.buffer;
		void *dst = outChunk.buffer;
		size_t count = inChunk.frameCount * inChunk.f.channelCount;

#define MASK(N, M) (((N)<<3) | (M))
#define HANDLE(N, M) \
	case MASK(N, M): convertInt<N, M>(src, dst, count); break;
		switch (MASK(m_inFormat, m_outFormat))
		{
			HANDLE(kInt8, kInt16)
			HANDLE(kInt8, kInt24)
			HANDLE(kInt8, kInt32)
			HANDLE(kInt16, kInt8)
			HANDLE(kInt16, kInt24)
			HANDLE(kInt16, kInt32)
			HANDLE(kInt24, kInt8)
			HANDLE(kInt24, kInt16)
			HANDLE(kInt24, kInt32)
			HANDLE(kInt32, kInt8)
			HANDLE(kInt32, kInt16)
			HANDLE(kInt32, kInt24)
		}
#undef MASK
#undef HANDLE
	}

private:
	FormatCode m_inFormat, m_outFormat;

	void getDefaultPCMMapping(int &bits, double &slope, double &intercept,
		double &minClip, double &maxClip)
	{
		bits = (m_outFormat + 1) * CHAR_BIT;
		slope = (1LL << (bits - 1));
		intercept = 0;
		minClip = -(1 << (bits - 1));
		maxClip = (1LL << (bits - 1)) - 1;
	}

	static bool isInteger(FormatCode code)
	{
		return code >= kInt8 && code <= kInt32;
	}

	template <FormatCode Input, FormatCode Output, bool = (Input > Output)>
		struct shift;

	template <FormatCode Input, FormatCode Output>
	struct shift<Input, Output, true> :
		public rshift<typename IntTypes<Input>::SignedType,
			typename IntTypes<Output>::SignedType,
			(Input - Output) * CHAR_BIT>
	{
	};

	template <FormatCode Input, FormatCode Output>
	struct shift<Input, Output, false> :
		public lshift<typename IntTypes<Input>::SignedType,
			typename IntTypes<Output>::SignedType,
			(Output - Input) * CHAR_BIT>
	{
	};

	template <FormatCode Input, FormatCode Output>
	static void convertInt(const void *src, void *dst, int count)
	{
		transform<shift<Input, Output> >(src, dst, count);
	}
};

template <typename Arg, typename Result>
struct floatToFloat : public std::unary_function<Arg, Result>
{
	Result operator()(Arg x) const { return x; }
};

struct ConvertFloat : public SimpleModule
{
	ConvertFloat(FormatCode inFormat, FormatCode outFormat) :
		m_inFormat(inFormat), m_outFormat(outFormat)
	{
		assert((m_inFormat == kFloat && m_outFormat == kDouble) ||
			(m_inFormat == kDouble && m_outFormat == kFloat));
	}
	virtual const char *name() const OVERRIDE { return "convertFloat"; }
	virtual void describe() OVERRIDE
	{
		switch (m_outFormat)
		{
			case kFloat:
				m_outChunk->f.sampleFormat = AF_SAMPFMT_FLOAT;
				m_outChunk->f.sampleWidth = 32;
				break;
			case kDouble:
				m_outChunk->f.sampleFormat = AF_SAMPFMT_DOUBLE;
				m_outChunk->f.sampleWidth = 64;
				break;
			default:
				assert(false);
		}
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		const void *src = inChunk.buffer;
		void *dst = outChunk.buffer;
		size_t count = inChunk.frameCount * inChunk.f.channelCount;

		switch (m_outFormat)
		{
			case kFloat:
				transform<floatToFloat<double, float> >(src, dst, count);
				break;
			case kDouble:
				transform<floatToFloat<float, double> >(src, dst, count);
				break;
			default:
				assert(false);
		}
	}

private:
	FormatCode m_inFormat, m_outFormat;
};

struct Clip : public SimpleModule
{
	Clip(FormatCode format, const PCMInfo &outputMapping) :
		m_format(format),
		m_outputMapping(outputMapping)
	{
	}
	virtual const char *name() const OVERRIDE { return "clip"; }
	virtual void describe() OVERRIDE
	{
		m_outChunk->f.pcm = m_outputMapping;
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		const void *src = inChunk.buffer;
		void *dst = outChunk.buffer;
		int count = inChunk.frameCount * inChunk.f.channelCount;

		switch (m_format)
		{
			case kInt8:
				run<int8_t>(src, dst, count); break;
			case kInt16:
				run<int16_t>(src, dst, count); break;
			case kInt24:
			case kInt32:
				run<int32_t>(src, dst, count); break;
			case kFloat:
				run<float>(src, dst, count); break;
			case kDouble:
				run<double>(src, dst, count); break;
			default:
				assert(false);
		}
	}

private:
	FormatCode m_format;
	PCMInfo m_outputMapping;

	template <typename T>
	void run(const void *srcData, void *dstData, int count)
	{
		const T minValue = m_outputMapping.minClip;
		const T maxValue = m_outputMapping.maxClip;

		const T *src = reinterpret_cast<const T *>(srcData);
		T *dst = reinterpret_cast<T *>(dstData);

		for (int i=0; i<count; i++)
		{
			T t = src[i];
			t = std::min(t, maxValue);
			t = std::max(t, minValue);
			dst[i] = t;
		}
	}
};

struct ConvertFloatToIntClip : public SimpleModule
{
	ConvertFloatToIntClip(FormatCode inputFormat, FormatCode outputFormat,
		const PCMInfo &inputMapping, const PCMInfo &outputMapping) :
		m_inputFormat(inputFormat),
		m_outputFormat(outputFormat),
		m_inputMapping(inputMapping),
		m_outputMapping(outputMapping)
	{
		assert(m_inputFormat == kFloat || m_inputFormat == kDouble);
		assert(m_outputFormat == kInt8 ||
			m_outputFormat == kInt16 ||
			m_outputFormat == kInt24 ||
			m_outputFormat == kInt32);
	}
	virtual const char *name() const OVERRIDE { return "convertPCMMapping"; }
	virtual void describe() OVERRIDE
	{
		m_outChunk->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
		m_outChunk->f.sampleWidth = (m_outputFormat + 1) * CHAR_BIT;
		m_outChunk->f.pcm = m_outputMapping;
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		const void *src = inChunk.buffer;
		void *dst = outChunk.buffer;
		int count = inChunk.frameCount * inChunk.f.channelCount;

		if (m_inputFormat == kFloat)
		{
			switch (m_outputFormat)
			{
				case kInt8:
					run<float, int8_t>(src, dst, count); break;
				case kInt16:
					run<float, int16_t>(src, dst, count); break;
				case kInt24:
				case kInt32:
					run<float, int32_t>(src, dst, count); break;
				default:
					assert(false);
			}
		}
		else if (m_inputFormat == kDouble)
		{
			switch (m_outputFormat)
			{
				case kInt8:
					run<double, int8_t>(src, dst, count); break;
				case kInt16:
					run<double, int16_t>(src, dst, count); break;
				case kInt24:
				case kInt32:
					run<double, int32_t>(src, dst, count); break;
				default:
					assert(false);
			}
		}
	}

private:
	FormatCode m_inputFormat, m_outputFormat;
	PCMInfo m_inputMapping, m_outputMapping;

	template <typename Input, typename Output>
	void run(const void *srcData, void *dstData, int count)
	{
		const Input *src = reinterpret_cast<const Input *>(srcData);
		Output *dst = reinterpret_cast<Output *>(dstData);

		double m = m_outputMapping.slope / m_inputMapping.slope;
		double b = m_outputMapping.intercept - m * m_inputMapping.intercept;
		double minValue = m_outputMapping.minClip;
		double maxValue = m_outputMapping.maxClip;

		for (int i=0; i<count; i++)
		{
			double t = m * src[i] + b;
			t = std::min(t, maxValue);
			t = std::max(t, minValue);
			dst[i] = static_cast<Output>(t);
		}
	}
};

struct ApplyChannelMatrix : public SimpleModule
{
public:
	ApplyChannelMatrix(FormatCode format, bool isReading,
		int inChannels, int outChannels,
		double minClip, double maxClip, const double *matrix);
	virtual ~ApplyChannelMatrix();
	virtual const char *name() const OVERRIDE;
	virtual void describe() OVERRIDE;
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE;

private:
	FormatCode m_format;
	int m_inChannels, m_outChannels;
	double m_minClip, m_maxClip;
	double *m_matrix;

	void initDefaultMatrix();
	template <typename T>
		void run(const void *input, void *output, int frameCount);
};

struct Transform : public SimpleModule
{
public:
	Transform(FormatCode format,
		const PCMInfo &inputMapping,
		const PCMInfo &outputMapping) :
		m_format(format),
		m_inputMapping(inputMapping),
		m_outputMapping(outputMapping)
	{
		assert(m_format == kFloat || m_format == kDouble);
	}
	virtual const char *name() const OVERRIDE { return "transform"; }
	virtual void describe() OVERRIDE
	{
		m_outChunk->f.pcm = m_outputMapping;
	}
	virtual void run(Chunk &inChunk, Chunk &outChunk) OVERRIDE
	{
		int count = inChunk.frameCount * inChunk.f.channelCount;
		if (m_format == kFloat)
			run<float>(inChunk.buffer, outChunk.buffer, count);
		else if (m_format == kDouble)
			run<double>(inChunk.buffer, outChunk.buffer, count);
		else
			assert(false);
	}

private:
	FormatCode m_format;
	PCMInfo m_inputMapping, m_outputMapping;

	template <typename T>
	void run(const void *srcData, void *dstData, int count)
	{
		const T *src = reinterpret_cast<const T *>(srcData);
		T *dst = reinterpret_cast<T *>(dstData);

		double m = m_outputMapping.slope / m_inputMapping.slope;
		double b = m_outputMapping.intercept - m * m_inputMapping.intercept;

		for (int i=0; i<count; i++)
			dst[i] = m * src[i] + b;
	}
};

#endif // SIMPLE_MODULE_H
