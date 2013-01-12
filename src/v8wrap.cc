#include <v8.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "v8wrap.h"
#include "json_util.h"
#include "v8wrap_context.h"

extern "C" {

static v8wrap_callback _go_callback = NULL;

void
v8_init(void *p) {
  _go_callback = (v8wrap_callback) p;
}

void*
v8_create() {
  return (void*) new V8Context(_go_callback); 
}

void
v8_release(void* ctx) {
  delete static_cast<V8Context *>(ctx);
}

char*
v8_error(void* ctx) {
  V8Context *context = static_cast<V8Context *>(ctx);
  return strdup(context->err().c_str());
}

static std::string
report_exception(v8::TryCatch& try_catch) {
  v8::Handle<v8::Message> message = try_catch.Message();
  v8::String::Utf8Value exception(try_catch.Exception());
  std::stringstream ss;
  if (message.IsEmpty()) {
    ss << *exception << std::endl;
  } else {
    v8::String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = *filename;
    int linenum = message->GetLineNumber();
    ss
      << filename_string
      << ":" << linenum
      << ": " << *exception << std::endl;
    v8::String::Utf8Value sourceline(message->GetSourceLine());
    ss << *sourceline << std::endl;
    int start = message->GetStartColumn();
    for (int n = 0; n < start; n++) {
      ss << " ";
    }
    int end = message->GetEndColumn();
    for (int n = start; n < end; n++) {
      ss << "^";
    }
    ss << std::endl;
    v8::String::Utf8Value stack_trace(try_catch.StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = *stack_trace;
      ss << stack_trace_string << std::endl;
    }
  }
  return ss.str();
}

char*
v8_execute(void *ctx, char* source) {
  v8::Locker v8Locker;

  V8Context *context = static_cast<V8Context *>(ctx);

  v8::HandleScope scope;
  v8::TryCatch try_catch;

  v8::Context::Scope context_scope(context->context());

  context->err("");
  v8::Handle<v8::Script> script
    = v8::Script::Compile(v8::String::New(source), v8::Undefined());
  if (script.IsEmpty()) {
    v8::ThrowException(try_catch.Exception());
    context->err(report_exception(try_catch));
    return NULL;
  } else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      v8::ThrowException(try_catch.Exception());
      context->err(report_exception(try_catch));
      return NULL;
    }
    else if (result->IsFunction() || result->IsUndefined()) {
      return strdup("");
    } else {
      return strdup(to_json(result).c_str());
    }
  }
}

}

// vim:set et sw=2 ts=2 ai:
