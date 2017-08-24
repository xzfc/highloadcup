struct $HANDLER_NAME : rapidjson::BaseReaderHandler<> {
	All &all;
	int state = 0;

	uint32_t id;
	$LOCAL_VAR;
	uint8_t have;

	$HANDLER_NAME(All &all) : all{all} { }

	bool StartObject() {
		switch (state) {
		case 0: state = 1; return true;
		case 3: state = 4; have = 0; return true;
		default: $err; return false;
		}
	}
	bool EndObject(size_t) {
		switch (state) {
		case 2: state = 0; return true;
		case 4:
			if (have != (1<<$KEY_NUMBER) - 1) { $err; return false; }
			state = 3;
			$ADD_NEW;
			return true;
		default: $err; return false;
		}
	}
	bool StartArray() {
		if (state != 2)
			{ $err; return false; }
		state = 3;
		return true;
	}
	bool EndArray(size_t) {
		if (state != 3)
			{ $err; return false; }
		state = 2;
		return true;
	}
	bool Key(const char* str, size_t length, bool) {
		switch (state) {
		case 1:
			if (strncmp(str, $TOPLEVEL_FIELD, length))
				{ $err; return false; }
			state = 2;
			return true;
		case 4:
			$KEY_HANDLER;
		default:
			$err; return false;
		}
	}
	bool Int(int val) {
		switch (state) {
		$INT_HANDLER;
		default: $err; return false;
		}
		state = 4;
		return true;
	}
	bool Uint(unsigned val) {
		switch (state) {
		$INT_HANDLER;
		default: $err; return false;
		}
		state = 4;
		return true;
	}
	bool String(const char* str, size_t length, bool) {
		(void)str;(void)length;
		switch (state) {
		$STRING_HANDLER;
		default: $err; return false;
		}
		state = 4; return true;
	}
};

#undef $HANDLER_NAME
#undef $LOCAL_VAR
#undef $TOPLEVEL_FIELD
#undef $ADD_NEW
#undef $KEY_NUMBER
#undef $KEY_HANDLER
#undef $INT_HANDLER
#undef $STRING_HANDLER
