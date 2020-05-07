/*
	Audio File Library
	Copyright (C) 2000-2001, Silicon Graphics, Inc.

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

/*
	units.cpp

	This file contains the file format units.
*/

#include "config.h"

#include "Features.h"
#include "audiofile.h"
#include "afinternal.h"
#include "units.h"

#include "AIFF.h"
#include "AVR.h"
#include "CAF.h"
#include "FLACFile.h"
#include "IFF.h"
#include "IRCAM.h"
#include "NeXT.h"
#include "NIST.h"
#include "Raw.h"
#include "SampleVision.h"
#include "VOC.h"
#include "WAVE.h"

#include "compression.h"

#include "modules/ALAC.h"
#include "modules/FLAC.h"
#include "modules/G711.h"
#include "modules/IMA.h"
#include "modules/MSADPCM.h"
#include "modules/PCM.h"

const Unit _af_units[_AF_NUM_UNITS] =
{
	{
		AF_FILE_RAWDATA,
		"Raw Data", "Raw Sound Data", "raw",
		true,
		&RawFile::completeSetup,
		&RawFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_RAW_NUM_COMPTYPES,
		_af_raw_compression_types,
		0,	/* maximum marker count */
		0,	/* maximum instrument count */
		0,	/* maxium number of loops per instrument */
		0, NULL,
	},
	{
		AF_FILE_AIFFC,
		"AIFF-C", "AIFF-C File Format", "aifc",
		true,
		AIFFFile::completeSetup,
		AIFFFile::recognizeAIFFC,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_AIFFC_NUM_COMPTYPES,
		_af_aiffc_compression_types,
		65535,	/* maximum marker count */
		1,	/* maximum instrument count */
		2,	/* maximum number of loops per instrument */
		_AF_AIFF_NUM_INSTPARAMS,
		_af_aiff_inst_params
	},
	{
		AF_FILE_AIFF,
		"AIFF", "Audio Interchange File Format", "aiff",
		true,
		AIFFFile::completeSetup,
		AIFFFile::recognizeAIFF,
		AF_SAMPFMT_TWOSCOMP, 16,
		0,	/* supported compression types */
		NULL,
		65535,	/* maximum marker count */
		1,	/* maximum instrument count */
		2,	/* maximum number of loops per instrument */
		_AF_AIFF_NUM_INSTPARAMS,
		_af_aiff_inst_params
	},
	{
		AF_FILE_NEXTSND,
		"NeXT .snd/Sun .au", "NeXT .snd/Sun .au Format", "next",
		true,
		NeXTFile::completeSetup,
		NeXTFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_NEXT_NUM_COMPTYPES,
		_af_next_compression_types,
		0,	/* maximum marker count */
		0,	/* maximum instrument count */
		0,	/* maximum number of loops per instrument */
		0, NULL
	},
	{
		AF_FILE_WAVE,
		"MS RIFF WAVE", "Microsoft RIFF WAVE Format", "wave",
		true,
		WAVEFile::completeSetup,
		WAVEFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_WAVE_NUM_COMPTYPES,
		_af_wave_compression_types,
		AF_NUM_UNLIMITED,	/* maximum marker count */
		1,			/* maximum instrument count */
		AF_NUM_UNLIMITED,	/* maximum number of loops per instrument */
		_AF_WAVE_NUM_INSTPARAMS,
		_af_wave_inst_params
	},
	{
		AF_FILE_IRCAM,
		"BICSF", "Berkeley/IRCAM/CARL Sound Format", "bicsf",
		true,
		IRCAMFile::completeSetup,
		IRCAMFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_IRCAM_NUM_COMPTYPES,
		_af_ircam_compression_types,
		0,		// maximum marker count
		0,		// maximum instrument count
		0,		// maximum number of loops per instrument
		0,		// number of instrument parameters
		NULL	// instrument parameters
	},
	{
		AF_FILE_MPEG1BITSTREAM,
		"MPEG", "MPEG Audio Bitstream", "mpeg",
		false
	},
	{
		AF_FILE_SOUNDDESIGNER1,
		"Sound Designer 1", "Sound Designer 1 File Format", "sd1",
		false
	},
	{
		AF_FILE_SOUNDDESIGNER2,
		"Sound Designer 2", "Sound Designer 2 File Format", "sd2",
		false
	},
	{
		AF_FILE_AVR,
		"AVR", "Audio Visual Research File Format", "avr",
		true,
		AVRFile::completeSetup,
		AVRFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		0,	/* number of compression types */
		NULL,	/* compression types */
		0,	/* maximum marker count */
		0,	/* maximum instrument count */
		0,	/* maximum number of loops per instrument */
		0,	/* number of instrument parameters */
	},
	{
		AF_FILE_IFF_8SVX,
		"IFF/8SVX", "Amiga IFF/8SVX Sound File Format", "iff",
		true,
		IFFFile::completeSetup,
		IFFFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 8,
		0,	/* number of compression types */
		NULL,	/* compression types */
		0,	/* maximum marker count */
		0,	/* maximum instrument count */
		0,	/* maximum number of loops per instrument */
		0,	/* number of instrument parameters */
	},
	{
		AF_FILE_SAMPLEVISION,
		"Sample Vision", "Sample Vision File Format", "smp",
		true,
		SampleVisionFile::completeSetup,
		SampleVisionFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		0,		// number of compression types
		NULL,	// compression types
		0,		// maximum marker count
		0,		// maximum instrument count
		0,		// maximum number of loops per instrument
		0,		// number of instrument parameters
		NULL	// instrument parameters
	},
	{
		AF_FILE_VOC,
		"VOC", "Creative Voice File Format", "voc",
		true,
		VOCFile::completeSetup,
		VOCFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_VOC_NUM_COMPTYPES,
		_af_voc_compression_types,
		0,		// maximum marker count
		0,		// maximum instrument count
		0,		// maximum number of loops per instrument
		0,		// number of instrument parameters
		NULL	// instrument parameters
	},
	{
		AF_FILE_NIST_SPHERE,
		"NIST SPHERE", "NIST SPHERE File Format", "nist",
		true,
		NISTFile::completeSetup,
		NISTFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		0,	/* number of compression types */
		NULL,	/* compression types */
		0,	/* maximum marker count */
		0,	/* maximum instrument count */
		0,	/* maximum number of loops per instrument */
		0,	/* number of instrument parameters */
		NULL	/* instrument parameters */
	},
	{
		AF_FILE_SOUNDFONT2,
		"SoundFont 2", "SoundFont 2 File Format", "sf2",
		false
	},
	{
		AF_FILE_CAF,
		"CAF", "Core Audio Format", "caf",
		true,
		CAFFile::completeSetup,
		CAFFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_CAF_NUM_COMPTYPES,
		_af_caf_compression_types,
		0,		// maximum marker count
		0,		// maximum instrument count
		0,		// maximum number of loops per instrument
		0,		// number of instrument parameters
		NULL	// instrument parameters
	},
	{
		AF_FILE_FLAC,
		"FLAC", "Free Lossless Audio Codec", "flac",
		true,
		FLACFile::completeSetup,
		FLACFile::recognize,
		AF_SAMPFMT_TWOSCOMP, 16,
		_AF_FLAC_NUM_COMPTYPES,
		_af_flac_compression_types,
		0,		// maximum marker count
		0,		// maximum instrument count
		0,		// maximum number of loops per instrument
		0,		// number of instrument parameters
		NULL	// instrument parameters
	}
};

