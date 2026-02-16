#include "zjson.h"
#include "jansson.h"

namespace json //using namespace will not work for the free functions
{
	Value NULL_VALUE()
	{
		return json_null();//using the "borrowed reference" constructor. It is fine here since for null json_incref is noop
	}

	Value::Value(const Value& rhs) : m_val(json_incref(rhs.m_val)) {}

	Value::Value(bool v) : m_val(json_boolean(v)) {}

	Value::Value(unsigned long long v) : m_val(json_integer(v)) {}

	Value::Value(long long v) : m_val(json_integer(v)) {}

	Value::Value(unsigned long v) : m_val(json_integer(v)) {}

	Value::Value(long v) : m_val(json_integer(v)) {}

	Value::Value(unsigned v) : m_val(json_integer(v)) {}

	Value::Value(int v) : m_val(json_integer(v)) {}

	Value::Value(unsigned short v) : m_val(json_integer(v)) {}

	Value::Value(short v) : m_val(json_integer(v)) {}

	Value::Value(unsigned char v) : m_val(json_integer(v)) {}

	Value::Value(double v) : m_val(json_real(v)) {}

	Value::Value(const char* v) : m_val(json_string_nocheck(v)) {}

	Value::Value(const std::string& v) : m_val(json_string_nocheck(v.c_str())) {}

	Value::Value(json_t* v) : m_val(json_incref(v)) {}//Internal for borrowed references (hence the ++ref).

	Value::~Value()
	{
		json_decref(m_val);
	}

	void Value::reset(Value&& rhs)
	{
		if (m_val == rhs.m_val)
			return;
		json_decref(m_val);
		m_val = rhs.m_val;
		rhs.m_val = NULL;
	}

	void Value::reset(const Value& rhs)
	{
		if (m_val == rhs.m_val)
			return;
		json_decref(m_val);
		m_val = json_incref(rhs.m_val);
	}

	Value Value::deepCopy() const
	{
		Value ret;
		ret.m_val = json_deep_copy(m_val);
		return ret;
	}

	bool Value::isNull() const
	{
		return json_is_null(m_val);
	}

	bool Value::isBool() const
	{
		return json_is_boolean(m_val);
	}

	bool Value::isInt() const
	{
		return json_is_integer(m_val);
	}

	bool Value::isFloat() const
	{
		return json_is_real(m_val);
	}

	bool Value::isString() const
	{
		return json_is_string(m_val);
	}

	bool Value::isArray() const
	{
		return json_is_array(m_val);
	}

	bool Value::isObject() const
	{
		return json_is_object(m_val);
	}
	//deep compare
	bool Value::operator==(const Value& rhs) const
	{
		return (!m_val && !rhs.m_val) || 0 != json_equal(m_val, rhs.m_val);
	}

	bool Value::asBool() const
	{
		if (!json_is_boolean(m_val))
			onCastErr("boolean");
		return json_typeof(m_val) == JSON_TRUE;
	}

	bool Value::asBoolSafe() const
	{
		if (json_is_true(m_val))
			return true;
		else
			return false;
	}

	long long Value::asInt() const
	{
		if (!json_is_integer(m_val))
			onCastErr("integer");
		return json_integer_value(m_val);
	}

	long long Value::asIntNum() const
	{
		if (json_is_integer(m_val))
			return json_integer_value(m_val);
		else if (json_is_real(m_val))
			return long long(json_real_value(m_val) + 0.5);
		else
			return onCastErr("integer_num"), 0;
	}

	long long Value::asIntSafe(long long def /*= 0ll*/) const
	{
		if (!json_is_integer(m_val))
			return def;
		return json_integer_value(m_val);
	}

	double Value::asFloat() const
	{
		if (!json_is_real(m_val))
			onCastErr("float");
		return json_real_value(m_val);
	}

	double Value::asFloatNum() const
	{
		if (json_is_integer(m_val))
			return double(json_integer_value(m_val));
		else if (json_is_real(m_val))
			return json_real_value(m_val);
		else
			return onCastErr("float_num"), 0;
	}

	double Value::asFloatSafe(double def /*= 0.*/) const
	{
		if (!json_is_real(m_val))
			return def;;
		return json_real_value(m_val);
	}

	const char* Value::c_str() const
	{
		if (!json_is_string(m_val))
			onCastErr("string");
		return json_string_value(m_val);
	}

	const char* Value::c_strSafe(const char* def/*= ""*/) const
	{
		if (!json_is_string(m_val))
			return def;
		return json_string_value(m_val);
	}

