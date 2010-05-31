/*====================================================================

$Id: assemble.cpp,v 1.3 2008-11-11 01:04:26 wntrmute Exp $

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

Revision 1.4  2008/10/04 10:30:00  Hermes
added function to export the code to .h file
added support for '/ *' '* /' and '//' for comentaries
added some sintax detection when use registers

$Log: not supported by cvs2svn $
Revision 1.2  2005/09/14 02:19:29  wntrmute
added header guards
use standard main function

Revision 1.1  2005/08/24 22:13:34  wntrmute
Initial import


====================================================================*/

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "dtypes.h"
#include "opcodes.h"
#include "disassemble.h"
#include "gdsp_tool.h"

#ifdef _MSVC_
// bha... Visual Studio 2005 deprecates fopen :)
#pragma warning(disable:4996)
#endif

char *include_dir = NULL;

typedef struct label_t
{
	char	*label;
	sint32	addr;
} label_t;

typedef struct param_t
{
	uint32		val;
	partype_t	type;
	char		*str;
} param_t;

label_t labels[10000];
uint32	labels_count = 0;


typedef struct fass_t
{
	FILE	*fsrc;
	uint32	code_line;
} fass_t;


typedef enum
{
	ERR_OK = 0,
	ERR_UNKNOWN,
	ERR_UNKNOWN_OPCODE,
	ERR_NOT_ENOUGH_PARAMETERS,
	ERR_TOO_MANY_PARAMETERS,
	ERR_WRONG_PARAMETER,
	ERR_EXPECTED_PARAM_STR,
	ERR_EXPECTED_PARAM_VAL,
	ERR_EXPECTED_PARAM_REG,
	ERR_EXPECTED_PARAM_MEM,
	ERR_EXPECTED_PARAM_IMM,
	ERR_INCORRECT_BIN,
	ERR_INCORRECT_HEX,
	ERR_INCORRECT_DEC,
	ERR_LABEL_EXISTS,
	ERR_UNKNOWN_LABEL,
	ERR_NO_MATCHING_BRACKETS,
	ERR_EXT_CANT_EXTEND_OPCODE,
	ERR_EXT_PAR_NOT_EXT,
	ERR_WRONG_PARAMETER_ACX,
	ERR_WRONG_PARAMETER_HIGH_ACX,
	ERR_WRONG_PARAMETER_LOW_ACX,
	ERR_WRONG_PARAMETER_ACX0_LOWHI,
	ERR_WRONG_PARAMETER_ACX1_LOWHI,
	ERR_WRONG_PARAMETER_ACC,
	ERR_WRONG_PARAMETER_MID_ACC,
	ERR_WRONG_PARAMETER_LOW_ACC,
	ERR_INVALID_REGISTER,
	ERR_OUT_RANGE_NUMBER
} err_t;

typedef enum
{
	SEGMENT_CODE = 0,
	SEGMENT_DATA,
	SEGMENT_OVERLAY,
	SEGMENT_MAX
} segment_t;


const char *err_string[] =
{
	"",
	"Unknown Error",
	"Unknown opcode",
	"Not enough parameters",
	"Too many parameters",
	"Wrong parameter",
	"Expected parameter of type 'string'",
	"Expected parameter of type 'value'",
	"Expected parameter of type 'register'",
	"Expected parameter of type 'memory pointer'",
	"Expected parameter of type 'immediate'",
	"Incorrect binary value",
	"Incorrect hexadecimal value",
	"Incorrect decimal value",
	"Label already exists",
	"Label not defined",
	"No matching brackets",
	"This opcode cannot be extended",
	"Given extending params for non extensible opcode",
	"Wrong parameter: must be secondary accumulator register",
	"Wrong parameter: must be high part of secondary accumulator register",
	"Wrong parameter: must be low part of secondary accumulator register",
	"Wrong parameter: must be low or high part of secondary accumulator register 0",
	"Wrong parameter: must be low or high part of secondary accumulator register 1",
	"Wrong parameter: must be accumulator register",
	"Wrong parameter: must be mid part of accumulator register",
	"Wrong parameter: must be low part of accumulator register",
	"Invalid register",
	"Number out of range"
};





