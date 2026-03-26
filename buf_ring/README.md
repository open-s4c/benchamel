`buf_ring` from FreeBSD version 8.0.0

- https://svnweb.freebsd.org/base/release/8.0.0/sys/sys/buf_ring.h?revision=199625&view=markup
- compare with HEAD: http://web.mit.edu/freebsd/head/sys/sys/buf_ring.h

The algorithm is quite simple, but has a bug (and no barriers). The latest
version of FreeBSD has added barriers, but the bug is still there.

# ABA bug

A thread tries to enqueue, gets preempted, and the whole buffer rotates, then
the CAS succeeds and prod head gets incremented although it shouldn't.

Link to the bug: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=246475

"Other ring-buffer implementations such as DPDK [3] take a slightly
different approach, ... this would be to let prod_head take values across
the whole uint32 range and use the mask only when reading or writing
entries. For the ABA to occur, the whole uint32 range would need to be
wrapped-around -which looks unrealistic-."

Define BUGFIX in `test.c` to fix the bug.  This bug cannot be easily found by
hw or random schedulers, but pos finds it within 400 iterations.

# Incorrect bugfix

To fix the previous bug, we avoid using the modulo operation when incrementing
head and tail, and apply the modulo operation only when accessing the array. In
this way, the head and tail also serve as a version number since they never
decrease (expect on overflow). However, I introduced a bug myself here because
I forgot to fix the limit check of the producer.

In the old code (buggy code) the check was done like this:

	if (prod_next == cons_tail) {

With the fix, the check should look like this;

	if (prod_head - cons_tail == br->br_prod_size) {

Define BUGFIX2 in `test.c` to fix the bug. This bug is found by random and pos
after 1..10 iterations. hw scheduler is not so lucky (on my machine).
