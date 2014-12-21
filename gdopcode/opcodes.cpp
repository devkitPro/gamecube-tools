/*====================================================================

$Id: opcodes.cpp,v 1.3 2008-11-18 14:43:34 shagkur Exp $

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
Revision 1.2  2008/11/11 01:04:26  wntrmute
add Hermes' patches

Revision 1.4  2008/10/04 10:30:00  Hermes
Revision 1.1  2005/08/24 22:13:34  wntrmute
Initial import


====================================================================*/

/* NOTES BY HERMES:

LZ flag: original opcodes andf and andcf are swaped. Also "jzr" and "jnz" are swaped but now named 'jlz' and 'jlnz' 
As you can see it obtain the same result but now LZ=1 correctly

Added conditional instructions:

conditional names:

NZ -> NOT ZERO
Z  -> ZERO

NC -> NOT CARRY
C  ->  CARRY

LZ  -> LOGIC ZERO (only used with andcf-andf instructions?)
LNZ -> LOGIC NOT ZERO

GT -> GREATER THAN
LE -> LESS EQUAL

GE -> GREATER EQUAL
LT -> LESS THAN

Examples:

jnz, ifs, retlnz

*/
#include "opcodes.h"

reg_t registers[] =
{
	{ "AR0", 0x00 },
	{ "AR1", 0x01 },
	{ "AR2", 0x02 },
	{ "AR3", 0x03 },
	{ "IX0", 0x04 },
	{ "IX1", 0x05 },
	{ "IX2", 0x06 },
	{ "IX3", 0x07 },
	{ "WR0", 0x08 },
	{ "WR1", 0x09 },
	{ "WR2", 0x0a },
	{ "WR3", 0x0b },
	{ "ST0", 0x0c },
	{ "ST1", 0x0d },
	{ "ST2", 0x0e },
	{ "ST3", 0x0f },
	{ "ACC0.H", 0x10 },
	{ "ACC1.H", 0x11 },
	{ "CONFIG", 0x12 },
	{ "STATUS", 0x13 },
	{ "PROD.L", 0x14 },
	{ "PROD.M1", 0x15 },
	{ "PROD.H", 0x16 },
	{ "PROD.M2", 0x17 },
	{ "ACX0.L", 0x18 },
	{ "ACX1.L", 0x19 },
	{ "ACX0.H", 0x1a },
	{ "ACX1.H", 0x1b },
	{ "ACC0.L", 0x1c },
	{ "ACC1.L", 0x1d },
	{ "ACC0.M", 0x1e },
	{ "ACC1.M", 0x1f },

	{ "ACX0", 0x2018 },
	{ "ACX1", 0x2019 },

	{ "ACC0", 0x201c },
	{ "ACC1", 0x201d },
};

