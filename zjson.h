#pragma once
#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>
#include <utility>

#if !defined(ZJSON_BUILD_SHARED)
 #define ZJSON_EXP_IMP
 #pragma message("ZJSON: Building static library.")
#else
 #define ZJSON_EXP_IMP __declspec(dllexport)
 #pragma message("ZJSON: Building shared library.")
#endif

struct json_t;

namespace json
{
	class ZJSON_EXP_IMP Exc : public std::runtime_error
	{
	public:
		Exc(const char* msg) : std::runtime_error(msg) {}
		Exc(const std::string& msg) : std::runtime_error(msg) {}
	};

	class Array;
	class Object;
	//reference counted json value. Not thread safe!
	class ZJSON_EXP_IMP Value
	{
	public:
		Value() : m_val(nullptr) {}
		Value(Value&& rhs) : m_val(rhs.m_val) { rhs.m_val = nullptr; }
		//Lazy copy! CoW (CopyOnWrite) implemented and called in Array and Object modifiers (Value does not have modifiers that preserve json_t value)
		Value(const Value& rhs);
		Value(bool v);
		Value(uint64_t v);
		Value(int64_t v);
		Value(unsigned long v);
		Value(long v);
		Value(unsigned v);
		Value(int v);
		Value(unsigned short v);
		Value(short v);
		Value(unsigned char v);
		Value(double v);
		Value(const char* v);
		Value(const std::string& v);
		Value(json_t* v);//Internal. Should only be used for borrowed references.
		~Value();

		Value& operator=(Value&& rhs) { reset(std::move(rhs)); return *this; }
		Value& operator=(const Value& rhs) { reset(rhs); return *this; }
		void reset(Value&& rhs);//public for backwards compatibility with existing code (TBD). New code should use direct assignment
		void reset(const Value& rhs);
		Value deepCopy() const;
		void swap(Value& rhs) { std::swap(m_val, rhs.m_val); }

		bool isEmpty() const { return !m_val; }
		bool isEmptyValue() const;
		bool isNull() const;
		bool isBool() const;
		bool isInt() const;
		bool isFloat() const;
		bool isString() const;
		bool isArray() const;
		bool isObject() const;
		//deep compare
		bool operator==(const Value& rhs) const;
		bool operator!=(const Value& rhs) const { return !(*this == rhs); }
		bool isSameVal(const Value& rhs) const { return m_val == rhs.m_val; }

		bool asBool() const;
		bool asBoolSafe() const;
		int64_t asInt() const;
		int64_t asIntNum() const;//can be int or float(rounded)
		int64_t asIntSafe(int64_t def = 0ll) const;
		template<typename IntT> IntT asIntT() const;
		template<typename UIntT> UIntT asUIntT() const;
		template<typename IntT> IntT asIntTSafe(IntT def) const;
		template<typename UIntT> UIntT asUIntTSafe(UIntT def) const;
		uint64_t asUInt64() const { return uint64_t(asInt()); }
		uint64_t asUInt64Safe(uint32_t def = 0) const { return uint64_t(asIntSafe(def)); }
		int32_t asInt32() const { return asIntT<int32_t>(); }
		int32_t asInt32Safe(int32_t def = 0) const { return asIntTSafe<int32_t>(def); }
		uint32_t asUInt32() const { return asUIntT<uint32_t>(); }
		uint32_t asUInt32Safe(uint32_t def = 0) const { return asUIntTSafe<uint32_t>(def); }
		int16_t asInt16() const { return asIntT<int16_t>(); }
		int16_t asInt16Safe(int16_t def = 0) const { return asIntTSafe<int16_t>(def); }
		uint16_t asUInt16() const { return asUIntT<uint16_t>(); }
		uint16_t asUInt16Safe(uint16_t def = 0) const { return asUIntTSafe<uint16_t>(def); }
		int8_t asInt8() const { return asIntT<int8_t>(); }
		int8_t asInt8Safe(int8_t def = 0) const { return asIntTSafe<int8_t>(def); }
		uint8_t asUInt8() const { return asUIntT<uint8_t>(); }
		uint8_t asUInt8Safe(uint8_t def = 0) const { return asUIntTSafe<uint8_t>(def); }
		long asInt32l() const { return asIntT<long>(); }
		long asInt32lSafe(long def = 0) const { return asIntTSafe<long>(def); }
		double asFloat() const;
		double asFloatNum() const;//can be int or float
		double asFloatSafe(double def = 0.) const;
		const char* c_str() const;
		const char* c_strSafe(const char* def = "") const;
		std::string asString() const { return c_str(); }
		std::string asStringSafe(std::string def = {}) const;
		//don't return ref, e.g.: for (auto jSec : jObj["sects"].asArray()) - "sects" value is optimized away in Release instantly!
		const Array asArray() const;
		const Object asObject() const;
		Array asArraySafe() const;
		Object asObjectSafe() const;

