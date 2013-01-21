
#include "v8context.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>

#include "json_util.h"

using namespace v8;

static v8wrap_callback _go_callback = NULL;

Handle<Value> goCallback(const Arguments& args) {
  Locker v8Locker;
  void* fn = static_cast<void*>(External::Unwrap(args.Data()));

  HandleScope scope;

  // Convert arg array to json string
  std::string jsonArgs("[");
  for (int i=0; i < args.Length(); i++) {
    jsonArgs += JSONUtil::to_json(args[i]);
    if (i+1 != args.Length()) {
      jsonArgs += ",";
    }
  }
  jsonArgs += "]";

  Handle<Value> ret = Undefined();
  char* retv = _go_callback(fn, strdup(jsonArgs.c_str()));
  if (retv != NULL) {
    ret = JSONUtil::from_json(retv);
    free(retv);
  }

  return scope.Close(ret);
}

V8Context::V8Context(v8wrap_callback callback) {
  _go_callback = callback;

  Locker v8Locker;
  HandleScope scope;

  Handle<ObjectTemplate> global = ObjectTemplate::New();  
  Handle<Context> context = Context::New(NULL, global);
  
  context_ = Persistent<Context>::New(context);
}

V8Context::~V8Context() {
	context_.Dispose();
}

// Executes a given string javascript within this context. 
char* V8Context::execute(char* source) {
  Locker v8Locker;

  HandleScope scope;
  TryCatch try_catch;

  Context::Scope context_scope(context_);

  err("");
  Handle<Script> script = Script::Compile(String::New(source), Undefined());
  if (script.IsEmpty()) {
    ThrowException(try_catch.Exception());
    err(parseV8Exception(try_catch));
    return NULL;
  }
  else {
    Handle<Value> result = script->Run();
    if (result.IsEmpty()) {
      ThrowException(try_catch.Exception());
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

char* V8Context::addFunc(char* name, void* fn) {
  Locker v8Locker;

  HandleScope scope;
  TryCatch try_catch;

  err("");

  Local<Object> global = context_->Global();
  Local<FunctionTemplate> func = makeStaticCallableFunc(goCallback, reinterpret_cast<void *>(fn));

  // Requires the scope to be set to this context so v8 knows where to add the function to.
  Context::Scope context_scope(context_);
  global->Set(static_cast<Handle<String> >(String::New(name)), func->GetFunction(), ReadOnly);

  return strdup("");
}

Local<FunctionTemplate> V8Context::makeStaticCallableFunc(InvocationCallback func, void* data) {
    HandleScope scope;
    Local<FunctionTemplate> funcTemplate = FunctionTemplate::New(func, ptrToExternal(data));
    return scope.Close(funcTemplate);
}
 
Local<External> V8Context::ptrToExternal(void* ptr) {
    HandleScope scope;
    return scope.Close(External::New(ptr));
}

std::string V8Context::parseV8Exception(TryCatch& try_catch) {
  Handle<Message> message = try_catch.Message();
  String::Utf8Value exception(try_catch.Exception());

  std::stringstream ss;
  if (message.IsEmpty()) {
    ss << *exception << std::endl;
  }
  else {
    String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = *filename;
    int linenum = message->GetLineNumber();
    ss
      << filename_string
      << ":" << linenum
      << ": " << *exception << std::endl;
    String::Utf8Value sourceline(message->GetSourceLine());
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
    String::Utf8Value stack_trace(try_catch.StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = *stack_trace;
      ss << stack_trace_string << std::endl;
    }
  }
  return ss.str();
}
