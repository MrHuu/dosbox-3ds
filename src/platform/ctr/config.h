#define VERSION "SVN"

/* Define to 1 to enable internal debugger, requires libcurses */
#define C_DEBUG 0

/* Define to 1 to enable output=ddraw */
#define C_DDRAW 0 

/* Define to 1 to enable screenshots, requires libpng */
#define C_SSHOT 0

/* Define to 1 to enable movie recording, requires zlib built without Z_SOLO */
#define C_SRECORD 0

/* Define to 1 to use opengl display output support */
#define C_OPENGL 0

/* Define to 1 to enable internal modem support, requires SDL_net */
#define C_MODEM 1

/* Define to 1 to enable IPX networking support, requires SDL_net */
#define C_IPX 1

/* Enable some heavy debugging options */
#define C_HEAVY_DEBUG 0

/* The type of cpu this host has */
#define C_TARGETCPU ARMV6KLE

/* Define to 1 to use x86 dynamic cpu core */
#define C_DYNAMIC_X86 0

/* Define to 1 to use recompiling cpu core. Can not be used together with the dynamic-x86 core */
#define C_DYNREC 1

/* Enable memory function inlining in */
#define C_CORE_INLINE 1

/* Enable the FPU module, still only for beta testing */
#define C_FPU 1

/* Define to 1 to use a x86 assembly fpu core */
#define C_FPU_X86 0

/* Define to 1 to use a unaligned memory access */
#define C_UNALIGNED_MEMORY 0

/* environ is defined */
#define ENVIRON_INCLUDED 0

/* environ can be linked */
#define ENVIRON_LINKED 0

/* Define to 1 if you want serial passthrough support. */
#define C_DIRECTSERIAL 0

#define GCC_ATTRIBUTE(x) __attribute__ ((x))
#define GCC_UNLIKELY(x)  __builtin_expect((x), 0)
#define GCC_LIKELY(x)    (x)

#define INLINE inline
#define DB_FASTCALL __fastcall

/* The internal types */
typedef             double  Real64;

typedef      unsigned char  Bit8u;
typedef        signed char  Bit8s;
typedef     unsigned short  Bit16u;
typedef       signed short  Bit16s;
typedef       unsigned int  Bit32u;
typedef         signed int  Bit32s;
typedef unsigned long long  Bit64u;
typedef   signed long long  Bit64s;
typedef       unsigned int  Bitu;
typedef             Bit32s  Bits;

#define sBit32t
#define sBit32fs(a) sBit32t #a
#define sBitfs      sBit32fs
