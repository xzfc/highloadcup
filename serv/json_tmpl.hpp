template <>
struct JsonHandler<$DATA> : rapidjson::BaseReaderHandler<> {
	int state = 0;
	bool single;

	$DATA data;
	$DATA::Mask mask;

	JsonHandler(bool single)
		: state(single ? 3 : 0), single(single)
		{}

	bool Default() { $err; }

	bool StartObject() {
		switch (state) {
		case 0: state = 1; return true;
		case 3: state = 4; mask.reset(); return true;
		default: $err;
		}
	}
	bool EndObject(size_t) {
		switch (state) {
		case 2: state = 0; return true;
		case 4:
			if (single) {
				state = 3;
				return true;
			} else {
				if (!mask.is_full()) $err;
				state = 3;
				::add(data);
				return true;
			}
		default: $err;
		}
	}
	bool StartArray() {
		if (state != 2) $err;
		state = 3;
		return true;
	}
	bool EndArray(size_t) {
		if (state != 3) $err;
		if (single) $err;
		state = 2;
		return true;
	}
	bool Key(const char* str, size_t length, bool) {
		switch (state) {
		case 1:
			if (strncmp(str, $TOPLEVEL_FIELD, length))
				$err;
			state = 2;
			return true;
		case 4:
			$KEY_HANDLER;
		default:
			$err;
		}
	}
	bool Int(int val) {
		switch (state) {
		$INT_HANDLER;
		default: $err;
		}
		state = 4;
		return true;
	}
	bool Uint(unsigned val) {
		switch (state) {
		$INT_HANDLER;
		default: $err;
		}
		state = 4;
		return true;
	}
	bool String(const char* str, size_t length, bool) {
		(void)str;(void)length;
		switch (state) {
		$STRING_HANDLER;
		default: $err;
		}
		state = 4;
		return true;
	}
};

#undef $DATA
#undef $TOPLEVEL_FIELD
#undef $ADD_NEW
#undef $KEY_HANDLER
#undef $INT_HANDLER
#undef $STRING_HANDLER
