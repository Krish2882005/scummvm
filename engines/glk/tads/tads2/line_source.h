/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/* Line source definitions
 *
 * A line source is a mechanism for reading source text.  The tokenizer
 * reads its input from a line source.  This is the basic class
 * definition; individual line sources will define the functions and
 * class data needed.
 */

#ifndef GLK_TADS_TADS2_LINE_SOURCE
#define GLK_TADS_TADS2_LINE_SOURCE

#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/object.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/**
 * Line source superclass structure
 */
struct lindef {
	int   (*lingetp)(lindef *lin);                         /* get next line */
	void  (*linclsp)(lindef *lin);                     /* close line source */
	void  (*linppos)(lindef *lin, char *buf, uint buflen);
			/* write printable rep of position to buf (for error reporting) */
	void  (*linglop)(lindef *lin, uchar *buf);
					   /* generate a line record for an OPCLINE instruction */
	int   (*linwrtp)(lindef *lin, osfildef *fp);
			/* write line source information to binary file; TRUE ==> error */
	void  (*lincmpp)(lindef *lin, uchar *buf);
		  /* give location of compiled code for current line to line source */
	void  (*linactp)(lindef *lin);                      /* activate for dbg */
	void  (*lindisp)(lindef *lin);                           /* disactivate */
	void  (*lintellp)(lindef *lin, uchar *pos);             /* get position */
	void  (*linseekp)(lindef *lin, uchar *pos);                     /* seek */
	int   (*linreadp)(lindef *lin, uchar *buf, uint siz);          /* fread */
	void  (*linpaddp)(lindef *lin, uchar *pos, long delta);
									   /* add an offset to a position value */
	int   (*linqtopp)(lindef *lin, uchar *pos);                  /* at top? */
	int   (*lingetsp)(lindef *lin, uchar *buf, uint siz);       /* get line */
	void  (*linnamp)(lindef *lin, char *buf);            /* get source name */
	void  (*linfindp)(lindef *lin, char *buf, objnum *objp,
					  uint *ofsp);              /* find nearest line record */
	void  (*lingotop)(lindef *lin, int where);               /* seek global */
	long  (*linofsp)(lindef *lin);            /* byte offset in line source */
	void  (*linrenp)(lindef *lin, objnum oldnum, objnum newnum);
									   /* renumber an object (for "modify") */
	void  (*lindelp)(lindef *lin, objnum objn);
										/* delete an object (for "replace") */
	ulong (*linlnump)(lindef *lin);          /* get the current line number */
#define  LINGOTOP   OSFSK_SET                   /* go to top of line source */
#define  LINGOEND   OSFSK_END                   /* go to end of line source */
	lindef *linpar;                        /* parent of current line source */
	lindef *linnxt;                       /* next line in line source chain */
	int     linid;           /* serial number of line source (for debugger) */
	char   *linbuf;                              /* pointer to current line */
	ushort  linflg;                                                /* flags */
#define  LINFEOF   0x01                    /* line source is at end of file */
#define  LINFMORE  0x02             /* there's more to the line than linlen */
#define  LINFDBG   0x04          /* debug record already generated for line */
#define  LINFNOINC 0x08        /* ignore # directives from this line source */
#define  LINFCMODE 0x10                  /* line source is parsed in C-mode */
	ushort  linlen;                                   /* length of the line */
	ushort  linlln;           /* length of line record generated by lingloc */
};

/**
 *   Maximum allowed value for linlln, in bytes.  This allows subsystems
 *   that need to maintain local copies of seek locations to know how big
 *   an area to allocate for them.
 */
#define LINLLNMAX   20

/* macros to cover calls to functions */
#define linget(lin) ((*((lindef *)(lin))->lingetp)((lindef *)(lin)))
#define lincls(lin) ((*((lindef *)(lin))->linclsp)((lindef *)(lin)))
#define linppos(lin, buf, buflen) \
 ((*((lindef *)(lin))->linppos)((lindef *)(lin), buf, buflen))
#define linglop(lin, buf) ((*((lindef *)(lin))->linglop)(lin, buf))
#define linwrt(lin, fp) ((*((lindef *)(lin))->linwrtp)(lin, fp))
#define lincmpinf(lin, buf) ((*((lindef *)(lin))->lincmpp)(lin, buf))
#define linactiv(lin) ((*((lindef *)(lin))->linactp)(lin))
#define lindisact(lin) ((*((lindef *)(lin))->lindisp)(lin))
#define lintell(lin, pos) ((*((lindef *)(lin))->lintellp)(lin, pos))
#define linseek(lin, pos) ((*((lindef *)(lin))->linseekp)(lin, pos))
#define linread(lin, buf, siz) ((*((lindef *)(lin))->linreadp)(lin, buf, siz))
#define linpadd(lin, pos, delta) \
  ((*((lindef *)(lin))->linpaddp)(lin, pos, delta))
#define linqtop(lin, pos) ((*((lindef *)(lin))->linqtopp)(lin, pos))
#define lingets(lin, buf, siz) ((*((lindef *)(lin))->lingetsp)(lin, buf, siz))
#define linnam(lin, buf) ((*((lindef *)(lin))->linnamp)(lin, buf))
#define linlnum(lin) ((*((lindef *)(lin))->linlnump)(lin))
#define linfind(lin, buf, objp, ofsp) \
  ((*((lindef *)(lin))->linfindp)(lin, buf, objp, ofsp))
#define lingoto(lin, where) ((*((lindef *)(lin))->lingotop)(lin, where))
#define linofs(lin) ((*((lindef *)(lin))->linofsp)(lin))
#define linrenum(lin, oldnum, newnum) \
  ((*((lindef *)(lin))->linrenp)(lin, oldnum, newnum))
#define lindelnum(lin, objn) ((*((lindef *)(lin))->lindelp)(lin, objn))

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
