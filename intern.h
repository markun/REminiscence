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

#ifndef __INTERN_H__
#define __INTERN_H__

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "sys.h"
#include "util.h"

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

template<typename T>
inline void SWAP(T &a, T &b) {
	T tmp = a;
	a = b;
	b = tmp;
}

struct Color {
	uint8 r;
	uint8 g;
	uint8 b;
};

struct Level {
	const char *name;
	const char *name2;
	uint16 cutscene_id;
};

struct InitPGE {
	uint16 type;
	int16 pos_x;
	int16 pos_y;
	uint16 obj_node_number;
	uint16 life;
	int16 counter_values[4];
	uint8 object_family;
	uint8 init_room;
	uint8 room_location;
	uint8 init_flags;
	uint8 unk16;
	uint8 icon_num;
	uint8 state;
	uint8 skill;
	uint8 unk1A;
	uint8 flags;
	uint8 unk1C; // collidable, collision_data_len
	uint8 unk1D;
	uint8 text_num;
	uint8 unk1F;
};

struct LivePGE {
	uint16 obj_type;
	int16 pos_x;
	int16 pos_y;
	uint8 anim_seq;
	uint8 room_location;
	int16 life;
	int16 counter_value;
	uint8 collision_slot;
	uint8 next_inventory_PGE;
	uint8 current_inventory_PGE;
	uint8 unkF; // unk_inventory_PGE
	uint16 anim_number;
	uint8 flags;
	uint8 index;
	uint16 first_obj_number;
	LivePGE *next_PGE_in_room;
	InitPGE *init_PGE;
};

struct GroupPGE {
	GroupPGE *next_entry;
	uint16 index;
	uint16 group_id;
};

struct Object {
	uint16 type;
	int8 dx;
	int8 dy;
	uint16 init_obj_type;
	uint8 opcode2;
	uint8 opcode1;
	uint8 flags;
	uint8 opcode3;
	uint16 init_obj_number;
	int16 opcode_arg1;
	int16 opcode_arg2;
	int16 opcode_arg3;
};

struct ObjectNode {
	uint16 last_obj_number;
	Object *objects;
	uint16 num_objects;
};

struct ObjectOpcodeArgs {
	LivePGE *pge; // arg0
	int16 a; // arg2
	int16 b; // arg4
};

struct AnimBufferState {
	int16 x;
	int16 y;
	const uint8 *dataPtr;
	LivePGE *pge;
};

struct AnimBuffers {
	AnimBufferState *_states[4];
	uint8 _curPos[4];

	void addState(int stateNum, int16 x, int16 y, const uint8 *dataPtr, LivePGE *pge);
};

struct CollisionSlot {
	int16 ct_pos;
	CollisionSlot *prev_slot;
	LivePGE *live_pge;
	uint16 index;
};

struct MbkEntry {
	uint16 offset;
	uint16 len;
};

struct BankSlot {
	uint16 entryNum;
	uint8 *ptr;
};

struct DirtyBlock {
	uint16 start_x;
	uint8 w;
	uint8 h;
};

struct CollisionSlot2 {
	CollisionSlot2 *next_slot;
	int8 *unk2;
	uint8 data_size;
	uint8 data_buf[0x10]; // XXX check size
};

struct InventoryItem {
	uint8 icon_num;
	InitPGE *init_pge;
	LivePGE *live_pge;
};

#endif
