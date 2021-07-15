//
//  fart-napi.hpp
//  fart-napi
//
//  Created by Kristian Trenskow on 14/01/2021.
//

#ifndef fart_napi_hpp
#define fart_napi_hpp

namespace fart {

	class napi {

	public:

		static Strong<Type> left(Napi::Env env, Napi::Value value, bool napiExceptions) {
			if (value.IsArray()) {

				Strong<Array<Type>> fArray;
				Napi::Array nArray = value.As<Napi::Array>();

				for (uint32_t idx = 0 ; idx < nArray.Length() ; idx++) {
					Strong<Type> item = left(env, nArray[idx], napiExceptions);
					if (item == nullptr) continue;
					fArray->append(item);
				}

				return fArray.as<Type>();

			}
			else if (value.IsBoolean()) {
				return Strong<Boolean>(value.As<Napi::Boolean>().Value()).as<Type>();
			}
			else if (value.IsDate()) {
				return Strong<Date>(Duration::fromMilliseconds(value.As<Napi::Date>().ValueOf()), Date::TimeZone::utc).as<Type>();
			}
			else if (value.IsNull()) {
				return Strong<Null>().as<Type>();
			}
			else if (value.IsNumber()) {
				double iValue = value.As<Napi::Number>().DoubleValue();
				if (iValue == round(iValue)) return Strong<Integer>(iValue).as<Type>();
				return Strong<Float>(iValue).as<Type>();
			}
			else if (value.IsObject()) {

				Strong<Dictionary<Type, Type>> fDictionary;
				Napi::Object nObject = value.As<Napi::Object>();

				Napi::Array keys = nObject.GetPropertyNames();

				for (uint32_t idx = 0 ; idx < keys.Length() ; idx++) {
					Strong<Type> value = left(env, nObject.Get(keys[idx]), napiExceptions);
					if (value == nullptr) continue;
					fDictionary->set(left(env, keys[idx], napiExceptions), value);
				}

				return fDictionary.as<Type>();

			}
			else if (value.IsString()) {
				return Strong<String>(value.As<Napi::String>().Utf8Value().c_str()).as<Type>();
			}
			else if (value.IsUndefined()) {
				return nullptr;
			}

			if (napiExceptions) throw Napi::Error::New(env, "Cannot convert data type.");
			throw TypeConversionException();

		}

		template<class T>
		static Strong<T> left(Napi::Env env, Napi::Value value, bool napiExceptions) {
			return left(env, value, napiExceptions).as<T>();
		}

		static Napi::Value right(Napi::Env env, const Type& value, bool napiExceptions) {

			switch (value.kind()) {
				case Type::Kind::array: {

					Array<Type>& fArray = value.as<Array<Type>>();
					Napi::Array nArray = Napi::Array::New(env);

					for (uint32_t idx = 0 ; idx < fArray.count() ; idx++) {
						nArray.Set(idx, right(env, fArray[idx], napiExceptions));
					}

					return nArray;

				}
				case Type::Kind::data: {
					if (napiExceptions) throw Napi::Error::New(env, "Cannot convert data type.");
					throw TypeConversionException();
				}
				case Type::Kind::date: {
					Date date = value.as<Date>().to(Date::TimeZone::utc);
					return Napi::Date::New(env, date.durationSinceEpoch().milliseconds());
				}
				case Type::Kind::dictionary: {

					Dictionary<Type, Type>& fDictionary = value.as<Dictionary<Type, Type>>();
					Napi::Object nObject = Napi::Object::New(env);

					Array<Type> keys = fDictionary.keys();
					Array<Type> values = fDictionary.values();

					for (size_t idx = 0 ; idx < fDictionary.count() ; idx++) {
						Napi::Value key = right(env, keys[idx], napiExceptions);
						Napi::Value value = right(env, values[idx], napiExceptions);
						nObject.Set(key, value);
					}

					return nObject;

				}
				case Type::Kind::null:
					return env.Null();
				case Type::Kind::number: {
					switch (value.as<Number<uint64_t>>().subType()) {
						case Subtype::boolean:
							return Napi::Boolean::New(env, value.as<Boolean>().value());
						case Subtype::integer:
							return Napi::Number::New(env, value.as<Integer>().value());
						case Subtype::floatingPoint:
							return Napi::Number::New(env, value.as<Float>().value());
						default:
							if (napiExceptions) throw Napi::Error::New(env, "Cannot convert data type.");
							throw TypeConversionException();
					}
				}
				case Type::Kind::string: {
					return value.as<String>().mapCString<Napi::String>([&env](const char* string) {
						return Napi::String::New(env, string);
					});
				}
				default:
					if (napiExceptions) throw Napi::Error::New(env, "Cannot convert data type.");
					throw TypeConversionException();
			}

		}

		template<typename T>
		static T right(Napi::Env env, const Type& value, bool napiExceptions) {
			return right(env, value, napiExceptions).As<T>();
		}

	};


}

#endif /* fart_napi_hpp */
