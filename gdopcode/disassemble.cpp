/*====================================================================

$Id: disassemble.cpp,v 1.3 2008-11-11 01:04:26 wntrmute Exp $

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
Revision 1.4  2008/10/04 10:30:00  Hermes
add register names, export to a textfile
Revision 1.2  2008/06/02 02:42:57  wntrmute
fix stuff on OSX

Revision 1.1  2005/08/24 22:13:34  wntrmute
Initial import


====================================================================*/

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "dtypes.h"
#include "opcodes.h"
#include "gdsp_tool.h"

#ifdef _MSVC_
#pragma warning(disable:4996)
#endif

uint32 unk_opcodes[0x10000];

uint16 swap16(uint16 x);

// predefined labels
typedef struct pdlabel_t
{
	uint16		addr;
	const char	*name;
	const char	*description;
} pdlabels_t;


pdlabel_t pdlabels[] =
{
	{ 0xffc9, "DSCR", "DSP DMA Control Reg", },
	{ 0xffcb, "DSBL", "DSP DMA Block Length", },
	{ 0xffcd, "DSPA", "DSP DMA DMEM Address", },
	{ 0xffce, "DSMAH", "DSP DMA Mem Address H", },
	{ 0xffcf, "DSMAL", "DSP DMA Mem Address L", },
	{ 0xffd1, "ACFMT", "", },
	{ 0xffd4, "ACSAH", "", },
	{ 0xffd5, "ACSAL", "", },
	{ 0xffd6, "ACEAH", "", },
	{ 0xffd7, "ACEAL", "", },
	{ 0xffd8, "ACCAH", "", },
	{ 0xffd9, "ACCAL", "", },
	{ 0xffda, "ACPDS", "", },
	{ 0xffdb, "ACYN1", "", },
	{ 0xffdc, "ACYN2", "", },
	{ 0xffdd, "ACDAT", "", },
	{ 0xffde, "ACGAN", "", },
	{ 0xffef, "AMDM", "ARAM DMA Request Mask", },
	{ 0xfffb, "DIRQ", "DSP Irq Request", },
	{ 0xfffc, "DMBH", "DSP Mailbox H", },
	{ 0xfffd, "DMBL", "DSP Mailbox L", },
	{ 0xfffe, "CMBH", "CPU Mailbox H", },
	{ 0xffff, "CMBL", "CPU Mailbox L", },
	{0,"","LABEL"}
};

pdlabel_t regnames[] =
{
	{ 0x00, "AR0", "Register 00", },
	{ 0x01, "AR1", "Register 00", },
	{ 0x02, "AR2", "Register 00", },
	{ 0x03, "AR3", "Register 00", },
	{ 0x04, "IX0", "Register 00", },
	{ 0x05, "IX1", "Register 00", },
	{ 0x06, "IX2", "Register 00", },
	{ 0x07, "IX3", "Register 00", },
	{ 0x08, "WR0", "Register 00", },
	{ 0x09, "WR1", "Register 00", },
	{ 0x0a, "WR2", "Register 00", },
	{ 0x0b, "WR3", "Register 00", },
	{ 0x0c, "ST0", "Register 00", },
	{ 0x0d, "ST1", "Register 00", },
	{ 0x0e, "ST2", "Register 00", },
	{ 0x0f, "ST3", "Register 00", },
	{ 0x10, "ACC0.H", "Accumulator High 0", },
	{ 0x11, "ACC1.H", "Accumulator High 1", },
	{ 0x12, "CONFIG", "Register 00", },
	{ 0x13, "STATUS", "Register 00", },
	{ 0x14, "PROD.L", "Register 00", },
	{ 0x15, "PROD.M1", "Register 00", },
	{ 0x16, "PROD.H", "Register 00", },
	{ 0x17, "PROD.M2", "Register 00", },
	{ 0x18, "ACX0.L", "Register 00", },
	{ 0x19, "ACX1.L", "Register 00", },
	{ 0x1a, "ACX0.H", "Register 00", },
	{ 0x1b, "ACX1.H", "Register 00", },
	{ 0x1c, "ACC0.L", "Register 00", },
	{ 0x1d, "ACC1.L", "Register 00", },
	{ 0x1e, "ACC0.M", "Register 00", },
	{ 0x1f, "ACC1.M", "Register 00", },

	{ 0x1e, "ACC0", "Register 00", },
	{ 0x1f, "ACC1", "Register 00", },

	{ 0x18, "ACX0", "Register 00", },
	{ 0x19, "ACX1", "Register 00", },
};