		void dump(std::ostream&) const;//not just for logging!
		std::string dump(size_t maxChars = 0) const;

	protected:
		friend class Array; //for setAt()
		friend class Object; //for setAt()
		json_t* m_val;

		std::string type2String() const;
		void cow();//read as CopyOnWrite (no domestic animals abused here!)

	private:
		void onCastErr(const std::string& toType) const
		{
			throw Exc("JSON error: Wrong type: " + type2String() + ". Expected type: " + toType);
		}
	};
	ZJSON_EXP_IMP Value NULL_VALUE();
	template<typename IntT>
	inline IntT Value::asIntT() const
	{
		const auto ret{ asInt() };
		if (ret > std::numeric_limits<IntT>::max() || ret < std::numeric_limits<IntT>::lowest())
			throw Exc("JSON error: integer out of range. Value " + std::to_string(ret)
				+ " is outside the range of " + std::to_string(sizeof(IntT)) + "bytes int");
		return IntT(ret);
	}
	template<typename UIntT>
	inline UIntT Value::asUIntT() const
	{
		const auto ret{ uint64_t(asInt()) };
		if (ret > std::numeric_limits<UIntT>::max())
			throw Exc("JSON error: unsigned integer out of range. Value " + std::to_string(ret)
				+ " is outside the range of " + std::to_string(sizeof(UIntT)) + "bytes uint");
		return UIntT(ret);
	}
	template<typename IntT>
	inline IntT Value::asIntTSafe(IntT def) const
	{
		const auto ret{ asIntSafe(def) };
		if (ret > std::numeric_limits<IntT>::max() || ret < std::numeric_limits<IntT>::lowest())
			throw Exc("JSON error: integer out of range. Value " + std::to_string(ret)
				+ " is outside the range of " + std::to_string(sizeof(IntT)) + "bytes int");
		return IntT(ret);
	}
	template<typename UIntT>
	inline UIntT Value::asUIntTSafe(UIntT def) const
	{
		const auto ret{ uint64_t(asIntSafe(def)) };
		if (ret > std::numeric_limits<UIntT>::max())
			throw Exc("JSON error: unsigned integer out of range. Value " + std::to_string(ret)
				+ " is outside the range of " + std::to_string(sizeof(UIntT)) + "bytes uint");
		return UIntT(ret);
	}

// PointerLike_ concept requires C++20. Provide a best-effort fallback for older standards.
#if defined(__cpp_concepts) || (defined(__cplusplus) && __cplusplus >= 202002L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
	#define ZJSON_CPP20
	template <typename T>
	concept PointerLike_ = requires(T v) {
		{ *v };       // Can be dereferenced
		{ v.operator->() }; // Has arrow operator (or is a raw pointer)
	} || std::is_pointer_v<T>;
#else
// Pre-C++20 fallback: detect pointer-like types using SFINAE (works in C++14).
	#undef ZJSON_CPP20
	namespace detail {
		template<typename, typename = void>
		struct has_deref : std::false_type {};
		template<typename T>
		struct has_deref<T, decltype(void(*std::declval<T&>()))> : std::true_type {};
		template<typename, typename = void>
		struct has_arrow : std::false_type {};
		template<typename T>
		struct has_arrow<T, decltype(void(std::declval<T&>().operator->()))> : std::true_type {};
	}
#endif

