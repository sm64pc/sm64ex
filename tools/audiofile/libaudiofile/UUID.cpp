/*
	Copyright (C) 2011, Michael Pruett. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

	3. The name of the author may not be used to endorse or promote products
	derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "UUID.h"

#include <stdio.h>
#include <string.h>

bool UUID::operator==(const UUID &u) const
{
	return !memcmp(data, u.data, 16);
}

bool UUID::operator!=(const UUID &u) const
{
	return memcmp(data, u.data, 16) != 0;
}

std::string UUID::name() const
{
	char s[37];
	uint32_t u1 =
		(data[0] << 24) |
		(data[1] << 16) |
		(data[2] << 8) |
		data[3];
	uint16_t u2 =
		(data[4] << 8) |
		data[5];
	uint16_t u3 =
		(data[6] << 8) |
		data[7];
	uint16_t u4 =
		(data[8] << 8) |
		data[9];
	snprintf(s, 37, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
		u1, u2, u3, u4,
		data[10], data[11], data[12], data[13], data[14], data[15]);
	return std::string(s);
}
