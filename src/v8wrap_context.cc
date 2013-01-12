
#include "v8wrap_context.h"
#include <cstdlib>
#include "json_util.h"


//static v8::Handle<v8::ObjectTemplate> global;
static v8wrap_callback _go_callback = NULL;

v8::Handle<v8::Value>
_go_call(const v8::Arguments& args) {
  v8::Locker v8Locker;

  uint32_t id = args[0]->ToUint32()->Value();
  v8::String::Utf8Value name(args[1]);
  v8::String::Utf8Value argv(args[2]);
  
  v8::HandleScope scope;
  v8::Handle<v8::Value> ret = v8::Undefined();
  char* retv = _go_callback(id, *name, *argv);
  if (retv != NULL) {
    ret = from_json(retv);
    free(retv);
  }
  return ret;
}

V8Context::V8Context(v8wrap_callback callback) {
  _go_callback = callback;

  v8::Locker v8Locker;
  v8::HandleScope scope;

  v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
  global->Set(v8::String::New("_go_call"), v8::FunctionTemplate::New(_go_call));
  
  v8::Handle<v8::Context> context = v8::Context::New(NULL, global);
  context_ = v8::Persistent<v8::Context>::New(context);
}

V8Context::~V8Context() {
	context_.Dispose();
};