char set_addr[65536];

char tmpstr[12];
const char * pdname(uint16 val)
{
static char my_label[32];

	uint32 i;
	for(i = 0 ; i < sizeof(pdlabels)/sizeof(pdlabel_t) ; i++)
	{

		if (pdlabels[i].name[0]==0)
			{
			sprintf(my_label,"LABEL_0x%4.4x", val);
			set_addr[val]|=1;
			return my_label;
			}
		
		if (pdlabels[i].addr == val)
			return pdlabels[i].name;

	}
	sprintf(tmpstr, "0x%04x", val);
	return tmpstr;
}

char *gd_dis_params(gd_globals_t *gdg, opc_t *opc, uint16 op1, uint16 op2, char *strbuf)
{
	char *buf = strbuf;
	uint32 val;
	int j;

	for (j = 0 ; j < opc->param_count ; j++)
	{
		if (j > 0)
		{
			sprintf(buf, ", ");
			buf += strlen(buf);
		}
		if (opc->params[j].loc >= 1)
			val = op2;
		else
			val = op1;

		val &= opc->params[j].mask;

		if (opc->params[j].lshift < 0)
			val = val << (-opc->params[j].lshift);
		else
			val = val >> opc->params[j].lshift;

		uint32 type;
		type = opc->params[j].type;
		if((type & 0xff)==0x10 || (type & 0xff)==0x20 || (type & 0xff)==0x30) type &=0xff00;
		if (type & P_REG)
		{

			if (type == P_ACC_D ||type==P_ACCM_D)
				val = (~val & 0x1) | ((type & P_REGS_MASK) >> 8);
			else
				val |= (type & P_REGS_MASK) >> 8;
			type &= ~P_REGS_MASK;
		}

		switch(type)
		{
		case P_REG:
			if (gdg->decode_registers) sprintf(buf, "$%s", regnames[val].name);
			else sprintf(buf, "$%d", val);
			break;
		case P_PRG:
			if (gdg->decode_registers) sprintf(buf, "@$%s", regnames[val].name);
			else sprintf(buf, "@$%d", val);
			break;
		case P_VAL:
		case P_ADDR_I:
		case P_ADDR_D:	
			if (gdg->decode_names)
				sprintf(buf, "%s", pdname(val));
			else
				sprintf(buf, "0x%04x", val);
			break;
		case P_IMM:
			if (opc->params[j].size != 2)
				{
				if(opc->opcode==P_OPC_LSR || opc->opcode==P_OPC_ASR)
					{
						sprintf(buf,"#%d", (0x40 - val));
					}
				else
					sprintf(buf, "#0x%02x", val);
				}
			else
				sprintf(buf, "#0x%04x", val);
			break;
		case P_MEM:
			if (opc->params[j].size != 2)
				val = (uint16)(sint8)val;
			if (gdg->decode_names)
				sprintf(buf, "@%s", pdname(val));
			else
				sprintf(buf, "@0x%04x", val);
			break;
		default:
			fprintf(stderr, "Unknown parameter type: %x\n", opc->params[j].type);
			exit(-1);
			break;
		}
		buf += strlen(buf);
	}
	return strbuf;
}


gd_globals_t *gd_init(void)
{
	gd_globals_t *gdg;

	gdg = (gd_globals_t *)malloc(sizeof(gd_globals_t));
	memset(gdg, 0, sizeof(gd_globals_t));
	return gdg;
}

