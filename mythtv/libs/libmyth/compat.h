// -*- Mode: c++ -*-
// Simple header which encapsulates platform incompatibilities, so we
// do not need to litter the codebase with ifdefs.

#ifndef __COMPAT_H__
#define __COMPAT_H__

// Turn off the visual studio warnings (identifier was truncated)
#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#ifdef _WIN32 
#include <windows.h>
#endif

// Dealing with Microsoft min/max mess: 
// assume that under Windows the code is compiled with NOMINMAX defined
// which disables #define's for min/max.
// however, Microsoft  violates the C++ standard even with 
// NOMINMAX on, and defines templates _cpp_min and _cpp_max 
// instead of templates min/max
// define the correct templates here

#if defined(__cplusplus) && defined(_WIN32)
template<class _Ty> inline
        const _Ty& max(const _Ty& _X, const _Ty& _Y)
        {return (_X < _Y ? _Y : _X); }
template<class _Ty, class _Pr> inline
	const _Ty& max(const _Ty& _X, const _Ty& _Y, _Pr _P)
	{return (_P(_X, _Y) ? _Y : _X); }

template<class _Ty> inline
        const _Ty& min(const _Ty& _X, const _Ty& _Y)
        {return (_Y < _X ? _Y : _X); }
template<class _Ty, class _Pr> inline
	const _Ty& min(const _Ty& _X, const _Ty& _Y, _Pr _P)
	{return (_P(_Y, _X) ? _Y : _X); }
#endif // defined(__cplusplus) && defined(_WIN32)

#ifdef _WIN32
#undef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
inline double drand48(void) { return double(rand())/double(RAND_MAX); }
#endif

#ifdef USING_MINGW
inline int random(void)
{
    srand(GetTickCount());
    return rand() << 20 ^ rand() << 10 ^ rand();
}
#endif // USING_MINGW

#if defined(__cplusplus) && defined(USING_MINGW)
#include <pthread.h>
inline bool operator==(const pthread_t& pt, const int n)
{
    return *((int*)(&pt)) == n;
}
inline bool operator!=(const pthread_t& pt, const int n)
{
    return *((int*)(&pt)) != n;
}
#endif // defined(__cplusplus) && defined(USING_MINGW)

#ifdef USING_MINGW
/* TODO: most small usleep's in MythTV are just a quick way to perform
 * a yield() call, those should just be replaced with an actual yield()
 * invocation.
 */
inline int usleep(unsigned int timeout)
{
    /*
    // windows seems to have 1us-resolution timers,
    // however this produces the same results as Sleep
    HANDLE hTimer = ::CreateWaitableTimer(NULL, TRUE, NULL);
    if (hTimer) {
        LARGE_INTEGER li;
        li.QuadPart = -((int)timeout * 10);
        if (SetWaitableTimer(hTimer, &li, 0, NULL, 0, FALSE)) {
            DWORD res = WaitForSingleObject(hTimer, (timeout / 1000 + 1));
            if (res == WAIT_TIMEOUT || res == WAIT_OBJECT_0)
                return 0;
        }
        CloseHandle(hTimer);
    }
    */
    //fallback
    Sleep(timeout < 1000 ? 1 : (timeout + 500) / 1000);
    return 0;
}
#endif // USING_MINGW

#ifdef USING_MINGW
inline unsigned sleep(unsigned int x)
{
    Sleep(x * 1000);
    return 0;
}
#endif // USING_MINGW

#ifdef USING_MINGW
struct statfs {
//   long    f_type;     /* type of filesystem */
   long    f_bsize;    /* optimal transfer block size */
   long    f_blocks;   /* total data blocks in file system */
//   long    f_bfree;    /* free blocks in fs */
   long    f_bavail;   /* free blocks avail to non-superuser */
//   long    f_files;    /* total file nodes in file system */
//   long    f_ffree;    /* free file nodes in fs */
//   long    f_fsid;     /* file system id */
//   long    f_namelen;  /* maximum length of filenames */
//   long    f_spare[6]; /* spare for later */
};
inline int statfs(const char* path, struct statfs* buffer)
{
    DWORD spc = 0, bps = 0, fc = 0, c = 0;

    if (buffer && GetDiskFreeSpaceA(path, &spc, &bps, &fc, &c))
    {
        buffer->f_bsize = bps;
        buffer->f_blocks = spc * c;
        buffer->f_bavail = spc * fc;
        return 0;
    }

    return -1;
}
#endif // USING_MINGW

#ifdef USING_MINGW
#define lstat stat
#define bzero(x, y) memset((x), 0, (y))
#define nice(x) ((int)!::SetThreadPriority(\
                    ::GetCurrentThread(), ((x) < -10) ? \
                        2 : (((x) < 0) ? \
                        1 : (((x) > 10) ? \
                        2 : (((x) > 0) ? 1 : 0)))))
#define PRIO_PROCESS 0
#define setpriority(x, y, z) ((x) == PRIO_PROCESS && y == 0 ? nice(z) : -1)
#endif // USING_MINGW

#ifdef USING_MINGW
//signals: not tested
#define SIGHUP 1
#define SIGPIPE 3   // not implemented in MINGW, will produce "unable to ignore sigpipe"
#define SIGALRM 13

#define S_IRGRP 0
#define S_IROTH 0
#define O_SYNC 0
#endif // USING_MINGW

#ifdef USING_MINGW
#define mkfifo(path, mode) \
    (int)CreateNamedPipeA(path, PIPE_ACCESS_DUPLEX | WRITE_DAC, \
                          PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, \
                          1024, 1024, 10000, NULL)
#endif // USING_MINGW

#ifdef USING_MINGW
#define RTLD_LAZY 0
#define dlopen(x, y) LoadLibraryA((x))
#define dlclose(x) !FreeLibrary((HMODULE)(x))
#define dlsym(x, y) GetProcAddress((HMODULE)(x), (y))
#define dlerror GetLastError
#endif // USING_MINGW

#ifdef USING_MINGW
// getuid/geteuid/setuid - not implemented
#define getuid() 0
#define geteuid() 0
#define setuid(x)
#endif // USING_MINGW

#ifdef USING_MINGW
#define	timeradd(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;			      \
    if ((result)->tv_usec >= 1000000)					      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_usec -= 1000000;					      \
      }									      \
  } while (0)
#define	timersub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)
#endif // USING_MINGW

#ifdef USING_MINGW
// TODO this stuff is not implemented yet
#define daemon(x, y) -1
#define getloadavg(x, y) -1
#endif // USING_MINGW

#ifdef USING_MINGW
// this stuff is untested
#define WIFEXITED(w)	(((w) & 0xff) == 0)
#define WIFSIGNALED(w)	(((w) & 0x7f) > 0 && (((w) & 0x7f) < 0x7f))
#define WIFSTOPPED(w)	(((w) & 0xff) == 0x7f)
#define WEXITSTATUS(w)	(((w) >> 8) & 0xff)
#define WTERMSIG(w)	((w) & 0x7f)
#endif // USING_MINGW

#endif // __COMPAT_H__
