/* clockreset.c: Generate clock reset code for Pokemon Gold/Silver Version
 * Copyright (C) 2009, 2010 Kirn Gill <segin2005@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Notes:
 * The multiplication "x" (as opposed to the Latin character "x")
 * is represented by an askterisk; The PK glyph is > and the
 * MN glyph is < in this table
 */
  
struct char_trans_t { 
	char	m_char;
	int	m_code;
} transtable[] = {
	{ 'A', 128 },	{ 'a', 160 },
	{ 'B', 129 },	{ 'b', 161 },	{ '-', 227 },
	{ 'C', 130 },	{ 'c', 162 },	{ '?', 230 },
	{ 'D', 131 },	{ 'd', 163 },	{ '/', 243 },
	{ 'E', 132 },	{ 'e', 164 },	{ '.', 232 },
	{ 'F', 133 },	{ 'f', 165 },	{ ',', 244 },
	{ 'G', 134 },	{ 'g', 166 },	{ '(', 154 },
	{ 'H', 135 },	{ 'h', 167 },	{ ')', 155 },
	{ 'I', 136 },	{ 'i', 168 },	{ ':', 156 },
	{ 'J', 137 },	{ 'j', 169 },	{ ';', 157 },
	{ 'K', 138 },	{ 'k', 170 },	{ '[', 158 },
	{ 'L', 139 },	{ 'l', 171 },	{ ']', 159 },
	{ 'M', 140 },	{ 'm', 172 },	{ '>', 225 },
	{ 'N', 141 },	{ 'n', 173 },	{ '<', 226 },
	{ 'O', 142 },	{ 'o', 174 },	{ '!', 231 },
	{ 'P', 143 },	{ 'p', 175 },	{ '*', 241 },
	{ 'Q', 144 },	{ 'q', 176 },	{ ' ', 000 },
	{ 'R', 145 },	{ 'r', 177 },
	{ 'S', 146 },	{ 's', 178 },
	{ 'T', 147 },	{ 't', 179 },
	{ 'U', 148 },	{ 'u', 180 },
	{ 'V', 149 },	{ 'v', 181 },
	{ 'W', 150 },	{ 'w', 182 },
	{ 'X', 151 },	{ 'x', 183 },
	{ 'Y', 152 },	{ 'y', 184 },
	{ 'Z', 153 },	{ 'z', 185 }
};
int lookup_char(char c) {
	/* Lookup character in translation table */
	int tablesize = sizeof(transtable) / sizeof(struct char_trans_t);
	int charcode, x;
	for(x = 0; x < tablesize; x++) { 
		if (transtable[x].m_char == c) {
			charcode = transtable[x].m_code;
		}
	}
	return charcode;
}

int main(void) 
{
	char name[10];
	int id, cash, x, code;
	printf( "Clock Reset code generator for Pokemon Gold/Silver\n"
		"We are going to need three things to generate your\n"
		"reset code - Your character name, your trainer ID,\n"
		"and the amount of money your currently possess.\n\n" 
	       );
	printf(	"Now, we need your character name. Type it EXACTLY\n"
		"including letter case; If your name is SEGIN, then\n"
		"type SEGIN, not Segin, or sEGIN, or whatever.\n\n"
		"For the multiplication cross (the times \"x\"), type\n"
		"an askterisk (*), for the PK symbol, type a \">\", and\n"
		"for the MN symbol, type a \"<\".\n\nname> "
	      );
	memset(name, 0, 10);
	fgets(name, 6, stdin);
#ifndef __WIN32__
	fpurge(stdin);
#endif /* __WIN32__ */	
	printf("\nNow, enter your trainer ID.\n\nID> ");
	scanf("%d",&id);
	
	printf(	"\nAnd finally, the current amount of money your\n"
		"character has on them. (Do not count the amount\n"
		"you have saved with Mom.)\n\ncash> $"
	      );
	scanf("%d",&cash);
	
	code = 0;
	id = id & 65535;
	cash = cash & 65535;
	for (x = 0; x < 5 ; x++) 
		code += lookup_char(name[x]);
	code += cash / 256;
	code += cash % 256;
	code += id / 256;
	code += id % 256;
	printf("Your clock reset code is %05d.\n", code);
	return(0);
}
