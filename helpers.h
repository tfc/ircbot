
#define MIN(a,b) \
({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b; \
})

#define FN_APPLY(type, fn, ...) { \
	void *__stopper = (int[]){0}; \
	type **__list = (type*[]){__VA_ARGS__, __stopper}; \
	for (int i=0; __list[i] != __stopper; i++) \
		fn(__list[i]); \
}

#define Free_list(...) FN_APPLY(void, free, __VA_ARGS__)

#define Sasprintf(write_to, ...) { \
	char *__tmp_str = write_to; \
	asprintf(&(write_to), __VA_ARGS); \
	free(__tmp_str); \
}