	//No memeber variables in this class! reinterpret_cast in Value::asArray
	class ZJSON_EXP_IMP Array : public Value
	{
	public:
		Array();
		Array(Array&& rhs) : Value(std::move(rhs)) {}
		Array(const Array& rhs) : Value(rhs) {}
		Array(std::initializer_list< Value > ilRhs);
		Array& operator=(Array&& rhs) { Value::operator=(std::move(rhs)); return *this; }
		Array& operator=(const Array& rhs) { Value::operator=(rhs); return *this; }
		//convert std container of primitive types to Array
		template<class StdCont> static Array fromStdArray(const StdCont& cont)
		{
			Array ret;
			for (const auto& el : cont)
				ret.push_back(el);
			return ret;
		}
		//convert std container of (ptr to) types with json::Value serialize() to Array
		template<class StdCont>
		static Array fromStdArraySer(const StdCont & cont)
		{
			Array ret;
			for (const auto& el : cont)
			{
#if defined(ZJSON_CPP20)
				using ElementType = std::remove_cvref_t<decltype(el)>;
				if constexpr (PointerLike_<ElementType>)
#else
				using ElementType = std::remove_cv_t<std::remove_reference_t<decltype(el)>>;
				if(std::is_pointer<ElementType>::value
					|| (detail::has_deref<ElementType>::value && detail::has_arrow<ElementType>::value))
#endif
					ret.push_back(el->serialize());
				else
					ret.push_back(el.serialize());
			}
			return ret;
		}
		//create vector "like" container with elements constructible from primitive type. E.g.:
		// using StrVec=std::vector<std::string>; StrVec strVec = jArr.toStdArray<StrVec>(&json::Value::asString)
		template<class StdCont, typename ValueMemFn> StdCont toStdArray(ValueMemFn valueMemFn) const
		{
			StdCont ret;
			ret.reserve(size());
			for (size_t i = 0, iEnd = size(); i != iEnd; ++i)
				ret.emplace_back(((*this)[i].*valueMemFn)());
			return ret;
		}
		//for container with elements directly constructible from json Value
		template<class StdCont> StdCont toStdArray() const
		{
			StdCont ret;
			ret.reserve(size());
			for (size_t i = 0, iEnd = size(); i != iEnd; ++i)
				ret.emplace_back((*this)[i]);
			return ret;
		}
		//for container with elements directly constructible from json Object
		template<class StdCont> StdCont toStdObjArray() const //container elements must be constructible from json Object
		{
			StdCont ret;
			ret.reserve(size());
			for (size_t i = 0, iEnd = size(); i != iEnd; ++i)
				ret.emplace_back((*this)[i].asObject());
			return ret;
		}
		//for set "like" container with elements constructible from primitive type. Use:
		// using IntSet=std::set<int64_t>; IntSet iset = jArr.toStdSet<IntSet>(&json::Value::asInt)
		template<class StdCont, typename ValueMemFn> StdCont toStdSet(ValueMemFn valueMemFn) const //container elements constructible from primitive type
		{
			StdCont ret;
			for (size_t i = 0, iEnd = size(); i != iEnd; ++i)
				ret.emplace(((*this)[i].*valueMemFn)());
			return ret;
		}

		class iterator;
		//helper class used as non-const (assign-able) ref to element.
		class ZJSON_EXP_IMP ValueAssign : public Value
		{
		public:

			ValueAssign& operator=(const Value& rhs) { m_pMyArray->setAt(m_idx, Value::operator=(rhs)); return *this; }
			ValueAssign& operator=(const ValueAssign& rhs) { return operator=(static_cast<const Value&>(rhs)); }

			//!Here T a{b}; is not the same as T a; a = b;! We need the public copy const for std algorithms sake (e.g sort).
			ValueAssign(const ValueAssign&) = default;
			ValueAssign(ValueAssign&&) = default;
		private:
			Array* m_pMyArray;
			size_t m_idx;
			friend class Array;
			ValueAssign(json_t* v, size_t idx, Array& myArr) : Value(v), m_pMyArray(&myArr), m_idx(idx) {}
			friend class Array::iterator;//iterator uses this class not only as return value, but for its intenals too
			void set(json_t* v, size_t idx, Array& jArray) { Value::operator=({ v }); m_idx = idx; m_pMyArray = &jArray; }
		};

		bool empty() const { return isEmpty() ? true : 0 == size(); }
		size_t size() const;
		const Value operator[](size_t idx) const { return getAt(idx); }
		ValueAssign operator[](size_t idx) { return getAt(idx); }
		const Value getAt(size_t idx) const;
		ValueAssign getAt(size_t idx);
		const Value front() const { return getAt(0); }
		ValueAssign front() { return getAt(0); }
		const Value back() const { return getAt(size() - 1); }
		ValueAssign back() { return getAt(size() - 1); }
		void setAt(size_t idx, const Value& val);
		void push_back(const Value& val);
		void insert(size_t idx, const Value& val);
		void erase(size_t idx);
		void clear();

