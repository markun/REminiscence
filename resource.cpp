/* REminiscence - Flashback interpreter
 * Copyright (C) 2005 Gregory Montoir
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "file.h"
#include "unpack.h"
#include "resource.h"


Resource::Resource(const char *dataPath) {
	memset(this, 0, sizeof(Resource));
	_dataPath = dataPath;
	_memBuf = (uint8 *)malloc(600 * 1024); // XXX reconsider this value
}

Resource::~Resource() {
	clearLevelRes();
	free(_fnt);
	free(_icn);
	free(_tab);
	free(_spr1);
	free(_memBuf);
}

void Resource::clearLevelRes() {
	free(_tbn); _tbn = 0;
	free(_mbk); _mbk = 0;
	free(_mbkData); _mbkData = 0;
	free(_pal); _pal = 0;
	free(_map); _map = 0;
	free(_spc); _spc = 0;
	free(_ani); _ani = 0;
	free_OBJ();
}

void Resource::load_MAP_menu(const char *fileName, uint8 *dstPtr) {
	debug(DBG_RES, "Resource::load_MAP_menu('%s')", fileName);
	sprintf(_entryName, "%s.map", fileName);
	File f;
	if (f.open(_entryName, _dataPath, "rb")) {
		assert(f.size() == 0x3800 * 4);
		f.read(dstPtr, 0x3800 * 4);
		if (f.ioErr()) {
			error("i/o error when reading '%s'", _entryName);
		}
	} else {
		error("Can't open '%s'", _entryName);
	}
}

void Resource::load_PAL_menu(const char *fileName, uint8 *dstPtr) {
	debug(DBG_RES, "Resource::load_PAL_menu('%s')", fileName);
	sprintf(_entryName, "%s.pal", fileName);
	File f;
	if (f.open(_entryName, _dataPath, "rb")) {
		assert(f.size() == 768);
		f.read(dstPtr, 768);
		if (f.ioErr()) {
			error("i/o error when reading '%s'", _entryName);
		}
	} else {
		error("Can't open '%s'", _entryName);
	}
}

void Resource::load_SPR_OFF(const char *fileName, uint8 *sprData) {
	debug(DBG_RES, "Resource::load_SPR_OFF('%s')", fileName);
	sprintf(_entryName, "%s.off", fileName);
	File f;
	if (f.open(_entryName, _dataPath, "rb")) {
		int len = f.size();
		uint8 *offData = (uint8 *)malloc(len);
		assert(offData);
		f.read(offData, len);
		if (f.ioErr()) {
			error("i/o error when reading '%s'", _entryName);
		}
		const uint8 *p = offData;
		uint16 pos;
		while ((pos = READ_LE_UINT16(p)) != 0xFFFF) {
			uint32 off = READ_LE_UINT32(p + 2);
			if (off == 0xFFFFFFFF) {
				_spr_off[pos] = 0;
			} else {
				_spr_off[pos] = sprData + off;
			}
			p += 6;
		}
		free(offData);
	} else {
		error("Can't open '%s'", _entryName);
	}
}

void Resource::load(const char *objName, int objType) {
	debug(DBG_RES, "Resource::load('%s', %d)", objName, objType);
	LoadStub loadStub = NULL;
	switch (objType) {
	case OT_MBK:
		debug(DBG_RES, "chargement bank (mbk)");
		sprintf(_entryName, "%s.mbk", objName);
		loadStub = &Resource::load_MBK;
		break;
	case OT_PGE:
		debug(DBG_RES, "chargement piege (pge)");
		sprintf(_entryName, "%s.pge", objName);
		loadStub = &Resource::load_PGE;
		break;
	case OT_PAL:
		debug(DBG_RES, "chargement palettes (pal)");
		sprintf(_entryName, "%s.pal", objName);
		loadStub = &Resource::load_PAL;
		break;
	case OT_CT:
		debug(DBG_RES, "chargement collision (ct)");
		sprintf(_entryName, "%s.ct", objName);
		loadStub = &Resource::load_CT;
		break;
	case OT_MAP:
		debug(DBG_RES, "ouverture map (map)");
		sprintf(_entryName, "%s.map", objName);
		loadStub = &Resource::load_MAP;
		break;
	case OT_SPC:
		debug(DBG_RES, "chargement sprites caracteres (spc)");
		strcpy(_entryName, "global.spc");
		loadStub = &Resource::load_SPC;
		break;
	case OT_RP:
		debug(DBG_RES, "chargement positions des banks pour sprite_car (rp)");
		sprintf(_entryName, "%s.rp", objName);
		loadStub = &Resource::load_RP;
		break;
	case OT_SPR:
		debug(DBG_RES, "chargement des sprites (spr)");
		sprintf(_entryName, "%s.spr", objName);
		loadStub = &Resource::load_SPR;
		break;
	case OT_SPRM:
		debug(DBG_RES, "chargement des sprites monstre (spr)");
		sprintf(_entryName, "%s.spr", objName);
		loadStub = &Resource::load_SPRM;
		break;
	case OT_ICN:
		debug(DBG_RES, "chargement des icones (icn)");
		sprintf(_entryName, "%s.icn", objName);
		loadStub = &Resource::load_ICN;
		break;
	case OT_FNT:
		debug(DBG_RES, "chargement de la font (fnt)");
		strcpy(_entryName, objName);
		loadStub = &Resource::load_FNT;
		break;
	case OT_OBJ:
		debug(DBG_RES, "chargement objets (obj)");
		sprintf(_entryName, "%s.obj", objName);
		loadStub = &Resource::load_OBJ;
		break;
	case OT_ANI:
		debug(DBG_RES, "chargement animations (ani)");
		sprintf(_entryName, "%s.ani", objName);
		loadStub = &Resource::load_ANI;
		break;
	case OT_TBN:
		debug(DBG_RES, "chargement des textes (tbn)");
		sprintf(_entryName, "%s.tbn", objName);
		loadStub = &Resource::load_TBN;
		break;
	default:
		error("unimplemented Resource::load() type %d", objType);
		break;
	}
	if (loadStub) {
		File f;
		if (f.open(_entryName, _dataPath, "rb")) {
			(this->*loadStub)(&f);
			if (f.ioErr()) {
				error("i/o error when reading '%s'", _entryName);
			}
		} else {
			error("Can't open '%s'", _entryName);
		}
	}
}

void Resource::load_CT(File *pf) {
	debug(DBG_RES, "Resource::load_CT()");
	int len = pf->size();
	uint8 *tmp = (uint8 *)malloc(len);
	assert(tmp);
	pf->read(tmp, len);

	Unpack unp;
	int decSize;
	bool ret = unp.unpack((uint8 *)_ctData, tmp, len, decSize);
	assert(ret && decSize == 0x1D00);

	free(tmp);
}

void Resource::load_FNT(File *f) {
	debug(DBG_RES, "Resource::load_FNT()");
	int len = f->size();
	_fnt = (uint8 *)malloc(len);
	assert(_fnt);
	f->read(_fnt, len);
}

void Resource::load_MBK(File *f) {
	debug(DBG_RES, "Resource::load_MBK()");
	uint8 num = f->readByte();
	int data_size = f->size() - num * 6;
	_mbk = (MbkEntry *)malloc(sizeof(MbkEntry) * num);
	assert(_mbk);
	f->seek(0);
	for (int i = 0; i < num; ++i) {
		f->readUint16BE(); /* unused */
		_mbk[i].offset = f->readUint16BE() - num * 6;
		_mbk[i].len = f->readUint16BE();
		debug(DBG_RES, "data_size = 0x%X entry %d off = 0x%X len = 0x%X", data_size, i, _mbk[i].offset + num * 6, _mbk[i].len);
		assert(_mbk[i].offset <= data_size);
	}
	_mbkData = (uint8 *)malloc(data_size);
	f->read(_mbkData, data_size);
}