segment_t cur_segment;
uint32	cur_addr;
uint8	cur_pass;
fass_t	*cur_fa;

uint32 segment_addr[SEGMENT_MAX];

int current_param=0;

void parse_error(err_t err_code, fass_t *fa)
{

	if (fa->fsrc)
		fclose(fa->fsrc);
	else
		{
		fprintf(stderr, "ERROR: %s\n", err_string[err_code]);
		exit(-1);
		}

	// modified by Hermes

	if(current_param==0)
		fprintf(stderr, "ERROR: %s Line: %d\n", err_string[err_code], fa->code_line);
	else 
		fprintf(stderr, "ERROR: %s Line: %d Param: %d\n", err_string[err_code], fa->code_line, current_param);

	exit(-1);
}

char *skip_spaces(char *ptr)
{
	while(*ptr == ' ')
	{
		ptr++;
	};
	return ptr;
}

sint32 strtoval(char *str)
{
	uint32  i;

	bool	negative = false;
	sint32	val = 0;
	char	*ptr = str;

	if (ptr[0]=='#')
	{
		ptr++; negative = true;
	}
	if(ptr[0]=='0')
	{
		if(ptr[1] >= '0' && ptr[1] <= '9')
		{
			for(i = 0 ; ptr[i]!=0 ; i++)
			{
				val *= 10;
				if(ptr[i] >= '0' && ptr[i] <= '9')
					val += ptr[i] - '0';
				else
					parse_error(ERR_INCORRECT_DEC, cur_fa);
			}
		}
		else
		{
			switch(ptr[1])
			{
			case 'X': // hex
				for(i = 2 ; ptr[i] !=0 ; i++)
				{
					val *=16;
					if(ptr[i] >= 'a' && ptr[i] <= 'f')
						val+= (ptr[i]-'a'+10);
					else if(ptr[i] >= 'A' && ptr[i] <= 'F')
						val += (ptr[i]-'A'+10);
					else if(ptr[i] >= '0' && ptr[i] <= '9')
						val += (ptr[i]-'0');
					else
						parse_error(ERR_INCORRECT_HEX, cur_fa);
				}
				break;
			case '\'': // binary
				for(i = 2; ptr[i]!=0; i++)
				{
					val *=2;
					if(ptr[i] >= '0' && ptr[i] <= '1')
						val += ptr[i] - '0';
					else
						parse_error(ERR_INCORRECT_BIN, cur_fa);
				}
				break;
			default:
				// value is 0 or error
				val = 0;
				break;
			}
		}
	}
	else
	{
		if(ptr[0]>='0' && ptr[0]<='9')
		{
			for(i = 0; ptr[i]!=0; i++)
			{
				val *=10;
				if(ptr[i]>='0' && ptr[i]<='9') val+= ptr[i]-'0';
				else parse_error(ERR_INCORRECT_DEC, cur_fa);
			}
		}
		else
		{
			// label
			for(i = 0 ; i < (uint32)labels_count ; i++)
			{
				if (strcmp(labels[i].label, ptr) == 0)
					return labels[i].addr;
			}
			if (cur_pass == 2)
				parse_error(ERR_UNKNOWN_LABEL, cur_fa);
		}
	}
	if (negative)
		return -val;
	return val;
}
char *find_brackets(char *src, char *dst)
{
	sint32 len = (sint32) strlen(src);
	sint32 first = -1;
	sint32 count = 0;
	sint32 i, j;
	j = 0;
	for (i = 0 ; i < len ; i++)
	{
		if (src[i] == '(')
		{
			if (first < 0)
			{
				count = 1;
				src[i] = 0x0;
				first = i;
			}
			else
			{
				count++;
				dst[j++] = src[i];
			}
		}
		else if (src[i] == ')')
		{
			if (--count == 0)
			{
				dst[j] = 0;
				return &src[i+1];
			}
			else
			{
				dst[j++] = src[i];
			}
		}
		else
		{
			if(first >= 0)
				dst[j++] = src[i];
		}
	}
	if (count)
		parse_error(ERR_NO_MATCHING_BRACKETS, cur_fa);
	return NULL;
}