		class ZJSON_EXP_IMP const_iterator
		{
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = Value;
			using difference_type = int64_t;
			using pointer = const value_type*;
			using reference = const value_type&;

			const_iterator(const Array& jArr, size_t idx) : m_pMyArr(&jArr), m_idx(idx) {}
			const_iterator(const_iterator&&) = default;
			const_iterator(const const_iterator&) = default;
			const_iterator& operator=(const_iterator&&) = default;
			const_iterator& operator=(const const_iterator&) = default;
			bool operator==(const const_iterator& rhs) const { compatItr(rhs);  return m_idx == rhs.m_idx; }
			bool operator!=(const const_iterator& rhs) const { compatItr(rhs);  return m_idx != rhs.m_idx; }
			bool operator<(const const_iterator& rhs) const { compatItr(rhs);  return m_idx < rhs.m_idx; }
			bool operator<=(const const_iterator& rhs) const { compatItr(rhs);  return m_idx <= rhs.m_idx; }
			bool operator>=(const const_iterator& rhs) const { compatItr(rhs);  return m_idx >= rhs.m_idx; }
			bool operator>(const const_iterator& rhs) const { compatItr(rhs);  return m_idx > rhs.m_idx; }
			const_iterator& operator++() { ++m_idx; return *this; } // preincrement
			const_iterator operator++(int) { return { *m_pMyArr, m_idx++ }; }// postincrement
			const_iterator& operator+=(difference_type d) { (difference_type&)m_idx += d; return *this; }
			const_iterator operator+(difference_type d) const { const_iterator ret(*this); ret += d; return ret; }
			const_iterator& operator--() { --m_idx; return *this; } // predecrement
			const_iterator operator--(int) { return { *m_pMyArr, m_idx-- }; }// postdecrement
			const_iterator& operator-=(difference_type d) { (difference_type&)m_idx -= d; return *this; }
			const_iterator operator-(difference_type d) const { const_iterator ret(*this); ret -= d; return ret; }
			difference_type operator-(const const_iterator& rhs) const { return difference_type(m_idx) - difference_type(rhs.m_idx); }

			reference operator*() const;
			pointer operator->() const { return &operator*(); }

		private:
			const Array* m_pMyArr;
			size_t m_idx;
			mutable Value m_curVal;//helps to implemnent operator->
			void compatItr(const const_iterator& rhs) const { if (m_pMyArr != rhs.m_pMyArr) throw Exc("JSON error: Incompatible const iterators"); }
		};

		class ZJSON_EXP_IMP iterator
		{
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = ValueAssign;
			using difference_type = int64_t;
			using pointer = value_type*;
			using reference = value_type&;

			iterator(Array& jArr, size_t idx) : m_pMyArr(&jArr), m_curVal(nullptr, idx, jArr) {}
			iterator(const iterator& rhs) : m_pMyArr(rhs.m_pMyArr), m_curVal(nullptr, rhs.idx(), *rhs.m_pMyArr) {}
			iterator(iterator&& rhs) : iterator(const_cast<const iterator&>(rhs)) {}
			iterator& operator=(const iterator& rhs) { m_pMyArr = rhs.m_pMyArr; m_curVal.set(nullptr, rhs.idx(), *m_pMyArr); return *this; }
			iterator& operator=(iterator&& rhs) { return operator=(const_cast<const iterator&>(rhs)); }
			bool operator==(const iterator& rhs) const { compatItr(rhs);  return idx() == rhs.idx(); }
			bool operator!=(const iterator& rhs) const { compatItr(rhs);  return idx() != rhs.idx(); }
			bool operator<(const iterator& rhs) const { compatItr(rhs);  return idx() < rhs.idx(); }
			bool operator<=(const iterator& rhs) const { compatItr(rhs);  return idx() <= rhs.idx(); }
			bool operator>=(const iterator& rhs) const { compatItr(rhs);  return idx() >= rhs.idx(); }
			bool operator>(const iterator& rhs) const { compatItr(rhs);  return idx() > rhs.idx(); }
			iterator& operator++() { ++idx(); return *this; } // preincrement
			iterator operator++(int) { return { const_cast<Array&>(*m_pMyArr), idx()++ }; }// postincrement
			iterator& operator+=(difference_type d) { (difference_type&)idx() += d; return *this; }
			iterator operator+(difference_type d) const { iterator ret(*this); ret += d; return ret; }
			iterator& operator--() { --idx(); return *this; } // predecrement
			iterator operator--(int) { return { const_cast<Array&>(*m_pMyArr), idx()-- }; }// postdecrement
			iterator& operator-=(difference_type d) { (difference_type&)idx() -= d; return *this; }
			iterator operator-(difference_type d) const { iterator ret(*this); ret -= d; return ret; }
			difference_type operator-(const iterator& rhs) const { compatItr(rhs); return difference_type(idx()) - difference_type(rhs.idx()); }

