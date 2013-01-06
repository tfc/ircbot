#ifndef __HELPERS_H__
#define __HELPERS_H__

#ifndef MIN
#define MIN(a,b) \
({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b; \
})
#endif

#ifndef MAX
#define MAX(a,b) \
({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; \
})
#endif


#define FN_APPLY(type, fn, ...) { \
	void *__stopper = (int[]){0}; \
	type **__list = (type*[]){__VA_ARGS__, __stopper}; \
    int i; \
	for (i=0; __list[i] != __stopper; i++) \
		fn(__list[i]); \
}

/* vectorized free() procedure.
 * Just use it like free(ptr1, ptr2, ptr3); on any kind of pointers.
 */
#define Free_list(...) FN_APPLY(void, free, __VA_ARGS__)

/* Safe asprintf function. This is nice an handy for 
 * multiple manipulations on the same string, since
 * it doesn't leak memory.
 */
#define Sasprintf(write_to, ...) { \
	char *__tmp_str = write_to; \
	asprintf(&(write_to), __VA_ARGS); \
	free(__tmp_str); \
}

#endif /* __HELPERS_H__ */

