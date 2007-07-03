#ifndef TRAVERSO_TYPES_H
#define TRAVERSO_TYPES_H

#include <inttypes.h>
#include "FastDelegate.h"

// Implementation for atomic int get/set from glibc's atomic.h/c
// to get rid of the glib dependency!
// Arches that need memory bariers: ppc (which we support)
// and sparc, alpha, ia64 which we do not support ??

#if defined(__ppc__) || defined(__powerpc__) || defined(__PPC__)

#  define T_ATOMIC_MEMORY_BARRIER __asm__ ("sync" : : : "memory")

static inline int t_atomic_int_get (volatile int *atomic)
{
	T_ATOMIC_MEMORY_BARRIER;
	return *atomic;
}

static inline void t_atomic_int_set (volatile int *atomic, int newval)
{
	*atomic = newval;
	T_ATOMIC_MEMORY_BARRIER; 
}

#else

# define t_atomic_int_get(atomic) 		(*(atomic))
# define t_atomic_int_set(atomic, newval) 	((void) (*(atomic) = (newval)))

#endif // ENDIF __ppc__


using namespace fastdelegate;

/**
 * Type used to represent sample frame counts.
 */
typedef uint32_t     nframes_t;

enum {
	TransportStopped = 0,
	TransportRolling = 1,
  	TransportLooping = 2,
  	TransportStarting = 3
};

typedef struct {
	int tranport;
	nframes_t frame;
	bool isSlave;
	bool realtime;
} transport_state_t;

/**
 * Type used to represent the value of free running
 * monotonic clock with units of microseconds.
 */
typedef double trav_time_t;

typedef unsigned long          channel_t;

typedef float audio_sample_t;


typedef FastDelegate1<nframes_t, int> ProcessCallback;
typedef FastDelegate0<int> RunCycleCallback;
typedef FastDelegate1<transport_state_t, int> TransportControlCallback;



/**
 * Used for the type argument of jack_port_register() for default
 * audio ports.
 */
#define JACK_DEFAULT_AUDIO_TYPE "32 bit float mono audio"


/**
 *  A port has a set of flags that are formed by AND-ing together the
 *  desired values from the list below. The flags "PortIsInput" and
 *  "PortIsOutput" are mutually exclusive and it is an error to use
 *  them both.
 */
enum PortFlags {

     /**
	 * if PortIsInput is set, then the port can receive
	 * data.
      */
	PortIsInput = 0x1,

     /**
  * if PortIsOutput is set, then data can be read from
  * the port.
      */
 PortIsOutput = 0x2,

     /**
  * if PortIsPhysical is set, then the port corresponds
  * to some kind of physical I/O connector.
      */
 PortIsPhysical = 0x4,

     /**
  * if PortCanMonitor is set, then a call to
  * jack_port_request_monitor() makes sense.
  *
  * Precisely what this means is dependent on the client. A typical
  * result of it being called with TRUE as the second argument is
  * that data that would be available from an output port (with
  * PortIsPhysical set) is sent to a physical output connector
  * as well, so that it can be heard/seen/whatever.
  *
  * Clients that do not control physical interfaces
  * should never create ports with this bit set.
      */
 PortCanMonitor = 0x8,

     /**
      * PortIsTerminal means:
  *
  *	for an input port: the data received by the port
  *                    will not be passed on or made
  *		           available at any other port
  *
  * for an output port: the data available at the port
  *                    does not originate from any other port
  *
  * Audio synthesizers, I/O hardware interface clients, HDR
  * systems are examples of clients that would set this flag for
  * their ports.
      */
 PortIsTerminal = 0x10
};


#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <time.h>
#ifndef _TIMEVAL_DEFINED /* also in winsock[2].h */
#define _TIMEVAL_DEFINED
struct timeval {
   long tv_sec;
   long tv_usec;
};
#endif /* _TIMEVAL_DEFINED */
#else
#  include <sys/time.h>
#endif


#if defined(_MSC_VER) || defined(__MINGW32__)

#include <Windows.h>

static inline int gettimeofday(struct timeval* tp, void* tzp) {
	DWORD t;
	t = timeGetTime();
	tp->tv_sec = t / 1000;
	tp->tv_usec = t % 1000;
	/* 0 indicates that the call succeeded. */
	return 0;
}
	
typedef uint8_t            u_int8_t;

#endif

static inline trav_time_t get_microseconds()
{
	struct timeval now;
	gettimeofday(&now, 0);
	trav_time_t time = (now.tv_sec * 1000000.0 + now.tv_usec);
	return time;
}


#endif

//eof
