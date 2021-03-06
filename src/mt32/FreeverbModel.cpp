/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011, 2012, 2013 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mt32emu.h"
#include "FreeverbModel.h"
#include "FileStream.h"

#include "freeverb/revmodel.h"

namespace MT32Emu {

FreeverbModel::FreeverbModel(float useScaleTuning, float useFiltVal, float useWet, Bit8u useRoom, float useDamp)
    :freeverb(NULL), scaleTuning(useScaleTuning), filtVal(useFiltVal), wet(useWet), room(useRoom), damp(useDamp) {
}

FreeverbModel::~FreeverbModel() {
	delete freeverb;
}

void FreeverbModel::open() {
	if (freeverb == NULL) {
		freeverb = new revmodel(scaleTuning);
	}
	freeverb->mute();

	// entrance Lowpass filter factor
	freeverb->setfiltval(filtVal);

	// decay speed of high frequencies in the wet signal
	freeverb->setdamp(damp);
}

void FreeverbModel::close() {
	delete freeverb;
	freeverb = NULL;
}

void FreeverbModel::process(const float *inLeft, const float *inRight, float *outLeft, float *outRight, unsigned long numSamples) {
	freeverb->process(inLeft, inRight, outLeft, outRight, numSamples);
}

void FreeverbModel::setParameters(Bit8u time, Bit8u level) {
	// wet signal level
	// FIXME: need to implement some sort of reverb level ramping
	freeverb->setwet((float)level / 7.0f * wet);

	// wet signal decay speed
	static float roomTable[] = {
		 0.25f,  0.37f, 0.54f, 0.71f, 0.78f, 0.86f, 0.93f, 1.00f,
		-1.00f, -0.50f, 0.00f, 0.30f, 0.51f, 0.64f, 0.77f, 0.90f,
		 0.50f,  0.57f, 0.70f, 0.77f, 0.85f, 0.93f, 0.96f, 1.01f};
	freeverb->setroomsize(roomTable[8 * room + time]);
}

bool FreeverbModel::isActive() const {
	// FIXME: Not bothering to do this properly since we'll be replacing Freeverb soon...
	return false;
}


void FreeverbModel::saveState( std::ostream &stream ) {
	Bit8u ptr_valid;


	if( freeverb == NULL ) {
		ptr_valid = 0xff;
		stream.write(reinterpret_cast<const char*>(&ptr_valid), sizeof(ptr_valid) );
	}
	else {
		ptr_valid = 0;
		stream.write(reinterpret_cast<const char*>(&ptr_valid), sizeof(ptr_valid) );

		freeverb->saveState( stream );
	}


	stream.write(reinterpret_cast<const char*>(&scaleTuning), sizeof(scaleTuning) );
	stream.write(reinterpret_cast<const char*>(&filtVal), sizeof(filtVal) );
	stream.write(reinterpret_cast<const char*>(&wet), sizeof(wet) );
	stream.write(reinterpret_cast<const char*>(&room), sizeof(room) );
	stream.write(reinterpret_cast<const char*>(&damp), sizeof(damp) );
}


void FreeverbModel::loadState( std::istream &stream ) {
	Bit8u ptr_valid;


	stream.read(reinterpret_cast<char*>(&ptr_valid), sizeof(ptr_valid) );


	if( ptr_valid == 0xff ) {
		if( freeverb ) delete freeverb;
		freeverb = NULL;
	}
	else {
		if( freeverb == NULL ) freeverb = new revmodel(scaleTuning);
		freeverb->loadState( stream );
	}


	stream.read(reinterpret_cast<char*>(&scaleTuning), sizeof(scaleTuning) );
	stream.read(reinterpret_cast<char*>(&filtVal), sizeof(filtVal) );
	stream.read(reinterpret_cast<char*>(&wet), sizeof(wet) );
	stream.read(reinterpret_cast<char*>(&room), sizeof(room) );
	stream.read(reinterpret_cast<char*>(&damp), sizeof(damp) );
}

}