opc_t opcodes[] =
{
	{ "NOP",	0x0000, 0xffff, 1, 0, {}, },
	{ "DAR",	0x0004, 0xfffc, 1, 1, {{P_REG, 1, 0, 0, 0x0003}}, },
	{ "IAR",	0x0008, 0xfffc, 1, 1, {{P_REG, 1, 0, 0, 0x0003}}, },
	{ "ADDARN",	0x0010, 0xfff0, 1, 2, {{P_REG, 1, 0, 0, 0x0003}, {P_REG04, 1, 0, 2, 0x000c}}, },

	{ "HALT",	0x0021, 0xffff, 1, 0, {}, },

	{ "IFGE",	0x0270, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFLT",	0x0271, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFGT",	0x0272, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFLE",	0x0273, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFNE",	0x0274, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFEQ",	0x0275, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFNC",	0x0276, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFC",	0x0277, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFLNZ",	0x027c, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFLZ",	0x027d, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IFOV",	0x027e, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "IF",		0x027f, 0xffff, 1, 0, {}, }, // new by Hermes

	{ "JGE",	0x0290, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "JLT",	0x0291, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "JGT",	0x0292, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "JLE",	0x0293, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "JNE",	0x0294, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },
	{ "JEQ",	0x0295, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },
	{ "JNC",	0x0296, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "JC",		0x0297, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "JLNZ",	0x029c, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },
	{ "JLZ",	0x029d, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },
	{ "JOV",	0x029e, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },
	{ "JMP",	0x029f, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },

	{ "CALLGE", 0x02b0, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLLT",	0x02b1, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLGT",	0x02b2, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLLE",	0x02b3, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLNE",	0x02b4, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },
	{ "CALLEQ",	0x02b5, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLNC",	0x02b6, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLC",	0x02b7, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLLNZ",0x02bc, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLLZ", 0x02bd, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALLOV", 0x02be, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, }, // new by Hermes
	{ "CALL",	0x02bf, 0xffff, 2, 1, {{P_ADDR_I, 2, 1, 0, 0xffff}}, },

	{ "RETGE",	0x02d0, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETLT",	0x02d1, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETGT",	0x02d2, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETLE",	0x02d3, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETNE",	0x02d4, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETEQ",	0x02d5, 0xffff, 1, 0, {}, },
	{ "RETNC",	0x02d6, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETC",	0x02d7, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETLNZ",	0x02dc, 0xffff, 1, 0, {}, }, // new by Hermes
	{ "RETLZ",	0x02dd, 0xffff, 1, 0, {}, },
	{ "RETOV",	0x02de, 0xffff, 1, 0, {}, },
	{ "RET",	0x02df, 0xffff, 1, 0, {}, },

	{ "RTI",	0x02ff, 0xffff, 1, 0, {}, },

	{ "JRGE",	0x1700, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRLT",	0x1701, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRGT",	0x1702, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRLE",	0x1703, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRNE",	0x1704, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JREQ",	0x1705, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRNC",	0x1706, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRC",	0x1707, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRLNZ",	0x170c, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JRLZ",	0x170d, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JROV",	0x170e, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "JMPR",	0x170f, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },

	{ "CALLRGE",	0x1710, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRLT",	0x1711, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRGT",	0x1712, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRLE",	0x1713, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRNE",	0x1714, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLREQ",	0x1715, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRNC",	0x1716, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRC",		0x1717, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRLNZ",	0x171c, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLRLZ",	0x171d, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLROV",	0x171e, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },  // new by Hermes
	{ "CALLR",		0x171f, 0xff1f, 1, 1, {{P_REG, 1, 0, 5, 0x00e0}}, },

	{ "SBCLR",	0x1200, 0xfff8, 1, 1, {{P_IMM, 1, 0, 0, 0x0007}}, },
	{ "SBSET",	0x1300, 0xfff8, 1, 1, {{P_IMM, 1, 0, 0, 0x0007}}, },

	{ "LSL",	0x1400, 0xfec0, 1, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}}, },
	{ "LSR",	0x1440, 0xfec0, 1, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}}, },
	{ "ASL",	0x1480, 0xfec0, 1, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}}, },
	{ "ASR",	0x14c0, 0xfec0, 1, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x003f}}, },

	{ "LRI",	0x0080, 0xffe0, 2, 2, {{P_REG, 1, 0, 0, 0x001f}, {P_IMM, 2, 1, 0, 0xffff}}, },
	{ "LR",		0x00c0,	0xffe0, 2, 2, {{P_REG, 1, 0, 0, 0x001f}, {P_MEM, 2, 1, 0, 0xffff}}, },
	{ "SR",		0x00e0,	0xffe0, 2, 2, {{P_MEM, 2, 1, 0, 0xffff}, {P_REG, 1, 0, 0, 0x001f}}, },

	{ "MRR",	0x1c00, 0xfc00, 1, 2, {{P_REG, 1, 0, 5, 0x03e0}, {P_REG, 1, 0, 0, 0x001f}}, },

	{ "SI",		0x1600, 0xff00, 2, 2, {{P_MEM, 1, 0, 0, 0x00ff}, {P_IMM, 2, 1, 0, 0xffff}}, },

	{ "ADDIS",	0x0400, 0xfe00, 1, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x00ff}}, },
	{ "CMPIS",	0x0600, 0xfe00, 1, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 1, 0, 0, 0x00ff}}, },
	{ "LRIS",	0x0800, 0xf800, 1, 2, {{P_REG18, 1, 0, 8, 0x0700}, {P_IMM, 1, 0, 0, 0x00ff}}, },

	{ "ADDI",	0x0200, 0xfeff, 2, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}}, },
	{ "XORI",	0x0220, 0xfeff, 2, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}}, },
	{ "ANDI",	0x0240, 0xfeff, 2, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}}, },
	{ "ORI",	0x0260, 0xfeff, 2, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}}, },
	{ "CMPI",	0x0280, 0xfeff, 2, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}}, },
	{ "ANDF",	0x02a0, 0xfeff, 2, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}}, },
	{ "ANDCF",	0x02c0, 0xfeff, 2, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_IMM, 2, 1, 0, 0xffff}}, }, 

	{ "ILRR",	0x0210, 0xfedc, 1, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "ILRRD",	0x0214, 0xfedc, 1, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "ILRRI",	0x0218, 0xfedc, 1, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "ILRRN",	0x021c, 0xfedc, 1, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_PRG, 1, 0, 0, 0x0003}}, },

	{ "LOOP",	0x0040,	0xffe0, 1, 1, {{P_REG, 1, 0, 0, 0x001f}}, },
	{ "BLOOP",	0x0060,	0xffe0, 2, 2, {{P_REG, 1, 0, 0, 0x001f}, {P_VAL, 2, 1, 0, 0xffff}}, },
	{ "LOOPI",	0x1000, 0xff00, 1, 1, {{P_IMM, 1, 0, 0, 0x00ff}}, },
	{ "BLOOPI",	0x1100, 0xff00, 2, 2, {{P_IMM, 1, 0, 0, 0x00ff}, {P_VAL, 2, 1, 0, 0xffff}}, },

	// load and store value pointed by indexing reg and increment; LRR/SRR variants
	{ "LRR",	0x1800, 0xff80, 1, 2, {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}}, },
	{ "LRRD",	0x1880, 0xff80, 1, 2, {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}}, },
	{ "LRRI",	0x1900, 0xff80, 1, 2, {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}}, },
	{ "LRRN",	0x1980, 0xff80, 1, 2, {{P_REG, 1, 0, 0, 0x001f}, {P_PRG, 1, 0, 5, 0x0060}}, },

	{ "SRR",	0x1a00, 0xff80, 1, 2, {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}}, },
	{ "SRRD",	0x1a80, 0xff80, 1, 2, {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}}, },
	{ "SRRI",	0x1b00, 0xff80, 1, 2, {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}}, },
	{ "SRRN",	0x1b80, 0xff80, 1, 2, {{P_PRG, 1, 0, 5, 0x0060}, {P_REG, 1, 0, 0, 0x001f}}, },

	{ "LRS",	0x2000, 0xf800, 1, 2, {{P_REG18, 1, 0, 8, 0x0700}, {P_MEM, 1, 0, 0, 0x00ff}}, },
	{ "SRS",	0x2800, 0xf800, 1, 2, {{P_MEM, 1, 0, 0, 0x00ff}, {P_ACCLM, 1, 0, 8, 0x0700}}, }, // for SRS only ACCx.L/.M works here.
																								 // As described in the dolphin DSP sources
																								 // the final address is calculated by taking the botton 8 bits
																								 // from the $config register (shifted by 8 to the right) and or'd 
																								 // with the 8-bit immediat value. Hence those functions are mostly 
																								 // used for operations on device registers like mailboxes etc.
																								 // Also this explains why we see lri $config,#0x00ff.


	// opcodes that can be extended
	// extended opcodes, note size of opcode will be set to 0

	{ "XORR",	0x3000, 0xfc80, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACXXH, 1, 0, 9, 0x0200}}, },
	{ "XORC",	0x3080, 0xfe80, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}}, },
	{ "NOT",	0x3280, 0xfe80, 1 | P_EXT, 1, {{P_ACCM, 1, 0, 8, 0x0100}}, },
	{ "ANDR",	0x3400, 0xfc80, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACXXH, 1, 0, 9, 0x0200}}, },
	{ "LSRNRX",	0x3480, 0xfc80, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACXXH, 1, 0, 9, 0x200}}, },
	{ "ORR",	0x3800, 0xfc80, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACXXH, 1, 0, 9, 0x0200}}, },
	{ "ASRNRX",	0x3880, 0xfc80, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACXXH, 1, 0, 9, 0x200}}, },
	{ "ANDC",	0x3c00, 0xfe80, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}}, },
	{ "LSRNR",	0x3c80, 0xfe80, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x100}}, },
	{ "ORC",	0x3e00, 0xfe80, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x0100}}, },
	{ "ASRNR",	0x3e80, 0xfe80, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACCM_D, 1, 0, 8, 0x100}}, },

	{ "ADDR",	0x4000, 0xf800, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0600}}, },
	{ "ADDAX",	0x4800, 0xfc00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACX, 1, 0, 9, 0x0200}}, },
	{ "ADD",	0x4c00, 0xfe00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACC_D, 1, 0, 8, 0x0100}}, },
	{ "ADDP",	0x4e00, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },

	{ "NX",		0x8000, 0xff00, 1 | P_EXT, 0, {}, },
	{ "CLR",	0x8100, 0xf700, 1 | P_EXT, 1, {{P_ACC, 1, 0, 11, 0x0800}}, },
	{ "CMP",	0x8200, 0xff00, 1 | P_EXT, 0, {}, },
	{ "CLRP",	0x8400, 0xff00, 1 | P_EXT, 0, {}, },
	{ "TSTAXH",	0x8600, 0xfe00, 1 | P_EXT, 1, {{P_ACXXH, 1, 0, 8, 0x0100}}, },
	{ "M2",		0x8a00, 0xff00, 1 | P_EXT, 0, {}, },
	{ "M0",		0x8b00, 0xff00, 1 | P_EXT, 0, {}, },
	{ "CLR15",	0x8c00, 0xff00, 1 | P_EXT, 0, {}, },
	{ "SET15",	0x8d00, 0xff00, 1 | P_EXT, 0, {}, },
	{ "S16",	0x8e00, 0xff00, 1 | P_EXT, 0, {}, },
	{ "S40",	0x8f00, 0xff00, 1 | P_EXT, 0, {}, },

	{ "CLRA0",	0x8100, 0xff00, 1 | P_EXT, 0, {}, }, // clear acc0 added by Hermes
	{ "CLRA1",	0x8900, 0xff00, 1 | P_EXT, 0, {}, }, // clear acc1 added by Hermes

	{ "ADDAXL",	0x7000, 0xfc00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0200}}, },
	{ "INCM",	0x7400, 0xfe00, 1 | P_EXT, 1, {{P_ACCM, 1, 0, 8, 0x0100}}, },
	{ "INC",	0x7600, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },
	{ "DECM",	0x7800, 0xfe00, 1 | P_EXT, 1, {{P_ACCM, 1, 0, 8, 0x0100}}, },
	{ "DEC",	0x7a00, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },
	{ "NEG",	0x7c00, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MOVNP",	0x7e00, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },

	{ "ASR16",	0x9100, 0xf700, 1 | P_EXT, 1, {{P_ACC, 1, 0, 11, 0x0800}}, },

	{ "ABS",	0xa100, 0xf700, 1 | P_EXT, 1, {{P_ACC, 1, 0,11, 0x0800}}, },

	{ "TST",	0xb100, 0xf700, 1 | P_EXT, 1, {{P_ACC, 1, 0, 11, 0x0800}}, },
	{ "CMPAXH",	0xc100, 0xe700, 1 | P_EXT, 2, {{P_ACC, 1, 0, 12, 0x1000}, {P_ACXXH, 1, 0, 11, 0x0800}}, },

	{ "CLRAL0",	0xFC00, 0xff00, 1 | P_EXT, 0, {}, }, // clear acl0 added by Hermes
	{ "CLRAL1",	0xFD00, 0xff00, 1 | P_EXT, 0, {}, }, // clear acl1 added by Hermes

	{ "MOVR",	0x6000, 0xf800, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0600}}, },
	{ "MOVAX",	0x6800, 0xfc00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACX, 1, 0, 9, 0x0200}}, },
	{ "MOV",	0x6c00, 0xfe00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACC_D, 1, 0, 8, 0x0100}}, },
	{ "MOVP",	0x6e00, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },

	{ "LSL16",	0xf000, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MADD",	0xf200, 0xfe00, 1 | P_EXT, 2, {{P_ACXXL, 1, 0, 8, 0x0100}, {P_ACXXH, 1, 0, 8, 0x0100}}, },
	{ "LSR16",	0xf400, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MSUB",	0xf600, 0xfe00, 1 | P_EXT, 2, {{P_ACXXL, 1, 0, 8, 0x0100}, {P_ACXXH, 1, 0, 8, 0x0100}}, },
	{ "ADDPAXZ",0xf800, 0xfc00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 9, 0x0200}, {P_ACX, 1, 0, 8, 0x0100}}, },
	{ "MOVPZ",	0xfe00, 0xfe00, 1 | P_EXT, 1, {{P_ACC, 1, 0, 8, 0x0100}}, },



	{ "MULX",	0xa000, 0xe700, 1 | P_EXT, 2, {{P_ACXHL0, 1, 0, 11, 0x1000}, {P_ACXHL1, 1, 0, 10, 0x0800}}, },
	{ "MULXMVZ",0xa200, 0xe600, 1 | P_EXT, 3, {{P_ACXHL0, 1, 0, 11, 0x1000}, {P_ACXHL1, 1, 0, 10, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MULXAC",	0xa400, 0xe600, 1 | P_EXT, 3, {{P_ACXHL0, 1, 0, 11, 0x1000}, {P_ACXHL1, 1, 0, 10, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MULXMV",	0xa600, 0xe600, 1 | P_EXT, 3, {{P_ACXHL0, 1, 0, 11, 0x1000}, {P_ACXHL1, 1, 0, 10, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },

	{ "MUL",	0x9000, 0xf700, 1 | P_EXT, 2, {{P_ACXXL, 1, 0, 11, 0x0800}, {P_ACXXH, 1, 0, 11, 0x0800}}, },
	{ "MULMVZ",	0x9200, 0xf600, 1 | P_EXT, 3, {{P_ACXXL, 1, 0, 11, 0x0800}, {P_ACXXH, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MULAC",	0x9400, 0xf600, 1 | P_EXT, 3, {{P_ACXXL, 1, 0, 11, 0x0800}, {P_ACXXH, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MULMV",	0x9600, 0xf600, 1 | P_EXT, 3, {{P_ACXXL, 1, 0, 11, 0x0800}, {P_ACXXH, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },

	{ "MULC",	0xc000, 0xe700, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 12, 0x1000}, {P_ACXXH, 1, 0, 11, 0x0800}}, },
	{ "MULCMVZ",0xc200, 0xe600, 1 | P_EXT, 3, {{P_ACCM, 1, 0, 12, 0x1000}, {P_ACXXH, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MULCAC",	0xc400, 0xe600, 1 | P_EXT, 3, {{P_ACCM, 1, 0, 12, 0x1000}, {P_ACXXH, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },
	{ "MULCMV",	0xc600, 0xe600, 1 | P_EXT, 3, {{P_ACCM, 1, 0, 12, 0x1000}, {P_ACXXH, 1, 0, 11, 0x0800}, {P_ACC, 1, 0, 8, 0x0100}}, },


	{ "SUBR",	0x5000, 0xf800, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 8, 0x0100}, {P_REG18, 1, 0, 9, 0x0600}}, },
	{ "SUBAX",	0x5800, 0xfc00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACX, 1, 0, 9, 0x0200}}, },
	{ "SUB",	0x5c00, 0xfe00, 1 | P_EXT, 2, {{P_ACC, 1, 0, 8, 0x0100}, {P_ACC_D, 1, 0, 8, 0x0100}}, },

	{ "MADDX",	0xe000, 0xfc00, 1 | P_EXT, 2, {{P_ACXHL0, 1, 0, 8, 0x0200}, {P_ACXHL1, 1, 0, 7, 0x0100}}, },
	{ "MSUBX",	0xe400, 0xfc00, 1 | P_EXT, 2, {{P_ACXHL0, 1, 0, 8, 0x0200}, {P_ACXHL1, 1, 0, 7, 0x0100}}, },
	{ "MADDC",	0xe800, 0xfc00, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 9, 0x0200}, {P_ACXXH, 1, 0, 7, 0x0100}}, },
	{ "MSUBC",	0xec00, 0xfc00, 1 | P_EXT, 2, {{P_ACCM, 1, 0, 9, 0x0200}, {P_ACXXH, 1, 0, 7, 0x0100}}, },


	// assemble CW
	{ "CW",		0x0000, 0xffff, 1, 1, {{P_VAL, 2, 0, 0, 0xffff}}, },
	// unknown opcode for disassemble
	{ "CW",		0x0000, 0x0000, 1, 1, {{P_VAL, 2, 0, 0, 0xffff}}, },

};
opc_t opcodes_ext[] = // revisar
{
	{ "L",		0x0040, 0x00c4, 1, 2, {{P_REG18, 1, 0, 3, 0x0038}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "LN",		0x0044, 0x00c4, 1, 2, {{P_REG18, 1, 0, 3, 0x0038}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "LS",		0x0080, 0x00ce, 1, 2, {{P_REG18, 1, 0, 4, 0x0030}, {P_ACCM, 1, 0, 0, 0x0001}}, },
	{ "LSN",	0x0084, 0x00ce, 1, 2, {{P_REG18, 1, 0, 4, 0x0030}, {P_ACCM, 1, 0, 0, 0x0001}}, },
	{ "LSM",	0x0088, 0x00ce, 1, 2, {{P_REG18, 1, 0, 4, 0x0030}, {P_ACCM, 1, 0, 0, 0x0001}}, },
	{ "LSNM",	0x008c, 0x00ce, 1, 2, {{P_REG18, 1, 0, 4, 0x0030}, {P_ACCM, 1, 0, 0, 0x0001}}, },
	{ "SL",		0x0082, 0x00ce, 1, 2, {{P_ACCM, 1, 0, 0, 0x0001}, {P_REG18, 1, 0, 4, 0x0030}}, },
	{ "SLN",	0x0086, 0x00ce, 1, 2, {{P_ACCM, 1, 0, 0, 0x0001}, {P_REG18, 1, 0, 4, 0x0030}}, },
	{ "SLM",	0x008a, 0x00ce, 1, 2, {{P_ACCM, 1, 0, 0, 0x0001}, {P_REG18, 1, 0, 4, 0x0030}}, },
	{ "SLNM",	0x008e, 0x00ce, 1, 2, {{P_ACCM, 1, 0, 0, 0x0001}, {P_REG18, 1, 0, 4, 0x0030}}, },
	{ "S",		0x0020, 0x00e4, 1, 2, {{P_PRG, 1, 0, 0, 0x0003}, {P_ACCLM, 1, 0, 3, 0x0018}}, },
	{ "SN",		0x0024, 0x00e4, 1, 2, {{P_PRG, 1, 0, 0, 0x0003}, {P_ACCLM, 1, 0, 3, 0x0018}}, },
	{ "LDX",	0x00c0, 0x00cf, 1, 3, {{P_ACXXL, 1, 0, 4, 0x0010}, {P_ACXXH, 1, 0, 4, 0x0010}, {P_PRG, 1, 0, 5, 0x0020}}, },
	{ "LDXN",	0x00c4, 0x00cf, 1, 3, {{P_ACXXL, 1, 0, 4, 0x0010}, {P_ACXXH, 1, 0, 4, 0x0010}, {P_PRG, 1, 0, 5, 0x0020}}, },
	{ "LDXM",	0x00c8, 0x00cf, 1, 3, {{P_ACXXL, 1, 0, 4, 0x0010}, {P_ACXXH, 1, 0, 4, 0x0010}, {P_PRG, 1, 0, 5, 0x0020}}, },
	{ "LDXNM",	0x00cc, 0x00cf, 1, 3, {{P_ACXXL, 1, 0, 4, 0x0010}, {P_ACXXH, 1, 0, 4, 0x0010}, {P_PRG, 1, 0, 5, 0x0020}}, },
	{ "LD",		0x00c0, 0x00cc, 1, 3, {{P_ACXHL0, 1, 0, 5, 0x0020}, {P_ACXHL1, 1, 0, 3, 0x0010}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "LDN",	0x00c4, 0x00cc, 1, 3, {{P_ACXHL0, 1, 0, 5, 0x0020}, {P_ACXHL1, 1, 0, 3, 0x0010}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "LDM",	0x00c8, 0x00cc, 1, 3, {{P_ACXHL0, 1, 0, 5, 0x0020}, {P_ACXHL1, 1, 0, 3, 0x0010}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "LDNM",	0x00cc, 0x00cc, 1, 3, {{P_ACXHL0, 1, 0, 5, 0x0020}, {P_ACXHL1, 1, 0, 3, 0x0010}, {P_PRG, 1, 0, 0, 0x0003}}, },
	{ "MV",		0x0010, 0x00f0, 1, 2, {{P_REG18, 1, 0, 2, 0x000c}, {P_ACCLM, 1, 0, 0, 0x0003}}, },
	{ "DR",		0x0004, 0x00fc, 1, 1, {{P_REG, 1, 0, 0, 0x0003}}, },
	{ "IR",		0x0008, 0x00fc, 1, 1, {{P_REG, 1, 0, 0, 0x0003}}, },
	{ "NR",		0x000c, 0x00fc, 1, 1, {{P_REG, 1, 0, 0, 0x0003}}, },
	{ "XXX",	0x0000, 0x0000, 1, 1, {{P_VAL, 1, 0, 0, 0x00ff}}, },
};

const uint32 registers_size = sizeof(registers)/sizeof(reg_t);
const uint32 opcodes_size = sizeof(opcodes)/sizeof(opc_t);
const uint32 opcodes_ext_size = sizeof(opcodes_ext)/sizeof(opc_t);