uint32 parse_exp(char *ptr)
{
	char *d_buffer;
	char *s_buffer;
	char *pbuf;
	int i,j;
	sint32 val = 0;

	d_buffer = (char *)malloc(1024);
	s_buffer = (char *)malloc(1024);
	strcpy(s_buffer, ptr);
	while ((pbuf = find_brackets(s_buffer, d_buffer)) != NULL)
	{
		val = parse_exp(d_buffer);
		sprintf(d_buffer, "%s%d%s", s_buffer, val, pbuf);
		strcpy(s_buffer, d_buffer);
	}
	j = 0;
	for(i = 0 ; i < ((sint32)strlen(s_buffer) + 1) ; i++)
	{
		char c;
		c = s_buffer[i];
		if (c != ' ')
			d_buffer[j++] = c;
	}

	for(i = 0 ; i < ((sint32)strlen(d_buffer) + 1) ; i++)
	{
		char c;
		char cx;
		c = d_buffer[i];
		if (c == '-')
		{
			if (i == 0)
				c = '#';
			else
			{
				cx = d_buffer[i-1];
				switch(cx)
				{
				case '/':
				case '%':
				case '*':
					c = '#';
				}
			}
		}
		d_buffer[i] = c;
	}
	while((pbuf = strstr(d_buffer, "+")) != NULL)
	{
		*pbuf = 0x0;
		val = parse_exp(d_buffer) + parse_exp(pbuf+1);
		sprintf(d_buffer, "%d", val);
	}

	while((pbuf = strstr(d_buffer, "-")) != NULL)
	{
		*pbuf = 0x0;
		val = parse_exp(d_buffer) - parse_exp(pbuf+1);
		if(val<0) 
			{val=0x10000+(val & 0xffff); // ATTENTION: avoid a terrible bug!!! number cannot write with '-' in sprintf
			if(cur_fa) fprintf(stderr, "WARNING: Number Underflow at Line: %d \n", cur_fa->code_line);
			}
		
		sprintf(d_buffer, "%d", val);
		
	}


	while((pbuf = strstr(d_buffer, "*")) != NULL)
	{
		*pbuf = 0x0;
		val = parse_exp(d_buffer) * parse_exp(pbuf+1);
		sprintf(d_buffer, "%d", val);
	}

	while((pbuf = strstr(d_buffer, "/")) != NULL)
	{
		*pbuf = 0x0;
		val = parse_exp(d_buffer) / parse_exp(pbuf+1);
		sprintf(d_buffer, "%d", val);
	}

	while((pbuf = strstr(d_buffer, "|")) != NULL)
	{
		*pbuf = 0x0;
		val = parse_exp(d_buffer) | parse_exp(pbuf+1);
		sprintf(d_buffer, "%d", val);
	}

	while((pbuf = strstr(d_buffer, "&")) != NULL)
	{
		*pbuf = 0x0;
		val = parse_exp(d_buffer) & parse_exp(pbuf+1);
		sprintf(d_buffer, "%d", val);
	}

	val = strtoval(d_buffer);
	free(d_buffer);
	free(s_buffer);
	return val;
}


uint32 parse_exp_f(char *ptr, fass_t *fa)
{
	cur_fa = fa;
	return parse_exp(ptr);
}

uint32 parse_reg_f(char *name, fass_t *fa)
{
	uint32 i;
	reg_t *reg;

	for(i=0;i<registers_size;i++) {
		reg = &registers[i];
		if(strncmp(reg->name,name,strlen(reg->name))==0)
			return reg->regv;
	}
	return parse_exp_f(name,fa);
}

param_t *params;
param_t *params_ext;
uint32 params_count;
uint32 params_count_ext;

