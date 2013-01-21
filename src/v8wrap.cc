#include "v8wrap.h"

#include <cstring>

#include "json_util.h"
#include "v8context.h"

static v8wrap_callback _go_callback = NULL;

void
v8_init(void *p) {
  _go_callback = (v8wrap_callback) p;
}

void*
v8_create_context() {
  return (void*) new V8Context(_go_callback); 
}

void
v8_release_context(void* ctx) {
  delete static_cast<V8Context *>(ctx);
}

char*
v8_error(void* ctx) {
  V8Context *context = static_cast<V8Context *>(ctx);
  return strdup(context->err().c_str());
}

char*
v8_execute(void *ctx, char* source) {
  V8Context *context = static_cast<V8Context *>(ctx);
  return context->execute(source);
}

char*
v8_add_func(void *ctx, char* name, void* fn) {
  V8Context *context = static_cast<V8Context *>(ctx);
  return context->addFunc(name, fn);
}
