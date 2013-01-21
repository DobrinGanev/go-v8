#ifndef _V8WRAP_H_
#define _V8WRAP_H_

#ifdef __cplusplus

/* Exported methods and type to c*/
extern "C" {
#endif

extern void v8_init(void *p);

extern void* v8_create_context();
extern void v8_release_context(void* ctx);

extern char* v8_error(void* ctx);

extern char* v8_execute(void* ctx, char* str);
extern char* v8_add_func(void *ctx, char* name, void* fn);

typedef char* (*v8wrap_callback)(void*, char*);

#ifdef __cplusplus
}
#endif

#endif /* !defined _V8WRAP_H_ */
