/**
 @file		myprintf.h
 */

#ifndef __MYPRINTF_H
#define __MYPRINTF_H

#include <stdio.h>
//#include <avr/pgmspace.h>

//#define DEBUG_MODE

#define PRINT(x) (			\
{					\
	static prog_char str[] = x;	\
	printf(str);			\
}					\
)

#define PRINT1(x,arg1) (		\
{					\
	static prog_char str[] = x;	\
	printf(str,arg1);		\
}					\
)

#define PRINT2(x,arg1,arg2) (		\
{					\
	static prog_char str[] = x;	\
	printf(str,arg1,arg2);	\
}					\
)

#define PRINT3(x,arg1,arg2,arg3) (	\
{					\
	static prog_char str[] = x;	\
	printf(str,arg1,arg2,arg3);	\
}					\
)

#define PRINT4(x,arg1,arg2,arg3,arg4) (		\
{						\
	static prog_char str[] = x;		\
	printf(str,arg1,arg2,arg3,arg4);	\
}						\
)

#define PRINTLN(x) (				\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str);				\
}						\
)

#define PRINTLN1(x,arg1) (			\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str,arg1);			\
}						\
)

#define PRINTLN2(x,arg1,arg2) (			\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str,arg1,arg2);		\
}						\
)

#define PRINTLN3(x,arg1,arg2,arg3) (		\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str,arg1,arg2,arg3);		\
}						\
)

#define PRINTLN4(x,arg1,arg2,arg3,arg4) (	\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str,arg1,arg2,arg3,arg4);	\
}						\
)

#ifdef DEBUG_MODE

#define DPRINT(x) (			\
{					\
	static prog_char str[] = x;	\
	printf(str);			\
}					\
)


#define DPRINT1(x,arg1) (		\
{					\
	static prog_char str[] = x;	\
	printf(str,arg1);		\
}					\
)

#define DPRINT2(x,arg1,arg2) (		\
{					\
	static prog_char str[] = x;	\
	printf(str,arg1,arg2);	\
}					\
)

#define DPRINT3(x,arg1,arg2,arg3) (	\
{					\
	static prog_char str[] = x;	\
	printf(str,arg1,arg2,arg3);	\
}					\
)

#define DPRINT4(x,arg1,arg2,arg3,arg4) (	\
{						\
	static prog_char str[] = x;		\
	printf(str,arg1,arg2,arg3,arg4);	\
}						\
)

#define DPRINTLN(x) (				\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str);				\
}						\
)						

#define DPRINTLN1(x,arg1) (			\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str,arg1);			\
}						\
)

#define DPRINTLN2(x,arg1,arg2) (		\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str,arg1,arg2);		\
}						\
)

#define DPRINTLN3(x,arg1,arg2,arg3) (		\
{						\
	static prog_char str[] = x"\r\n";	\
	printf(str,arg1,arg2,arg3);		\
}						\
)

#define DPRINTLN4(x,arg1,arg2,arg3,arg4) (		\
{							\
	static prog_char str[] = x"\r\n";		\
	printf(str,arg1,arg2,arg3,arg4);		\
}							\
)

#else

#define DPRINT(x) {}

#define DPRINT1(x,arg1) {}

#define DPRINT2(x,arg1,arg2) {}

#define DPRINT3(x,arg1,arg2,arg3) {}

#define DPRINT4(x,arg1,arg2,arg3,arg4) {}

#define DPRINTLN(x) {}

#define DPRINTLN1(x,arg1) {}

#define DPRINTLN2(x,arg1,arg2) {}

#define DPRINTLN3(x,arg1,arg2,arg3) {}

#define DPRINTLN4(x,arg1,arg2,arg3,arg4) {}

#endif

#endif