	std::string Value::asStringSafe(std::string def /*= {}*/) const
	{//copy of the c_str, so that def can be moved on return
		if (!json_is_string(m_val))
			return def;
		return json_string_value(m_val);
	}

	const Array Value::asArray() const
	{
		if (!json_is_array(m_val))
			onCastErr("array");
		return reinterpret_cast<const Array&>(*this);
	}

	const Object Value::asObject() const
	{
		if (!json_is_object(m_val))
			onCastErr("object");
		return reinterpret_cast<const Object&>(*this);
	}

	Object Value::asObjectSafe() const
	{
		if (!json_is_object(m_val))
			return {};
		return reinterpret_cast<const Object&>(*this);
	}

	Array Value::asArraySafe() const
	{
		if (!json_is_array(m_val))
			return {};
		return static_cast<const Array&>(*this);
	}

	bool Value::isEmptyValue() const
	{
		if (!isEmpty())
		{
			switch (m_val->type)
			{
			case JSON_OBJECT: return asObject().empty();
			case JSON_ARRAY: return asArray().empty();
			case JSON_STRING: return 0 == *c_str();
			case JSON_INTEGER: return 0 == asInt();
			case JSON_REAL: return 0. == asFloat();
			case JSON_TRUE: return false;
			case JSON_FALSE: return true;
			case JSON_NULL: return true;
			}
		}
		return true;
	}

	void Value::dump(std::ostream& os) const
	{
		if (isEmpty())
			os << "#absent#";
		else
		{
			os << setIgnoreOutErrs;
			switch (m_val->type)
			{
			case JSON_OBJECT: os << asObject(); break;
			case JSON_ARRAY: os << asArray(); break;
			case JSON_STRING: os << '"' << asString() << '"'; break;
			case JSON_INTEGER: os << asInt(); break;
			case JSON_REAL: os << asFloat(); break;
			case JSON_TRUE: os << "true"; break;
			case JSON_FALSE: os << "false"; break;
			case JSON_NULL: os << "null"; break;
			}
		}
	}

	namespace
	{
		std::string cutStr(std::string s, size_t maxSz)
		{//Avoid cuts in the middle of a UTF char, e.g. when dumping to logs expecting UTF8 strings.
			if (s.size() > maxSz)
			{//UTF8 multibytes are all negative (first bit 1), but only the first byte has the 2 major bits set
				if (3 < maxSz)
					maxSz -= 3;//for the "..." at the end
				else
					return std::string{ "..." }.substr(0, maxSz);
				static const uint8_t UTF8_START_BYTE{ 0b11000000 };//start of UTF8 char
				static const uint8_t UTF8_MULTY_BYTE{ 0b10000000 };//start of UTF8 char
				for (; 0 < maxSz && (UTF8_MULTY_BYTE & s[maxSz]); --maxSz)
					if (UTF8_START_BYTE == (UTF8_START_BYTE & s[maxSz]))
						break;//we are at the start of the UTF8 char
				s.resize(maxSz);
				s += "...";
			}
			return s;
		}
	}
	std::string Value::dump(size_t maxChars /*= 0*/) const
	{
		std::ostringstream os;
		dump(os);
		return 0 == maxChars
			? os.str()
			: cutStr(os.str(), maxChars);
	}

	std::string Value::type2String() const
	{
		if (NULL == m_val)
			return "empty";
		switch (json_typeof(m_val))
		{
		case JSON_OBJECT: return "object";
		case JSON_ARRAY: return "array";
		case JSON_STRING: return "string";
		case JSON_INTEGER: return "integer";
		case JSON_REAL: return "float";
		case JSON_TRUE: return "boolean";
		case JSON_FALSE: return "boolean";
		case JSON_NULL: return "null";
		default: return "unknown";
		}
	}
	void Value::cow()
	{//read as CopyOnWrite (no domestic animals abused here!)
		if (1 < m_val->refcount)
		{
			json_t* valCopy = json_copy(m_val);//shallow copy will do (if any elem of the orig obj is modified later it will copy itself too)
			json_decref(m_val);
			m_val = valCopy;
		}
	}

	Array::Array()
	{
		m_val = json_array();
	}

	Array::Array(std::initializer_list<Value> ilRhs)
		: Array()
	{
		//Fix the confusing case (same as std::vector<int> a{10}; VS std::vector<int> a(10);)
		// where this is called instead of the copy constructor when construct syntax with curly braces is used.
		//	Array a{array2};
		//will NOT do copy/move as expected - as Array a(array2); would.
		//If you truly want array with one array element use push_back()
		if(1 == ilRhs.size() && ilRhs.begin()->isArray())
			m_val = json_incref(ilRhs.begin()->m_val);
		else
			for (auto& el : ilRhs)
				push_back(el);
	}

