/*
	Audio File Library
	Copyright (C) 2000, Silicon Graphics, Inc.

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
	track.h
*/

#ifndef TRACK_H
#define TRACK_H

#include "AudioFormat.h"
#include "Shared.h"
#include "afinternal.h"

class ModuleState;
class PacketTable;
struct Marker;
struct MarkerSetup;

struct TrackSetup
{
	int id;

	AudioFormat f;

	bool rateSet, sampleFormatSet, sampleWidthSet, byteOrderSet,
		channelCountSet, compressionSet, aesDataSet, markersSet,
		dataOffsetSet, frameCountSet;

	int markerCount;
	MarkerSetup *markers;

	AFfileoffset dataOffset;
	AFframecount frameCount;
};

struct Track
{
	Track();
	~Track();

	int	id;	/* usually AF_DEFAULT_TRACKID */

	AudioFormat f, v;	/* file and virtual audio formats */

	SharedPtr<PacketTable> m_packetTable;

	double *channelMatrix;

	int markerCount;
	Marker *markers;

	bool hasAESData;	/* Is AES nonaudio data present? */
	unsigned char aesData[24];	/* AES nonaudio data */

	AFframecount totalfframes;		/* frameCount */
	AFframecount nextfframe;		/* currentFrame */
	AFframecount frames2ignore;
	AFfileoffset fpos_first_frame;	/* dataStart */
	AFfileoffset fpos_next_frame;
	AFfileoffset fpos_after_data;
	AFframecount totalvframes;
	AFframecount nextvframe;
	AFfileoffset data_size;		/* trackBytes */

	SharedPtr<ModuleState> ms;

	double taper, dynamic_range;
	bool ratecvt_filter_params_set;

	bool filemodhappy;

	void print();

	Marker *getMarker(int markerID);
	status copyMarkers(TrackSetup *setup);

	void computeTotalFileFrames();
};

#endif
