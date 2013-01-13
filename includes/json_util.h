#ifndef _JSON_UTIL_H_
#define _JSON_UTIL_H_

#include <v8.h>
#include <string>

namespace JSONUtil {
	std::string to_json(v8::Handle<v8::Value> value);

	v8::Handle<v8::Value> from_json(std::string str);
}

#endif /* !defined _JSON_UTIL_H_ */
