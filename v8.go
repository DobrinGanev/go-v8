package v8

/*
#include <stdlib.h>
#include "includes/v8wrap.h"

extern char* goV8WrapCallback(void* fn, char* args);

static void
v8_callback_init() {
	v8_init((void*)goV8WrapCallback);
}

*/
// #cgo LDFLAGS: -Lbin -lv8wrap -lstdc++
import "C"
import (
	"bytes"
	"encoding/json"
	"errors"
	"runtime"
	"unsafe"
)

type CallbackFunc func(...interface{}) interface{}

//export goV8WrapCallback
func goV8WrapCallback(p unsafe.Pointer, a *C.char) *C.char {
	f := *(*CallbackFunc)(unsafe.Pointer(&p))

	var argv []interface{}
	json.Unmarshal([]byte(C.GoString(a)), &argv)

	ret := f(argv...)
	if ret != nil {
		b, _ := json.Marshal(ret)
		return C.CString(string(b))
	}
	return nil
}

func init() {
	C.v8_callback_init()
}

type V8Context struct {
	v8context unsafe.Pointer
}

func NewContext() *V8Context {
	v := &V8Context{
		C.v8_create_context(),
	}
	runtime.SetFinalizer(v, func(p *V8Context) {
		C.v8_release_context(p.v8context)
	})
	return v
}

func (v *V8Context) Eval(in string) (res interface{}, err error) {
	ptr := C.CString(in)
	defer C.free(unsafe.Pointer(ptr))
	ret := C.v8_execute(v.v8context, ptr)
	if ret != nil {
		out := C.GoString(ret)
		if out != "" {
			C.free(unsafe.Pointer(ret))
			var buf bytes.Buffer
			buf.Write([]byte(out))
			dec := json.NewDecoder(&buf)
			err = dec.Decode(&res)
			return
		}
		return nil, nil
	}
	ret = C.v8_error(v.v8context)
	out := C.GoString(ret)
	C.free(unsafe.Pointer(ret))
	return nil, errors.New(out)
}

func (v *V8Context) AddFunc(name string, f CallbackFunc) error {
	cName := C.CString(name)
	cFunc := *(*unsafe.Pointer)(unsafe.Pointer(&f))

	defer C.free(unsafe.Pointer(cName))

	C.v8_add_func(v.v8context, cName, cFunc)

	return nil
}
