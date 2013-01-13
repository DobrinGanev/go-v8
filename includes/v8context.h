#ifndef _V8CONTEXT_H_
#define _V8CONTEXT_H_

#include <v8.h>
#include <string>

#include "v8wrap.h"

class V8Context {
public:
  V8Context(v8wrap_callback callback);
  virtual ~V8Context();

  char* execute(char* source);
  std::string parseV8Exception(v8::TryCatch& try_catch);

  v8::Handle<v8::Context> context() { return context_; };

  // error getter & setter
  std::string err() const { return err_; };
  void err(const std::string err) { this->err_ = err; }

private:
  v8::Persistent<v8::Context> context_;
  std::string err_;
};

#endif /* !defined _V8CONTEXT_H_*/  