const CompressionUnit _af_compression[_AF_NUM_COMPRESSION] =
{
	{
		AF_COMPRESSION_NONE,
		true,
		"none",	/* label */
		"none",	/* short name */
		"not compressed",
		1.0,
		AF_SAMPFMT_TWOSCOMP, 16,
		false,	/* needsRebuffer */
		false,	/* multiple_of */
		_af_pcm_format_ok,
		_AFpcminitcompress, _AFpcminitdecompress
	},
	{
		AF_COMPRESSION_G711_ULAW,
		true,
		"ulaw",	/* label */
		"CCITT G.711 u-law",	/* shortname */
		"CCITT G.711 u-law",
		2.0,
		AF_SAMPFMT_TWOSCOMP, 16,
		false,	/* needsRebuffer */
		false,	/* multiple_of */
		_af_g711_format_ok,
		_AFg711initcompress, _AFg711initdecompress
	},
	{
		AF_COMPRESSION_G711_ALAW,
		true,
		"alaw",	/* label */
		"CCITT G.711 A-law",	/* short name */
		"CCITT G.711 A-law",
		2.0,
		AF_SAMPFMT_TWOSCOMP, 16,
		false,	/* needsRebuffer */
		false,	/* multiple_of */
		_af_g711_format_ok,
		_AFg711initcompress, _AFg711initdecompress
	},
	{
		AF_COMPRESSION_IMA,
		true,
		"ima4",	/* label */
		"IMA ADPCM",	/* short name */
		"IMA DVI ADPCM",
		4.0,
		AF_SAMPFMT_TWOSCOMP, 16,
		true,	/* needsRebuffer */
		false,	/* multiple_of */
		_af_ima_adpcm_format_ok,
		_af_ima_adpcm_init_compress, _af_ima_adpcm_init_decompress
	},
	{
		AF_COMPRESSION_MS_ADPCM,
		true,
		"msadpcm",	/* label */
		"MS ADPCM",	/* short name */
		"Microsoft ADPCM",
		4.0,
		AF_SAMPFMT_TWOSCOMP, 16,
		true,	/* needsRebuffer */
		false,	/* multiple_of */
		_af_ms_adpcm_format_ok,
		_af_ms_adpcm_init_compress, _af_ms_adpcm_init_decompress
	},
	{
		AF_COMPRESSION_FLAC,
#if ENABLE(FLAC)
		true,
#else
		false,
#endif
		"flac",	// label
		"FLAC",	// short name
		"Free Lossless Audio Codec",
		1.0,
		AF_SAMPFMT_TWOSCOMP, 16,
		false,	// needsRebuffer
		false,	// multiple_of
		_af_flac_format_ok,
		_af_flac_init_compress, _af_flac_init_decompress
	},
	{
		AF_COMPRESSION_ALAC,
		true,
		"alac",	// label
		"ALAC",	// short name
		"Apple Lossless Audio Codec",
		1.0,
		AF_SAMPFMT_TWOSCOMP, 16,
		true,	// needsRebuffer
		false,	// multiple_of
		_af_alac_format_ok,
		_af_alac_init_compress, _af_alac_init_decompress
	}
};