uint32 get_params(char *parstr, param_t *par, fass_t *fa)
{
	char *tmpstr;
	uint32 count;
	int i;

	count = 0;
	tmpstr = skip_spaces(parstr);
	tmpstr = strtok(tmpstr, ",\x00");

	for(i = 0 ; i < 10 ; i++)
	{
		if (tmpstr == NULL)
			break;
		tmpstr = skip_spaces(tmpstr);
		if (strlen(tmpstr) == 0)
			break;
		if(tmpstr) count++;
		else break;

		par[i].type = P_NONE;
		switch (tmpstr[0])
		{
		case '"':
			par[i].str = strtok(tmpstr, "\"");
			par[i].type = P_STR;
			break;
		case '#':
			par[i].val = parse_exp_f(tmpstr + 1, fa);
			par[i].type = P_IMM;
			break;
		case '@':
			if (tmpstr[1] == '$')
			{
				par[i].val = parse_reg_f(tmpstr + 2, fa);
				par[i].type = P_PRG;
			}
			else
			{
				par[i].val = parse_exp_f(tmpstr + 1, fa);
				par[i].type = P_MEM;
			}
			break;
		case '$':
			par[i].val = parse_reg_f(tmpstr + 1, fa);
			par[i].type = P_REG;
			break;
		default:
			par[i].val = parse_exp_f(tmpstr, fa);
			par[i].type = P_VAL;
			break;
		}
		tmpstr = strtok(NULL, ",\x00");
	}
	return count;
}

opc_t *find_opcode(char *opcode, uint32 par_count, opc_t *opcod, uint32 opcod_size, fass_t *fa)
{
	uint32	i;
	opc_t	*opc;

	for(i = 0 ; i < opcod_size ; i++)
	{
		opc = &opcod[i];
		if(strcmp(opc->name, opcode) == 0)
		{
			if (par_count < opc->param_count)
			{
				parse_error(ERR_NOT_ENOUGH_PARAMETERS, fa);
			}
			if (par_count > opc->param_count)
			{
				parse_error(ERR_TOO_MANY_PARAMETERS, fa);
			}
			return opc;
		}
	}
	parse_error(ERR_UNKNOWN_OPCODE, fa);
	return NULL;
}

uint16 get_mask(uint16 mask)
{
	while(!(mask & 1)) mask>>=1;
	return mask;
}

