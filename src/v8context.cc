
#include "v8context.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

#include "json_util.h"

static v8wrap_callback _go_callback = NULL;

v8::Handle<v8::Value> _go_call(const v8::Arguments& args) {
  v8::Locker v8Locker;

  uint32_t id = args[0]->ToUint32()->Value();
  v8::String::Utf8Value name(args[1]);
  v8::String::Utf8Value argv(args[2]);
  
  v8::HandleScope scope;
  v8::Handle<v8::Value> ret = v8::Undefined();
  char* retv = _go_callback(id, *name, *argv);
  if (retv != NULL) {
    ret = JSONUtil::from_json(retv);
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
}

// Executes a given string javascript within this context. 
char* V8Context::execute(char* source) {
  v8::Locker v8Locker;

  v8::HandleScope scope;
  v8::TryCatch try_catch;

  v8::Context::Scope context_scope(context_);

  err("");
  v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New(source), v8::Undefined());
  if (script.IsEmpty()) {
    v8::ThrowException(try_catch.Exception());
    err(parseV8Exception(try_catch));
    return NULL;
  }
  else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      v8::ThrowException(try_catch.Exception());
      err(parseV8Exception(try_catch));
      return NULL;
    }
    else if (result->IsFunction() || result->IsUndefined()) {
      return strdup("");
    }
    else {
      return strdup(JSONUtil::to_json(result).c_str());
    }
  }
}

std::string V8Context::parseV8Exception(v8::TryCatch& try_catch) {
  v8::Handle<v8::Message> message = try_catch.Message();
  v8::String::Utf8Value exception(try_catch.Exception());

  std::stringstream ss;
  if (message.IsEmpty()) {
    ss << *exception << std::endl;
  }
  else {
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
