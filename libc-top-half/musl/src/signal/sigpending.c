#include <signal.h>
#include <errno.h>
#ifdef __wasilibc_unmodified_upstream
#include "syscall.h"
#else
#include <wasi/api.h>
#include "pthread_impl.h"
#include "sigmask.h"
#if defined(__wasm64__)
__attribute__((import_module("wasix_64v1"), import_name("verkosto_signal_pending_v1")))
#else
__attribute__((import_module("wasix_32v1"), import_name("verkosto_signal_pending_v1")))
#endif
extern unsigned long long __pending_process_signals(void);
#endif

int sigpending(sigset_t *set)
{
#ifdef __wasilibc_unmodified_upstream
	return syscall(SYS_rt_sigpending, set, _NSIG/8);
#else
	/* A signal the host has queued but not yet handed over is not in our
	 * pending set yet. Delivery happens at syscall boundaries, so yielding
	 * flushes the queue into it (blocked ones land in sigpending, deliverable
	 * ones run their handlers) and makes the answer current. */
	(void)__wasi_sched_yield();
	__sig_critical_enter();
	__sigset_copy(set, &__pthread_self()->sigpending);
	unsigned long long shared = __pending_process_signals();
	__sigset_or(set, &shared);
	__sig_critical_leave();
	return 0;
#endif
}