uint16 gd_dis_get_opcode_size(gd_globals_t *gdg)
{
	uint32 j;
	opc_t *opc=NULL;
	opc_t	*opc_ext=NULL;
	uint32	op1;
	bool	extended;

	if ((gdg->pc & 0x7fff) >= 0x1000)
	{
		return 1;
	}
	op1 = swap16(gdg->binbuf[gdg->pc & 0x0fff]);

	for(j = 0 ; j < opcodes_size ; j++)
	{
		uint16 mask;
		if (opcodes[j].size & P_EXT)
			mask = opcodes[j].opcode_mask & 0xff00;
		else
			mask = opcodes[j].opcode_mask;
		if ((op1 & mask) == opcodes[j].opcode)
		{
			opc = &opcodes[j];
			break;
		}
	}
	if (opc->size & P_EXT && op1 & 0x00ff)
		extended = true;
	else
		extended = false;

	if (extended)
	{
		// opcode has an extension
		// find opcode
		for(j = 0 ; j < opcodes_ext_size ; j++)
		{
			if ((op1 & opcodes_ext[j].opcode_mask) == opcodes_ext[j].opcode)
			{
				opc_ext = &opcodes_ext[j];
				break;
			}
		}
		return opc_ext->size;
	}

	return opc->size & ~P_EXT;
}
char *gd_dis_opcode(gd_globals_t *gdg)
{
	uint32	j;
	uint32	op1, op2;
	opc_t	*opc=NULL;
	opc_t	*opc_ext=NULL;
	uint16	pc;
	char	*buf = gdg->buffer;
	bool	extended = false;
	bool	only7bits = false;

	pc = gdg->pc;
	*buf = '\0';
	if ((pc & 0x7fff) >= 0x1000)
	{
		gdg->pc++;
		return gdg->buffer;
	}
	pc &= 0x0fff;
	op1 = swap16(gdg->binbuf[pc]);

	// find opcode
	for(j = 0 ; j < opcodes_size ; j++)
	{
		uint16 mask = opcodes[j].opcode_mask;
		if ((op1 & mask) == opcodes[j].opcode)
		{
			opc = &opcodes[j];
			break;
		}
	}

	if((opc->opcode>>12)==0x03 && op1&0x7f) {
		extended = true;
		only7bits = true;
	} else if((opc->opcode>>12)>0x03 && op1&0xff) 
		extended = true;
	else
		extended = false;

	if (extended)
	{
		// opcode has an extension
		// find opcode
		for(j = 0 ; j < opcodes_ext_size ; j++)
		{
			if(only7bits==true) {
				if(((op1&0x7f)&opcodes_ext[j].opcode_mask)==opcodes_ext[j].opcode) {
					opc_ext = &opcodes_ext[j];
					break;
				}
			} else {
				if ((op1 & opcodes_ext[j].opcode_mask) == opcodes_ext[j].opcode)
				{
					opc_ext = &opcodes_ext[j];
					break;
				}
			}
		}
	}

	// printing
	
	if(gdg->decode_names)
		{if(set_addr[gdg->pc] & 1) sprintf(buf,"LABEL_0x%4.4x:\n", gdg->pc);buf += strlen(buf);}
	
	if (gdg->show_pc) sprintf(buf, "%04x ", gdg->pc);
	else sprintf(buf, "	");

	buf += strlen(buf);

	if ((opc->size & ~P_EXT) == 2)
	{
		op2 = swap16(gdg->binbuf[pc + 1]);
		if (gdg->show_hex) sprintf(buf, "%04x %04x ", op1, op2);
	}
	else
	{
		op2 = 0;
		if (gdg->show_hex) sprintf(buf, "%04x      ", op1);
	}
	buf += strlen(buf);

	char tmpbuf[20];

	if (extended)
		sprintf(tmpbuf, "%s%c%s", opc->name, gdg->ext_separator, opc_ext->name);
	else
		sprintf(tmpbuf, "%s", opc->name);

	if (gdg->print_tabs)
		sprintf(buf, "%s\t", tmpbuf);
	else
		sprintf(buf, "%-12s", tmpbuf);
	buf += strlen(buf);
	if (opc->param_count > 0)
	{
		gd_dis_params(gdg, opc, op1, op2, buf);
	}
	buf += strlen(buf);

	if(extended)
	{
		if (opc->param_count > 0)
			sprintf(buf, " ");
		buf += strlen(buf);

		sprintf(buf, ": ");
		buf += strlen(buf);

		if (opc_ext->param_count > 0)
		{
			gd_dis_params(gdg, opc_ext, op1, op2, buf);
		}
		buf += strlen(buf);
	}

	if (opc->opcode_mask == 0)
	{
		// unknown opcode
		unk_opcodes[op1]++;
		sprintf(buf, "\t\t; *** UNKNOWN OPCODE ***");
	}
	if (extended)
		gdg->pc += opc_ext->size;
	else
		gdg->pc += opc->size & ~P_EXT;

	return gdg->buffer;
}

