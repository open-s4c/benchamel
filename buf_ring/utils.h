#define NOP() do {} while(0)
#define powerof2(X) 1
#define KASSERT(A,B) assert(A && B)
#define critical_enter() NOP()
#define critical_exit() NOP()
#define ENOBUFS -1
#define wmb() __sync_synchronize()
#define rmb() __sync_synchronize()
#ifdef VSYNC_VERIFICATION
extern int __VERIFIER_assume(int);
#define cpu_spinwait() __VERIFIER_assume(0)
#else
#define cpu_spinwait() NOP()
#endif
struct malloc_type {};
struct mtx {};
#define M_ZERO 0
#define atomic_cmpset_int(dst, expect, src) \
        __atomic_compare_exchange_n(dst, &expect, src, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