void Resource::load_ICN(File *f) {
	debug(DBG_RES, "Resource::load_ICN()");
	int len = f->size();
	_icn = (uint8 *)malloc(len);
	assert(_icn);
	f->read(_icn, len);
}

void Resource::load_SPR(File *f) {
	debug(DBG_RES, "Resource::load_SPR()");
	int len = f->size() - 12;
	_spr1 = (uint8 *)malloc(len);
	assert(_spr1);
	f->seek(12);
	f->read(_spr1, len);
}

void Resource::load_SPRM(File *f) {
	debug(DBG_RES, "Resource::load_SPRM()");
	int len = f->size() - 12;
	f->seek(12);
	f->read(_sprm, len);
}

void Resource::load_RP(File *f) {
	debug(DBG_RES, "Resource::load_RP()");
	f->read(_rp, 0x4A);
}

void Resource::load_SPC(File *f) {
	debug(DBG_RES, "Resource::load_SPC()");
	int len = f->size();
	_spc = (uint8 *)malloc(len);
	assert(_spc);
	f->read(_spc, len);
	_numSpc = READ_BE_UINT16(_spc) / 2;
}

void Resource::load_PAL(File *f) {
	debug(DBG_RES, "Resource::load_PAL()");
	int len = f->size();
	_pal = (uint8 *)malloc(len);
	assert(_pal);
	f->read(_pal, len);
}

void Resource::load_MAP(File *f) {
	debug(DBG_RES, "Resource::load_MAP()");
	int len = f->size();
	_map = (uint8 *)malloc(len);
	assert(_map);
	f->read(_map, len);
}

