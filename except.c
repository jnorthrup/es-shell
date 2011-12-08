/* except.c -- exception mechanism ($Revision: 1.6 $) */

#include "es.h"
#include "print.h"

/* globals */
Handler *tophandler = NULL;
Handler *roothandler = NULL;
List *exception = NULL;
Push *pushlist = NULL;

/* pophandler -- remove a handler */
extern void pophandler(Handler *handler) {
	assert(tophandler == handler);
	assert(handler->rootlist == rootlist);
	tophandler = handler->up;
}

/* throw -- raise an exception */
extern noreturn throw(List *e) {
	Handler *handler = tophandler;

	assert(!gcisblocked());
	assert(e != NULL);
	assert(handler != NULL);
	tophandler = handler->up;
	
	while (pushlist != handler->pushlist) {
		rootlist = &pushlist->defnroot;
		varpop(pushlist);
	}

#if ASSERTIONS
	for (; rootlist != handler->rootlist; rootlist = rootlist->next)
		assert(rootlist != NULL);
#else
	rootlist = handler->rootlist;
#endif
	exception = e;
	longjmp(handler->label, 1);
	NOTREACHED;
}

/* fail -- pass a user catchable error up the exception chain */
extern noreturn fail VARARGS2(const char *, from, const char *, fmt) {
	char *s;
	va_list args;

	VA_START(args, fmt);
	s = strv(fmt, args);
	va_end(args);

	gcdisable(0);
	Ref(List *, e, mklist(mkterm("error", NULL),
			      mklist(mkterm((char *) from, NULL),
				     mklist(mkterm(s, NULL), NULL))));
	while (gcisblocked())
		gcenable();
	throw(e);
	RefEnd(e);
}

/* newchildcatcher -- remove the current handler chain for a new child */
extern void newchildcatcher(void) {
	tophandler = roothandler;
}
