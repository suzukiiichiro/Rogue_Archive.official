/*
 * C copy protection routines
 */

#include	"swint.h"

extern int goodchk;
int no_step;

#define UNDEFINED	0
#define DONTCARE	0

#define	CRC		0x10

static struct sw_regs rom_read = {
	0x206,
	0,
	0x2701,
	UNDEFINED,
	DONTCARE,
	DONTCARE,
	DONTCARE,
	0xF800
} ;

static struct sw_regs sig1_read = {
	0x201,
	UNDEFINED,
	0x2707,
	UNDEFINED,
	DONTCARE,
	DONTCARE,
	DONTCARE,
	UNDEFINED
} ;

static struct sw_regs sig2_read = {
	0x201,
	UNDEFINED,
	0x27F1,
	UNDEFINED,
	DONTCARE,
	DONTCARE,
	DONTCARE,
	UNDEFINED
} ;

protect(drive)
{
	int i, flags;
	struct sw_regs rgs;
	char buf2[512];
	char buf1[32];

	no_step++;
	rom_read.dx = sig1_read.dx = sig2_read.dx = drive;
	sig1_read.es = sig2_read.es = getds();
	sig1_read.bx = (int)(&buf1[0]);
	sig2_read.bx = (int)(&buf2[0]);

	for (i=0,flags=CF;i<7 && (flags&CF);i++)
	{
		rgs = rom_read;
		no_step = 0;
		flags = sysint(SW_DSK,&rgs,&rgs);
		no_step++;
	}
	if (CF&flags)
	{
		no_step = 0;
		return;
	}
	for (i=0,flags=CF;i<3 && (flags&CF);i++)
	{
		rgs = sig1_read;
		no_step = 0;
		flags = sysint(SW_DSK,&rgs,&rgs);
		no_step++;
	}
	if (CF&flags)
	{
		no_step = 0;
		return;
	}
	for (i=0;i<4;i++)
	{
		rgs = sig2_read;
		no_step = 0;
		flags = sysint(SW_DSK,&rgs,&rgs);
		no_step++;
		if ((flags&CF) && HI(rgs.ax) == CRC)
		{
			if (memcmp(&buf1[0],&buf2[0x8c],32) == 0)
				goodchk = 0xD0D;
			no_step = 0;
			return ;
		}
	}
	no_step = 0;
}
