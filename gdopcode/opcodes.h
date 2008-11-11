/*====================================================================

$Id: opcodes.h,v 1.3 2008-11-11 01:04:26 wntrmute Exp $

project:      GameCube DSP Tool (gcdsp)
mail:		  duddie@walla.com

Copyright (c) 2005 Duddie

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

$Log: not supported by cvs2svn $
Revision 1.2  2005/09/14 02:19:29  wntrmute
added header guards
use standard main function

Revision 1.1  2005/08/24 22:13:34  wntrmute
Initial import


====================================================================*/

#ifndef _opcodes_h_
#define _opcodes_h_

#include "dtypes.h"

typedef enum partype_t
{
	P_NONE		= 0x0000,
	P_VAL		= 0x0001,
	P_IMM		= 0x0002,
	P_MEM		= 0x0003,
	P_STR		= 0x0004,
	P_REG		= 0x8000,
	P_REG08		= P_REG | 0x0800,
	P_REG10		= P_REG | 0x1000,
	P_REG18		= P_REG | 0x1800,
	P_REG19		= P_REG | 0x1900,
	P_REGM18	= P_REG | 0x1810, // used in multiply instructions
	P_REGM19	= P_REG | 0x1910, // used in multiply instructions
	P_REG1A		= P_REG | 0x1a00,
	P_REG1C		= P_REG | 0x1c00,
	P_ACC		= P_REG | 0x1c10, // used for global accum
	P_ACCD		= P_REG | 0x1c80,
	P_ACCMID	= P_REG | 0x1e00, // used for mid accum
	P_REGS_MASK = 0x01f80,
	P_REF		= P_REG | 0x4000,
	P_PRG		= P_REF | P_REG,
};

#define P_EXT	0x80

typedef struct opcpar_t
{
	partype_t	type;
	uint8	size;
	uint8	loc;
	sint8	lshift;
	uint16	mask;
} opcpar_t;

typedef struct opc_t
{
	char	*name;
	uint16	opcode;
	uint16	opcode_mask;
	uint8	size;
	uint8	param_count;
	opcpar_t	params[8];
} opc_t;

extern opc_t opcodes[];
extern const uint32 opcodes_size;
extern opc_t opcodes_ext[];
extern const uint32 opcodes_ext_size;

inline uint16 swap16(uint16 x)
{
	return (x >> 8) | (x << 8);
}

#endif