void Resource::load_OBJ(File *f) {
	debug(DBG_RES, "Resource::load_OBJ()");

	uint16 i;

	_numObjectNodes = f->readUint16LE();
	assert(_numObjectNodes < 255);

	uint32 offsets[256];
	for (i = 0; i < _numObjectNodes; ++i) {
		offsets[i] = f->readUint32LE();
	}
	offsets[i] = f->size() - 2;

	int numObjectsCount = 0;
	uint16 objectsCount[256];
	for (i = 0; i < _numObjectNodes; ++i) {
		int diff = offsets[i + 1] - offsets[i];
		if (diff != 0) {
			objectsCount[numObjectsCount] = (diff - 2) / 0x12;
			debug(DBG_RES, "i = %d objectsCount[numObjectsCount] = %d", i, objectsCount[numObjectsCount]);
			++numObjectsCount;
		}
	}

	uint32 prevOffset = 0;
	ObjectNode *prevNode = NULL;
	int iObj = 0;
	for (i = 0; i < _numObjectNodes; ++i) {
		if (prevOffset != offsets[i]) {
			ObjectNode *on = (ObjectNode *)malloc(sizeof(ObjectNode));
			f->seek(offsets[i] + 2);
			on->last_obj_number = f->readUint16LE();
			on->num_objects = objectsCount[iObj];
			on->objects = (Object *)malloc(sizeof(Object) * on->num_objects);
			for (uint16 j = 0; j < on->num_objects; ++j) {
				Object *obj = &on->objects[j];
				obj->type = f->readUint16LE();
				obj->dx = f->readByte();
				obj->dy = f->readByte();
				obj->init_obj_type = f->readUint16LE();
				obj->opcode2 = f->readByte();
				obj->opcode1 = f->readByte();
				obj->flags = f->readByte();
				obj->opcode3 = f->readByte();
				obj->init_obj_number = f->readUint16LE();
				obj->opcode_arg1 = f->readUint16LE();
				obj->opcode_arg2 = f->readUint16LE();
				obj->opcode_arg3 = f->readUint16LE();
				debug(DBG_RES, "obj_node=%d obj=%d op1=0x%X op2=0x%X op3=0x%X", i, j, obj->opcode2, obj->opcode1, obj->opcode3);
			}
			++iObj;
			prevOffset = offsets[i];
			prevNode = on;
		}
		_objectNodesMap[i] = prevNode;
	}
}

void Resource::load_PGE(File *f) {
	debug(DBG_RES, "Resource::load_PGE()");
	int len = f->size() - 2;
	_pgeNum = f->readUint16LE();
	memset(_pgeInit, 0, sizeof(_pgeInit));
	debug(DBG_RES, "len = %d _pgeNum = %d mod = %d", len, _pgeNum, len % 0x20);
	for (uint16 i = 0; i < _pgeNum; ++i) {
		InitPGE *pge = &_pgeInit[i];
		pge->type = f->readUint16LE();
		pge->pos_x = f->readUint16LE();
		pge->pos_y = f->readUint16LE();
		pge->obj_node_number = f->readUint16LE();
		pge->life = f->readUint16LE();
		for (int lc = 0; lc < 4; ++lc) {
			pge->counter_values[lc] = f->readUint16LE();
		}
		pge->object_type = f->readByte();
		pge->init_room = f->readByte();
		pge->room_location = f->readByte();
		pge->init_flags = f->readByte();
		pge->unk16 = f->readByte();
		pge->icon_num = f->readByte();
		pge->object_id = f->readByte();
		pge->skill = f->readByte();
		pge->mirror_x = f->readByte();
		pge->flags = f->readByte();
		pge->unk1C = f->readByte();
		pge->unk1D = f->readByte();
		pge->text_num = f->readByte();
		pge->unk1F = f->readByte();
	}
}

void Resource::load_ANI(File *f) {
	debug(DBG_RES, "Resource::load_ANI()");
	int len = f->size() - 2;
	_ani = (uint8 *)malloc(len);
	assert(_ani);
	f->seek(2);
	f->read(_ani, len);
}

void Resource::load_TBN(File *f) {
	debug(DBG_RES, "Resource::load_TBN()");
	int len = f->size();
	_tbn = (uint8 *)malloc(len);
	assert(_tbn);
	f->read(_tbn, len);
}

void Resource::free_OBJ() {
	debug(DBG_RES, "Resource::free_OBJ()");
	ObjectNode *prevNode = NULL;
	for (int i = 0; i < _numObjectNodes; ++i) {
		if (_objectNodesMap[i] != prevNode) {
			ObjectNode *curNode = _objectNodesMap[i];
			free(curNode->objects);
			_objectNodesMap[i] = NULL;
			prevNode = curNode;
		}
	}
}