			reference operator*() const;
			pointer operator->() const { return &operator*(); }

		private:
			Array* m_pMyArr;
			mutable value_type m_curVal;//helps to implement operator->
			size_t idx() const { return m_curVal.m_idx; }
			size_t& idx() { return m_curVal.m_idx; }
			void compatItr(const iterator& rhs) const { if(m_pMyArr != rhs.m_pMyArr) throw Exc("JSON error: Incompatible iterators"); }
		};
		const_iterator begin() const { return const_iterator(*this, 0); }
		const_iterator end() const { return const_iterator(*this, size()); }
		iterator begin() { return iterator(*this, 0); }
		iterator end() { return iterator(*this, size()); }

	private:
		friend ZJSON_EXP_IMP std::ostream& operator<<(std::ostream& os, const Array&);
		friend ZJSON_EXP_IMP std::istream& operator>>(std::istream& is, Array&);
	};

	//No memeber variables in this class!!! reinterpret_cast in Value::asObject
	class ZJSON_EXP_IMP Object : public Value
	{
	public:
		Object();
		Object(Object&& rhs) : Value(std::move(rhs)) {}
		Object(const Object& rhs) : Value(rhs) {}
		Object(std::initializer_list < std::pair< const char*, Value > > ilRhs);
		//Object( std::initializer_list < std::pair< const std::string&, Value > > ilRhs )  - ends with overload resolution was ambiguous in VS2019
		Object& operator=(Object&& rhs) { Value::operator=(std::move(rhs)); return *this; }
		Object& operator=(const Object& rhs) { Value::operator=(rhs); return *this; }

		//helper class used for direct assigning with operator[]. Derived from Value since it is used as return value (as well as assignment).
		//Do NOT use on its own (it has a pointer to the field name, which may become invliad out of the [] call scope)!
		//E.g. auto v = jObj[ std::string( "fld" ) ]; v = "val"; is UB!
		//Also Value &a = jObj[ "fld" ]; v = "val"; does not modify jObj!
		//jObj[ std::string( "fld" ) ] = "val"; works as expected (i.e. same as jObj[ "fld" ] = "val";)
		class ZJSON_EXP_IMP ValueAssign : public Value
		{
		public:
			void operator=(const Value& rhs) { m_pMyObj->setAt(m_key, Value::operator=(rhs)); }
			void operator=(const ValueAssign& rhs) { m_pMyObj->setAt(m_key, Value::operator=(rhs)); }

		private:
			friend class Object;
			ValueAssign(json_t* v, const char* key, Object& myObj) : Value(v), m_pMyObj(&myObj), m_key(key) {}
			Object* m_pMyObj;
			const char* m_key;
		};

		bool empty() const { return isEmpty() ? true : 0 == size(); }
		size_t size() const;
		bool hasField(const char* key) const;
		bool hasField(const std::string& key) const { return hasField(key.c_str()); }
		const Value operator[](const char* key) const;
		const Value operator[](const std::string& key) const { return (*this)[key.c_str()]; }
		ValueAssign operator[](const char* key);
		ValueAssign operator[](const std::string& key) { return (*this)[key.c_str()]; }
		void setAt(const char* key, const Value& val);
		void setAt(const std::string& key, const Value& val) { setAt(key.c_str(), val); }
		bool erase(const char* key);
		bool erase(const std::string& key) { return erase(key.c_str()); }
		void clear();

		class ZJSON_EXP_IMP const_iterator : public std::forward_iterator_tag
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::pair<const char*, const Value>;
			using difference_type = int64_t;
			using pointer = const value_type*;
			using reference = const value_type&;