bool verify_params(opc_t *opc, param_t *par, uint32 count, fass_t *fa)
{
	uint32 i;
	int value,regval;
	unsigned int valueu;


	for(i = 0 ; i < count ; i++)
	{
		current_param=i+1;
		if (opc->params[i].type != par[i].type || (par[i].type & P_REG))
		{
			
			if(par[i].type==P_VAL && (opc->params[i].type==P_ADDR_I || opc->params[i].type==P_ADDR_D)) continue;

			if ((opc->params[i].type & P_REG) && (par[i].type & P_REG))
				{
				// modified by Hermes: test the register range

				switch((unsigned)opc->params[i].type)
					{
					case P_REG18:
					case P_ACCLM:
						regval = par[i].val&0xff;
						value=(opc->params[i].type>>8) & 31;
						if(regval<value || regval>(value + get_mask(opc->params[i].mask)))
						{
							parse_error(ERR_INVALID_REGISTER, fa);
						}
					break;
					case P_PRG:
						if((int)par[i].val<0 || (int)par[i].val>0x3)
						{
							parse_error(ERR_INVALID_REGISTER, fa);
						}
					break;

					case P_ACC:
						if((int)par[i].val<0x201c || (int)par[i].val>0x201d)
						{
							if(par[i].val>=0x1e && par[i].val<=0x1f)
								fprintf(stderr, "WARNING: $ACC%d.M register used instead $ACC%d register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else if(par[i].val>=0x1c && par[i].val<=0x1d)
								fprintf(stderr, "WARNING: $ACC%d.L register used instead $ACC%d register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else
								parse_error(ERR_WRONG_PARAMETER_ACC, fa);
						} 					break;
					case P_ACCM:
						if((int)par[i].val<0x1e || (int)par[i].val>0x1f)
						{
							if(par[i].val>=0x1c && par[i].val<=0x1d)
								fprintf(stderr, "WARNING: $ACC%d.L register used instead $ACC%d.M register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else if(par[i].val>=0x201c && par[i].val<=0x201d)
								fprintf(stderr, "WARNING: $ACC%d register used instead $ACC%d.M register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else
								parse_error(ERR_WRONG_PARAMETER_MID_ACC, fa);
						}
					break;
					case P_ACCL:
						if((int)par[i].val<0x1c || (int)par[i].val>0x1d)
						{
							if(par[i].val>=0x1e && par[i].val<=0x1f)
								fprintf(stderr, "WARNING: $ACC%d.M register used instead $ACC%d.L register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else if(par[i].val>=0x201c && par[i].val<=0x201d)
								fprintf(stderr, "WARNING: $ACC%d register used instead $ACC%d.L register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else
								parse_error(ERR_WRONG_PARAMETER_LOW_ACC, fa);
						}
					break;
					case P_ACX:
						if((int)par[i].val<0x2018 || (int)par[i].val>0x2019)
						{
							if(par[i].val>=0x18 && par[i].val<=0x19)
								fprintf(stderr, "WARNING: $ACX%d.L register used instead $ACX%d register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else if(par[i].val>=0x1a && par[i].val<=0x1b)
								fprintf(stderr, "WARNING: $ACX%d.H register used instead $ACX%d register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else
								parse_error(ERR_WRONG_PARAMETER_ACX, fa);
						}
					break;
					case P_ACXXL:
						if((int)par[i].val<0x18 || (int)par[i].val>0x19) {
							if(par[i].val>=0x1a && par[i].val<=0x1b)
								fprintf(stderr, "WARNING: $ACX%d.H register used instead $ACX%d.L register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else if(par[i].val>=0x2018 && par[i].val<=0x2019)
								fprintf(stderr, "WARNING: $ACX%d register used instead $ACX%d.L register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else
								parse_error(ERR_WRONG_PARAMETER_LOW_ACX,fa);
						}
					break;
					case P_ACXXH:
						if((int)par[i].val<0x1a || (int)par[i].val>0x1b) {
							if(par[i].val>=0x18 && par[i].val<=0x19)
								fprintf(stderr, "WARNING: $ACX%d.L register used instead $ACX%d.H register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else if(par[i].val>=0x2018 && par[i].val<=0x2019)
								fprintf(stderr, "WARNING: $ACX%d register used instead $ACX%d.H register Line: %d Param: %d\n",
										(par[i].val & 1), (par[i].val & 1), fa->code_line, current_param);
							else
								parse_error(ERR_WRONG_PARAMETER_HIGH_ACX,fa);
						}
					break;
					case P_ACXHL0:
						if((int)par[i].val!=0x18 && (int)par[i].val!=0x1a){
							parse_error(ERR_WRONG_PARAMETER_ACX0_LOWHI,fa);
						}
					break;
					case P_ACXHL1:
						if((int)par[i].val!=0x19 && (int)par[i].val!=0x1b){
							parse_error(ERR_WRONG_PARAMETER_ACX1_LOWHI,fa);
						}
					break;
					}
				
				
				
				continue;
				}
			switch(par[i].type & (P_REG | P_VAL | P_MEM | P_IMM | P_STR | P_ADDR_I | P_ADDR_D))
			{
			case P_REG:
				parse_error(ERR_EXPECTED_PARAM_REG, fa);
				break;
			case P_MEM:
				parse_error(ERR_EXPECTED_PARAM_MEM, fa);
				break;
			case P_VAL:
				parse_error(ERR_EXPECTED_PARAM_VAL, fa);
				break;
			case P_IMM:
				parse_error(ERR_EXPECTED_PARAM_IMM, fa);
				break;
			}
			parse_error(ERR_WRONG_PARAMETER, fa);
			break;
		}
	else
	if ((opc->params[i].type & 7)!=0 && (par[i].type & 7)!=0)
		{// modified by Hermes: test NUMBER range
		value=get_mask(opc->params[i].mask);

		valueu=0xffff & ~(value>>1);
		if((int)par[i].val<0)
			{
			if(value==7) // value 7 por sbclr/sbset
				{
				fprintf(stderr,"Value must be from 0x0 to 0x%x\n", value);
				parse_error(ERR_OUT_RANGE_NUMBER, fa);
				}
			else
			if(opc->params[i].type==P_MEM)
					{
					if(value<256)
						fprintf(stderr,"Address value must be from 0x%x to 0x%x\n",valueu, (value>>1));
					else
						fprintf(stderr,"Address value must be from 0x0 to 0x%x\n", value);

					parse_error(ERR_OUT_RANGE_NUMBER, fa);
					}
			else
			if((int)par[i].val<-(value+1))
				{
				if(value<128)
					fprintf(stderr,"Value must be from -0x%x to 0x%x\n",(value+1), value);
				else
					fprintf(stderr,"Value must be from -0x%x to 0x%x or 0x0 to 0x%x\n",(value+1), value ,value);

				parse_error(ERR_OUT_RANGE_NUMBER, fa);
				}
			}
		else
			{
			if(value==7) // value 7 por sbclr/sbset
				{
				if(par[i].val>(unsigned)value)
					{
					fprintf(stderr,"Value must be from 0x%x to 0x%x\n",valueu, value);
					parse_error(ERR_OUT_RANGE_NUMBER, fa);
					}
				}
			else
			if(opc->params[i].type==P_MEM)
				{
				if(value<256) value>>=1; // addressing 8 bit with sign
				if(par[i].val>(unsigned)value && (par[i].val<valueu || par[i].val>(unsigned)0xffff))
					{
					if(value<256)
						fprintf(stderr,"Address value must be from 0x%x to 0x%x\n",valueu, value);
					else
						fprintf(stderr,"Address value must be minor of 0x%x\n", value+1);
					parse_error(ERR_OUT_RANGE_NUMBER, fa);
					}
				}
			else
				{
				if(value<128) value>>=1; // special case ASL/ASR/LSL/LSR

				if(par[i].val>(unsigned)value)
					{
					if(value<64)
						fprintf(stderr,"Value must be from -0x%x to 0x%x\n",(value+1), value);
					else
						fprintf(stderr,"Value must be minor of 0x%x\n", value+1);
					parse_error(ERR_OUT_RANGE_NUMBER, fa);
					}
				}
			}
		
		continue;
		}
			
	}

current_param=0;

	return true;
}

void build_code(opc_t *opc, param_t *par, uint32 par_count, uint16 *outbuf)
{
	uint32 i;
	outbuf[cur_addr] |= swap16(opc->opcode);
	for(i = 0 ; i < par_count ; i++)
	{
		uint16 v16;
		uint16 t16;
		if(opc->params[i].type != P_ACC_D && opc->params[i].type != P_ACCM_D)
		{
			t16 = swap16(outbuf[cur_addr + opc->params[i].loc]);
			v16 = par[i].val;
			if(opc->params[i].type==P_IMM && (opc->opcode==P_OPC_LSR || opc->opcode==P_OPC_ASR)) v16 = (uint16)(0 - v16);
			if (opc->params[i].lshift > 0)
				v16 <<= opc->params[i].lshift;
			else
				v16 >>= -opc->params[i].lshift;
			v16 &= opc->params[i].mask;
			outbuf[cur_addr + opc->params[i].loc] = swap16(t16 | v16);
		}
	}
}


void gd_ass_init(void)
{
	cur_addr = 0;
	cur_segment = SEGMENT_CODE;
	segment_addr[SEGMENT_CODE] = 0;
	segment_addr[SEGMENT_DATA] = 0;
	segment_addr[SEGMENT_OVERLAY] = 0;
}

void gd_ass_file(gd_globals_t *gdg, char *fname, uint32 pass)
{
	char linebuffer[1024];
	uint32 opcode_size;
	int i;
	fass_t fa;
    int disable_text=0; // modified by Hermes

	params = (param_t *)malloc(sizeof(param_t) * 10);
	params_ext = (param_t *)malloc(sizeof(param_t) * 10);

	fa.fsrc = fopen(fname, "r");

	if (fa.fsrc == NULL)
	{
		fprintf(stderr, "Cannot open %s file\n", fname);
		exit(-1);
	}

	fseek(fa.fsrc, 0, SEEK_SET);
	printf("Pass %d\n", pass);
	fa.code_line = 0;
	cur_pass = pass;
	while(!feof(fa.fsrc))
	{
		opcode_size = 0;
		memset(linebuffer, 0, 1024);
		fgets(linebuffer, 1024, fa.fsrc);
		fa.code_line++;

		for(i = 0 ; i < 1024 ; i++)
		{
			char c;
			c = linebuffer[i];

			// modified by Hermes : added // and /* */ for long comentaries 
			if(c=='/')
				{
				if(i<1023)
					{
					if(linebuffer[i+1]=='/') c=0x00;
						else
							if(linebuffer[i+1]=='*') 
								{
								if(disable_text) {disable_text=0;}
								else disable_text=1;
								}
					}
				}
			else
				if(c=='*')
				{
				if(i<1023)
					if(linebuffer[i+1]=='/') 
					   if(disable_text) {disable_text=0;c=32;linebuffer[i+1]=32;}
				}
			

			if(disable_text && ((unsigned char )c)>32) c=32;

			if (c == 0x0a || c == 0x0d || c == ';')
				c = 0x00;
			if (c == 0x09)				// tabs to spaces
				c = ' ';
			if (c >= 'a' && c <= 'z')	// convert to uppercase
				c = c - 'a' + 'A';
			linebuffer[i] = c;
			if(c==0) break; // modified by Hermes
		}
		char *ptr;
		ptr = linebuffer;

		char *opcode;
		char *label;
		char	*col_ptr;

		opcode = NULL;
		label = NULL;
		if ((col_ptr = strstr(ptr, ":")) != NULL)
		{
			int		j;
			bool	valid;
			j = 0;
			valid = true;
			while ((ptr+j) < col_ptr)
			{
				if (j == 0)
					if (!((ptr[j] >= 'A' && ptr[j] <= 'Z') || (ptr[j] == '_')))
						valid = false;
				if (!((ptr[j] >= '0' && ptr[j] <= '9') || (ptr[j] >= 'A' && ptr[j] <= 'Z') || (ptr[j] == '_')))
					valid = false;
				j++;
			}
			if (valid)
			{
				label = strtok(ptr, ":\x20");
				ptr	= col_ptr + 1;
			}
		}

		opcode = strtok(ptr, " ");
		char *opcode_ext=NULL;

		char *paramstr;
		char *paramstr_ext;

		if (opcode)
		{
			if ((opcode_ext = strstr(opcode, "'")) != NULL)
			{
				opcode_ext[0] = '\0';
				opcode_ext++;
				if (strlen(opcode_ext) == 0)
					opcode_ext = NULL;
			}
			// now we have opcode and label

			params_count = 0;
			params_count_ext = 0;

			paramstr = paramstr_ext = 0;
			// there is valid opcode so probably we have parameters

			paramstr = strtok(NULL, "\0");

			if (paramstr)
			{
				if ((paramstr_ext = strstr(paramstr, ":")) != NULL)
				{
					paramstr_ext[0] = '\0';
					paramstr_ext++;
				}
			}

			if (paramstr)
				params_count = get_params(paramstr, params, &fa);
			if (paramstr_ext)
				params_count_ext = get_params(paramstr_ext, params_ext, &fa);
		}

		if (label)
		{
			// there is a valid label so lets store it in labels table
			uint32 lval = cur_addr;
			if (opcode)
			{
				if (strcmp(opcode, "EQU") == 0)
				{
					lval = params[0].val;
					opcode = NULL;
				}
			
				
			}
			if (pass == 1)
			{
				labels[labels_count].label = (char *)malloc(strlen(label) + 1);
				strcpy(labels[labels_count].label, label);
				labels[labels_count].addr = lval;
				labels_count++;
			}
		
		
		}

		if (opcode == NULL)
			continue;
		

		// check if opcode is reserved compiler word
		if (strcmp("INCLUDE", opcode) == 0)
		{
			if (params[0].type == P_STR)
			{
				char *tmpstr;
				if (include_dir)
				{
					tmpstr = (char *)malloc(strlen(include_dir) + strlen(params[0].str) + 2);
					sprintf(tmpstr, "%s/%s", include_dir, params[0].str);
				}
				else
				{
					tmpstr = (char *)malloc(strlen(params[0].str) + 1);
					strcpy(tmpstr, params[0].str);
				}
				gd_ass_file(gdg, tmpstr, pass);
				free(tmpstr);
			}
			else
				parse_error(ERR_EXPECTED_PARAM_STR, &fa);
			continue;
		}

		if (strcmp("INCDIR", opcode) == 0)
		{
			if (params[0].type == P_STR)
			{
				if (include_dir) free(include_dir);
				include_dir = (char *)malloc(strlen(params[0].str) + 1);
				strcpy(include_dir, params[0].str);
			}
			else
				parse_error(ERR_EXPECTED_PARAM_STR, &fa);
			continue;
		}

		if (strcmp("ORG", opcode) == 0)
		{
			if (params[0].type == P_VAL)
				cur_addr = params[0].val;
			else
				parse_error(ERR_EXPECTED_PARAM_VAL, &fa);
			continue;
		}

		if (strcmp("SEGMENT", opcode) == 0)
		{
			if(params[0].type == P_STR)
			{
				segment_addr[cur_segment] = cur_addr;
				if (strcmp("DATA", params[0].str) == 0)
					cur_segment = SEGMENT_DATA;
				if (strcmp("CODE", params[0].str) == 0)
					cur_segment = SEGMENT_CODE;
				cur_addr = segment_addr[cur_segment];
			}
			else
				parse_error(ERR_EXPECTED_PARAM_STR, &fa);
			continue;
		}

		opc_t *opc;
		opc_t *opc_ext = NULL;

		opc = find_opcode(opcode, params_count, opcodes, opcodes_size, &fa);
		opcode_size = opc->size & ~P_EXT;

		verify_params(opc, params, params_count, &fa);

		if (opc->size & P_EXT)
		{
			if (opcode_ext)
			{
				opc_ext = find_opcode(opcode_ext, params_count_ext, opcodes_ext, opcodes_ext_size, &fa);
				verify_params(opc_ext, params_ext, params_count_ext, &fa);
			}
			else if (params_count_ext)
				parse_error(ERR_EXT_PAR_NOT_EXT, &fa);
		}
		else
		{
			if (opcode_ext)
				parse_error(ERR_EXT_CANT_EXTEND_OPCODE, &fa);
			if (params_count_ext)
				parse_error(ERR_EXT_PAR_NOT_EXT, &fa);
		}

		if (pass == 2)
		{
			// generate binary
			((uint16 *)gdg->buffer)[cur_addr] = 0x0000;
			build_code(opc, params, params_count, (uint16 *)gdg->buffer);
			if (opc_ext)
				build_code(opc_ext, params_ext, params_count_ext, (uint16 *)gdg->buffer);
		}

		cur_addr += opcode_size;
	};
	if (gdg->buffer == NULL)
	{
		gdg->buffer_size = cur_addr;
		gdg->buffer = (char *)malloc(gdg->buffer_size * sizeof(uint16) + 4);
		memset(gdg->buffer, 0, gdg->buffer_size * sizeof(uint16));
	}
	fclose(fa.fsrc);
}