	size_t Array::size() const
	{
		return json_array_size(m_val);
	}

	const Value Array::getAt(size_t idx) const
	{
		if (idx >= size())
			throw Exc{ "Invalid index " + std::to_string(idx) + " for JSON array with size " + std::to_string(size()) };
		return json_array_get(m_val, idx);
	}

	Array::ValueAssign Array::getAt(size_t idx)
	{
		return ValueAssign{ json_array_get(m_val, idx), idx, *this };
	}

	void Array::setAt(size_t idx, const Value& val)
	{
		cow();
		if (0 != json_array_set(m_val, idx, val.m_val))
		{
			std::ostringstream os;
			os << "JSON error: array element can not be set. Idx: " << idx << ". New value type: " << val.type2String();
			throw Exc(os.str());
		}
	}

	void Array::push_back(const Value& val)
	{
		cow();
		if (0 != json_array_append(m_val, val.m_val))
		{
			std::ostringstream os;
			os << "JSON error: array element can not be appended. Size: " << size() << ". New value type: " << val.type2String();
			throw Exc(os.str());
		}
	}

	void Array::insert(size_t idx, const Value& val)
	{
		cow();
		if (0 != json_array_insert(m_val, idx, val.m_val))
		{
			std::ostringstream os;
			os << "JSON error: array element can not be inserted. idx: " << idx << ". Size: " << size() << ". New value type: " << val.type2String();
			throw Exc(os.str());
		}
	}

	void Array::erase(size_t idx)
	{
		cow(); json_array_remove(m_val, idx);
	}

	void Array::clear()
	{
		cow();
		json_array_clear(m_val);
	}

	Array::const_iterator::reference Array::const_iterator::operator*() const
	{
		m_curVal = m_pMyArr->getAt(m_idx);
		return m_curVal;
	}

	Array::iterator::reference Array::iterator::operator*() const
	{
		static_cast<Value&>(m_curVal) = const_cast<const Array*>(m_pMyArr)->getAt(idx());
		return m_curVal;
	}

	Object::Object()
	{
		m_val = json_object();
	}

	Object::Object(std::initializer_list<std::pair<const char*, Value>> ilRhs) : Object()
	{
		for (auto& el : ilRhs)
			setAt(el.first, el.second);
	}

	size_t Object::size() const
	{
		return json_object_size(m_val);
	}

	bool Object::hasField(const char* key) const
	{
		return NULL != json_object_iter_at(m_val, key);
	}

	void Object::clear()
	{
		cow();
		json_object_clear(m_val);
	}

	const Value Object::operator[](const char* key) const
	{//return const to prevent accidntal assignemt to temporary
		return { json_object_get(m_val, key) };
	}

	Object::ValueAssign Object::operator[](const char* key)
	{
		return ValueAssign{ json_object_get(m_val, key), key, *this };
	}

	void Object::setAt(const char* key, const Value& val)
	{
		cow();
		if (0 != json_object_set_nocheck(m_val, key, val.m_val))
		{
			std::ostringstream os;
			os << "JSON error: object element can not be set. Key: " << key << ". New value type: " << val.type2String();
			throw Exc(os.str());
		}
	}

	bool Object::erase(const char* key)
	{
		cow();
		return 0 == json_object_del(m_val, key);
	}


	Object::const_iterator::const_iterator(const Object& jObj, bool atEnd)
		: m_pMyObj(&jObj)
		, m_iter(atEnd ? nullptr : json_object_iter(rawObj()))
	{}

	Object::const_iterator& Object::const_iterator::operator++()
	{
		m_iter = json_object_iter_next(rawObj(), m_iter);
		return *this;
	}

	Object::const_iterator::reference Object::const_iterator::operator*() const
	{
		m_curVal.first = json_object_iter_key(m_iter);
		const_cast<Value&>(m_curVal.second) = json_object_iter_value(m_iter);
		return m_curVal;
	}

	const int osFormatIdx()
	{
		static const auto ret{ std::ios_base::xalloc() };
		return ret;
	};
	std::ostream& operator<<(std::ostream& os, const setOStreamIdent& ident)
	{
		os.iword(osFormatIdx()) = ident.m_ident;
		return os;
	}