			const_iterator(const Object& pObj, bool atEnd);
			//default copy assign are fine
			bool operator==(const const_iterator& rhs) const { compatItr(rhs); return m_iter == rhs.m_iter; }
			bool operator!=(const const_iterator& rhs) const { compatItr(rhs); return m_iter != rhs.m_iter; }
			const_iterator& operator++();//preincrement
			const_iterator operator++(int) { const_iterator ret = *this; operator++(); return ret; }//postincrement

			const char* key() const { return operator*().first; }
			const Value& value() const { return operator*().second; }
			reference operator*() const;
			pointer operator->() const { return &operator*(); }

		private:
			const Object* m_pMyObj;//keep it first
			void* m_iter;
			mutable value_type m_curVal;//helps to implemnent operator->
			json_t* rawObj() const { return const_cast<Object*>(m_pMyObj)->m_val; }
			void compatItr(const const_iterator& rhs) const { if (m_pMyObj != rhs.m_pMyObj) throw Exc("JSON error: Incompatible obj iterators"); }
		};
		const_iterator begin() const { return const_iterator(*this, false); }
		const_iterator end() const { return const_iterator(*this, true); }
	private:
		friend ZJSON_EXP_IMP std::ostream& operator<<(std::ostream& os, const Object&);
		friend ZJSON_EXP_IMP std::istream& operator>>(std::istream& is, Object&);
	};

	//tabulated output: os << json::setOStreamIdent(4)
	struct ZJSON_EXP_IMP setOStreamIdent
	{
		//Constuct with 0 for compact
		setOStreamIdent(int ident) : m_ident(ident) {}
		friend ZJSON_EXP_IMP std::ostream& operator<<(std::ostream& os, const setOStreamIdent& ident);

	private:
		const int m_ident;
	};
	ZJSON_EXP_IMP std::ostream& operator<<(std::ostream& os, const setOStreamIdent& ident);

	//sorted objects fields output: os << json::setOStreamSorted
	ZJSON_EXP_IMP std::ostream& setOStreamSorted(std::ostream& os);

	//currently used only for dumping values. Ignore output failures (e.g. invalid UTF8) instead of throwing: os << json::setIgnoreOutErrs
	ZJSON_EXP_IMP std::ostream& setIgnoreOutErrs(std::ostream& os);

	ZJSON_EXP_IMP std::ostream& operator<<(std::ostream& os, const Array& arrVal);
	ZJSON_EXP_IMP std::ostream& operator<<(std::ostream& os, const Object& objVal);

	template<class jObjOrArr>
	inline std::string jsonToString(const jObjOrArr& jSmthing, bool sortIt = false, int identSpaces = 0)
	{
		std::ostringstream os;
		if (sortIt)
			os << setOStreamSorted;
		if (0 != identSpaces)
			os << setOStreamIdent(identSpaces);
		os << jSmthing;
		return os.str();
	}

	ZJSON_EXP_IMP std::istream& operator>>(std::istream& is, Array& arrVal);
	ZJSON_EXP_IMP std::istream& operator>>(std::istream& is, Object& objVal);

	ZJSON_EXP_IMP Object strToObject(const char* cStr, size_t sz);//no extra mem copy
	ZJSON_EXP_IMP inline Object strToObject(const std::string& strJ) { return strToObject(strJ.c_str(), strJ.size()); }
	ZJSON_EXP_IMP Array strToArray(const char* cStr, size_t sz);//no extra mem copy
	ZJSON_EXP_IMP inline Array strToArray(const std::string& strJ) { return strToArray(strJ.c_str(), strJ.size()); }
	ZJSON_EXP_IMP inline void stringToJson(const char* cStr, size_t sz, Object& jO) { jO = strToObject(cStr, sz); }
	ZJSON_EXP_IMP inline void stringToJson(const char* cStr, size_t sz, Array& jA) { jA = strToArray(cStr, sz); }
	template<class ObjOrArr>
	inline void stringToJson(const std::string& strJ, ObjOrArr& j) { stringToJson(strJ.c_str(), strJ.size(), j); }
}//namespace json

/*Might be possible to use as header only, but that means adding
* jansson header(s) to the public interface (slow compile & include/link deps).
* To do so, each definition in the cpp must be prefixed with ZJSON_INLINE
* and clients can define ZJSON_INLINE before including this header and uncomment:
#if !defined(ZJSON_BUILD_SHARED) && defined(ZJSON_INLINE)
#define ZJSON_INLINE inline
#include "zjson.cpp"
#endif
*/