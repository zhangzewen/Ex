#incldue "evbuf.h"

/*some systems do not have MAP_FAILD*/
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#if defined(_EVENT_HAVE_SYS_SENDFILE_H) && defined(_EVENT_HAVE_SENDFILE) && defined(__linux__)
#define USE_SENDFILE    1     
#define SENDFILE_IS_LINUX 1   
#elif defined(_EVENT_HAVE_SENDFILE) && defined(__FreeBSD__)
#define USE_SENDFILE    1     
#define SENDFILE_IS_FREEBSD 1 
#elif defined(_EVENT_HAVE_SENDFILE) && defined(__APPLE__)
#define USE_SENDFILE    1
#define SENDFILE_IS_MACOSX  1
#elif defined(_EVENT_HAVE_SENDFILE) && defined(__sun__) && defined(__svr4__)
#define USE_SENDFILE    1
#define SENDFILE_IS_SOLARIS 1
#endif 

#ifdef USE_SENDFILE           
static int use_sendfile = 1;  
#endif 
#ifdef _EVENT_HAVE_MMAP       
static int use_mmap = 1;
#endif 



/*Mask of user-selectable callback flags.*/
#define EVBUFFER_CB_USER_FLAGS	0Xffff
/*Mask of all internal-use-only flags.*/
#define EVBUFFER_CB_INTERNAL_FLAGS 0xffff0000

/*Flag set if the callback is using the cb_obsolete function pointer */
#define EVBUFFER_CB_OBSOLETE			0x00040000

/*evbuffer_chain support*/

#define CHAIN_SPACE_PTR(ch) ((ch)->buffer + (ch)->misalign + (ch)->off)
