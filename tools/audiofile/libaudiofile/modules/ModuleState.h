/*
	Audio File Library
	Copyright (C) 2000, Silicon Graphics, Inc.
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

#ifndef MODULESTATE_H
#define MODULESTATE_H

#include "Module.h"
#include "Shared.h"
#include "afinternal.h"
#include <vector>

class FileModule;
class Module;

class ModuleState : public Shared<ModuleState>
{
public:
	ModuleState();
	virtual ~ModuleState();

	bool isDirty() const { return m_isDirty; }
	void setDirty() { m_isDirty = true; }
	status init(AFfilehandle file, Track *track);
	status setup(AFfilehandle file, Track *track);
	status reset(AFfilehandle file, Track *track);
	status sync(AFfilehandle file, Track *track);

	int numModules() const { return m_modules.size(); }
	const std::vector<SharedPtr<Module> > &modules() const;
	const std::vector<SharedPtr<Chunk> > &chunks() const;

	bool mustUseAtomicNVFrames() const { return true; }

	void print();

	bool fileModuleHandlesSeeking() const;

private:
	std::vector<SharedPtr<Module> > m_modules;
	std::vector<SharedPtr<Chunk> > m_chunks;
	bool m_isDirty;

	SharedPtr<FileModule> m_fileModule;
	SharedPtr<Module> m_fileRebufferModule;

	status initFileModule(AFfilehandle file, Track *track);

	status arrange(AFfilehandle file, Track *track);

	void addModule(Module *module);

	void addConvertIntToInt(FormatCode input, FormatCode output);
	void addConvertIntToFloat(FormatCode input, FormatCode output);
	void addConvertFloatToInt(FormatCode input, FormatCode output,
		const PCMInfo &inputMapping, const PCMInfo &outputMapping);
	void addConvertFloatToFloat(FormatCode input, FormatCode output);
};

#endif