bool gd_dis_file(gd_globals_t *gdg, char *name, FILE *output)
{
	FILE *in;
	uint32 size;
	int pass,n;
	uint16 start_pc= gdg->pc;

	in = fopen(name, "rb");
	if (in == NULL)
		return false;
	fseek(in, 0, SEEK_END);
	size = ftell(in);
	fseek(in, 0, SEEK_SET);
	gdg->binbuf = (uint16 *)malloc(size);
	fread(gdg->binbuf, 1, size, in);

	gdg->buffer = (char *)malloc(256);
	gdg->buffer_size = 256;

	memset(set_addr, 0, 65536);
	
	for(pass=!gdg->decode_names; pass<2; pass++) // two pass to add labels
		{
		gdg->pc=start_pc;
		if(pass==1 && gdg->decode_names)
			{
			int first=1;

			fprintf(output, "\n/********************************/\n");
			fprintf(output, "/**      REGISTER NAMES        **/\n");
			fprintf(output, "/********************************/\n\n");

			for(n=0;n<32;n++)
				fprintf(output, "%s:	equ	0x%4.4x\n", regnames[n].name, regnames[n].addr);

			fprintf(output, "\n/********************************/\n");
			fprintf(output, "/**      REGISTER ADDRESS      **/\n");
			fprintf(output, "/********************************/\n\n");

			n=0;
			while(pdlabels[n].name[0]!=0)
				{
				fprintf(output, "%s:	equ	0x%4.4x\n", pdlabels[n].name, pdlabels[n].addr);
				n++;
				}
			for(n=0;n<65536;n++) // addresses out of the code
				{
				if(set_addr[n]==1)
					{
					if(first)
						{
						first=0;
						fprintf(output, "\n/********************************/\n");
						fprintf(output, "/**       OTHER ADDRESSES      **/\n");
						fprintf(output, "/********************************/\n\n");
						}
					fprintf(output, "LABEL_0x%4.4x:	equ	0x%4.4x\n", n, n);
					}
				}
			fprintf(output, "\n/********************************/\n");
			fprintf(output, "/**         START CODE         **/\n");
			fprintf(output, "/********************************/\n\n");
			fprintf(output, "	org	0x%4.4x\n\n", gdg->pc);

			fprintf(output,"START:\n");
			}
		// gdg->pc PC is now provided by one param
		for (gdg->pc =start_pc ; gdg->pc < (start_pc+(size/2)) ;)
			{
			set_addr[gdg->pc]|=2;
			if(pass==0) gd_dis_opcode(gdg);
				else fprintf(output, "%s\n", gd_dis_opcode(gdg));
			}
		}

	fclose(in);

	free(gdg->binbuf);
	gdg->binbuf = NULL;

	free(gdg->buffer);
	gdg->buffer = NULL;
	gdg->buffer_size = 0;

	return true;
}

void gd_dis_close_unkop(void)
{
	FILE *uo;
	int i, j;
	uint32 count = 0;

	uo = fopen("uo.bin", "wb");
	if (uo)
	{
		fwrite(unk_opcodes, 1, sizeof(unk_opcodes), uo);
		fclose(uo);
	}
	uo = fopen("unkopc.txt", "w");

	if (uo)
	{
		for(i = 0 ; i < 0x10000 ; i++)
		{
			if (unk_opcodes[i])
			{
				count++;
				fprintf(uo, "OP%04x\t%d", i, unk_opcodes[i]);
				for(j = 15 ; j >= 0 ; j--)
				{
					if ((j & 0x3) == 3)
						fprintf(uo, "\tb");
					fprintf(uo, "%d", (i >> j) & 0x1);
				}
				fprintf(uo, "\n");
			}
		}
		fprintf(uo, "Unknown opcodes count: %d\n", count);
		fclose(uo);
	}
}
void gd_dis_open_unkop(void)
{
	FILE *uo;

	uo = fopen("uo.bin", "rb");
	if (uo)
	{
		fread(unk_opcodes, 1, sizeof(unk_opcodes), uo);
		fclose(uo);
	}
	else
	{
		int i;
		for(i = 0 ; i < 0x10000 ; i++)
			unk_opcodes[i] = 0;
	}
}

