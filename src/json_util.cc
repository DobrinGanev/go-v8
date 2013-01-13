#include "json_util.h"

std::string JSONUtil::to_json(v8::Handle<v8::Value> value) {
  v8::HandleScope scope;
  v8::TryCatch try_catch;
  v8::Handle<v8::Object> json = v8::Handle<v8::Object>::Cast(
    v8::Context::GetCurrent()->Global()->Get(v8::String::New("JSON")));
  v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(
    json->GetRealNamedProperty(v8::String::New("stringify")));
  v8::Handle<v8::Value> args[1];
  args[0] = value;
  v8::String::Utf8Value ret(
    func->Call(v8::Context::GetCurrent()->Global(), 1, args)->ToString());
  return (char*) *ret;
}

v8::Handle<v8::Value> JSONUtil::from_json(std::string str) {
  v8::HandleScope scope;
  v8::TryCatch try_catch;
  v8::Handle<v8::Object> json = v8::Handle<v8::Object>::Cast(
    v8::Context::GetCurrent()->Global()->Get(v8::String::New("JSON")));
  v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(
    json->GetRealNamedProperty(v8::String::New("parse")));
  v8::Handle<v8::Value> args[1];
  args[0] = v8::String::New(str.c_str());
  return func->Call(v8::Context::GetCurrent()->Global(), 1, args);
}