	const int osSortedIdx()
	{
		static const auto ret{ std::ios_base::xalloc() };
		return ret;
	};
	std::ostream& setOStreamSorted(std::ostream& os)
	{
		os.iword(osSortedIdx()) = 1;
		return os;
	}

	const int osIgnoreErrsIdx()
	{
		static const auto ret{ std::ios_base::xalloc() };
		return ret;
	};
	std::ostream& setIgnoreOutErrs(std::ostream& os)
	{
		os.iword(osIgnoreErrsIdx()) = 1;
		return os;
	}

	namespace
	{
		int ostream_callback(const char* buffer, size_t sz, void* data)
		{
			std::ostream& os = *(std::ostream*)data;
			os.write(buffer, sz);
			return 0;
		}

		std::ostream& operator<<(std::ostream& os, json_t* v)
		{
			if (!os)
				throw Exc("JSON error: Output stream is not in a good state. Check permissions.");
			const int ident = os.iword(osFormatIdx());
			const size_t sortedFlag = (1 == os.iword(osSortedIdx()) ? JSON_SORT_KEYS : 0);
			int janssonRes = -1;
			if (0 == ident)
				janssonRes = json_dump_callback(v, &ostream_callback, &os, JSON_COMPACT | sortedFlag);
			else
				janssonRes = json_dump_callback(v, &ostream_callback, &os, JSON_INDENT(ident) | sortedFlag);

			if (0 != janssonRes && 1 != os.iword(osIgnoreErrsIdx()))
				throw Exc("JSON serialization failed (invalid UTF8 string?)");
			return os;
		}
	}
	std::ostream& operator<<(std::ostream& os, const Array& arrVal)
	{
		return os << arrVal.m_val;
	}

	std::ostream& operator<<(std::ostream& os, const Object& objVal)
	{
		return os << objVal.m_val;
	}

	namespace
	{
		size_t istream_callback(void* buffer, size_t buflen, void* data)
		{
			std::istream& is = *(std::istream*)data;
			if (!is.good())
			{
				if (is.eof())
					return 0;
				else
					return (size_t)-1;
			}
			is.read((char*)buffer, buflen);
			return size_t(is.gcount());
		}

		std::istream& operator>>(std::istream& is, json_t*& v)
		{
			if (!is)
				throw Exc("JSON error: Input stream is not in a good state. Check existence and permissions.");
			json_decref(v);//first clean any previous values (if any)
			json_error_t err;
			v = json_load_callback(istream_callback, &is, 0/*JSON_DISABLE_EOF_CHECK*/, &err);
			if (NULL == v)
			{
				std::ostringstream os;
				os << "JSON error: Deserialization failure (line "
					<< err.line << ", clm " << err.column << ", pos " << err.position << "). "
					<< err.text;
				throw Exc(os.str());
			}
			return is;
		}
	}

	std::istream& operator>>(std::istream& is, Array& arrVal)
	{
		is >> arrVal.m_val;
		if (!json_is_array(arrVal.m_val))
			throw Exc(std::string("JSON error: Deserialization failure: Expecting array but deserialized ") + arrVal.type2String());

		return is;
	}

	std::istream& operator>>(std::istream& is, Object& objVal)
	{
		is >> objVal.m_val;
		if (!json_is_object(objVal.m_val))
			throw Exc(std::string("JSON error: Deserialization failure: Expecting object but deserialized ") + objVal.type2String());

		return is;
	}

//Optimized (no extra mem copy) with boost, if can
// #include <boost/iostreams/device/array.hpp>
// #include <boost/iostreams/stream.hpp>
#if defined(BOOST_IOSTREAMS_ARRAY_HPP_INCLUDED)
	Object strToObject(const char* cStr, size_t sz)
	{//no extra mem copy
		boost::iostreams::stream< boost::iostreams::array_source> is(cStr, sz);
		Object jObj;
		(std::istream&)is >> jObj;
		return jObj;
	}

	Array strToArray(const char* cStr, size_t sz)
	{//no extra mem copy
		boost::iostreams::stream< boost::iostreams::array_source> is(cStr, sz);
		Array ret;
		(std::istream&)is >> ret;
		return ret;
	}
#else
	Object strToObject(const char* cStr, size_t sz)
	{
		std::stringstream is{ std::string{cStr, sz} };
		Object jObj;
		(std::istream&)is >> jObj;
		return jObj;
	}

	Array strToArray(const char* cStr, size_t sz)
	{
		std::stringstream is{ std::string{cStr, sz} };
		Array ret;
		(std::istream&)is >> ret;
		return ret;
	}
#endif
}
