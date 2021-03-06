/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <boost/optional.hpp>

# include <libport/cassert>
# include <libport/format.hh>
# include <libport/path.hh>
# include <libport/symbol.hh>

# include <urbi/object/urbi-exception.hh>

# include <urbi/object/cxx-object.hh>
# include <urbi/object/float.hh>
# include <urbi/object/list.hh>
# include <urbi/object/path.hh>
# include <urbi/object/string.hh>
# include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    /*----------.
    | Helpers.  |
    `----------*/

    /// Convert an rFloat to an integral value.
    template <typename T>
    T
    to_integer(const rObject& o)
    {
      libport::ufloat value = o->as<Float>()->value_get();
      try
      {
        return libport::numeric_cast<T>(value);
      }
#define RETHROW(Name, Message)                                          \
    catch (const libport::Name&)                                        \
    {                                                                   \
      runner::raise_bad_integer_error(value,                            \
                                      std::string(Message) + ": %s");   \
    }
    RETHROW(negative_overflow, "expected non-negative integer")
    RETHROW(positive_overflow, "expected non-positive integer")
    RETHROW(bad_numeric_cast, "expected integer")
#undef RETHROW
    }


    /*----------.
    | Objects.  |
    `----------*/

    template <typename T>
    typename CxxConvert<T>::target_type
    CxxConvert<T>::to(rObject o)
    {
      return *type_check<T>(o);
    }

    template <typename T>
    rObject
    CxxConvert<T>::from(source_type v)
    {
      return &v;
    }

    template <>
    struct CxxConvert<libport::intrusive_ptr<Object> >
    {
      typedef libport::intrusive_ptr<Object> target_type;

      static rObject
      to(const rObject& o)
      {
        return o;
      }

      static rObject
      from(rObject o)
      {
        if (o)
          return o;
        return nil_class;
      }
    };


    /*-------------.
    | Urbi types.  |
    `-------------*/

    template <typename Urbi>
    struct CxxConvert<libport::intrusive_ptr<Urbi> >
    {
      typedef libport::intrusive_ptr<Urbi> target_type;
      static target_type
      to(const rObject& o)
      {
        return type_check<Urbi>(o);
      }

      static rObject
      from(const target_type& v)
      {
        return v;
      }
    };

    /*-------------.
    | RefCounted.  |
    `-------------*/

    template <>
    struct CxxConvert<const libport::RefCounted*>
    {
      typedef libport::RefCounted* target_type;
      static target_type
      to(Object* o)
      {
        return o;
      }
    };

    /*----------------------.
    | Urbi types pointers.  |
    `----------------------*/

    template <typename Urbi>
    struct CxxConvert<Urbi*>
    {
      typedef Urbi* target_type;
      static target_type
      to(const rObject& o)
      {
        return type_check<Urbi>(o);
      }

      static rObject
      from(target_type v)
      {
        return v;
      }
    };

    /*------.
    | int.  |
    `------*/

    template<>
    struct CxxConvert<int>
    {
      typedef Float::int_type target_type;
      static target_type
      to(const rObject& o)
      {
        return type_check<Float>(o)->to_int_type();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };


    /*-----------------.
    | Integral types.  |
    `-----------------*/

#define CONVERT(Type)                           \
    template<>                                  \
    struct CxxConvert<Type>                     \
    {                                           \
      typedef Type target_type;                 \
      static target_type                        \
      to(const rObject& o)                      \
      {                                         \
        type_check<Float>(o);                   \
        return to_integer<target_type>(o);      \
      }                                         \
                                                \
      static rObject                            \
      from(target_type v)                       \
      {                                         \
        return new Float(v);                    \
      }                                         \
    };

    CONVERT(long long);
    CONVERT(long);
    CONVERT(short);
    CONVERT(unsigned char);
    CONVERT(unsigned long long);
    CONVERT(unsigned long);
    CONVERT(unsigned short);
#undef CONVERT


    /*--------.
    | float.  |
    `--------*/

    template<>
    struct CxxConvert<float>
    {
      typedef float target_type;
      static target_type
      to(const rObject& o)
      {
        return type_check<Float>(o)->value_get();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };


    /*----------------.
    | unsigned_type.  |
    `----------------*/

    template<>
    struct CxxConvert<Float::unsigned_type>
    {
      typedef Float::unsigned_type target_type;
      static target_type
      to(const rObject& o)
      {
	return type_check<Float>(o)->to_unsigned_type();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };


    /*-----------------.
    | floating point.  |
    `-----------------*/

    template<>
    struct CxxConvert<double>
    {
      typedef double target_type;
      static target_type
      to(const rObject& o)
      {
        return type_check<Float>(o)->value_get();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };

# if defined LIBPORT_URBI_UFLOAT_LONG || defined LIBPORT_URBI_UFLOAT_LONG_LONG
    // Ufloat if neither double nor float
    template<>
    struct CxxConvert<ufloat>
    {
      typedef double target_type;
      static target_type
      to(const rObject& o)
      {
        return type_check<Float>(o)->value_get();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };
#endif

    /*--------------.
    | std::string.  |
    `--------------*/

    template <>
    struct CxxConvert<std::string>
    {
      typedef       std::string  target_type;
      typedef const std::string& source_type;
      static target_type
      to(const rObject& o)
      {
        if (rPath p = o->as<Path>())
          return p->value_get();
        return type_check<String>(o)->value_get();
      }

      static rObject
      from(source_type v)
      {
        return new String(v);
      }
    };


    /*-------.
    | char.  |
    `-------*/

    template <>
    struct CxxConvert<char>
    {
      typedef char target_type;
      static target_type
      to(const rObject& o)
      {
        std::string str = type_check<String>(o)->value_get();
        if (str.size() != 1)
          runner::raise_primitive_error("expected one character string");
          //FIXME: Primitive error or custom type error?
        return str[0];
      }

      static rObject
      from(target_type v)
      {
        return new String(std::string(1, v));
      }
    };


    /*------------------------.
    | char* and const char*.  |
    `------------------------*/

/// For char* and const char* the CxxConvert template is intentionally
/// specialized without the 'to' method that would imply a memory leak.

#define CONVERT(Type)                                           \
    template <>                                                 \
    struct CxxConvert<Type>                                     \
    {                                                           \
      typedef Type target_type;                                 \
                                                                \
      static rObject                                            \
      from(target_type v)                                       \
      {                                                         \
        return CxxConvert<std::string>::from(std::string(v));   \
      }                                                         \
    };

    CONVERT(char*);
    CONVERT(const char*);

#undef CONVERT


    /*------------------.
    | libport::Symbol.  |
    `------------------*/

    template <>
    struct CxxConvert<libport::Symbol>
    {
      typedef libport::Symbol target_type;
      static target_type
      to(const rObject& o)
      {
        return libport::Symbol(type_check<String>(o)->value_get());
      }

      static rObject
      from(target_type v)
      {
        return new String(v.name_get());
      }
    };


    /*-------.
    | bool.  |
    `-------*/

    template <>
    struct CxxConvert<bool>
    {
      typedef bool target_type;
      static target_type
      to(const rObject& o)
      {
        return o->as_bool();
      }

      static rObject
      from(target_type v)
      {
        return v ? true_class : false_class;
      }
    };


    /*-------------------------------------------------------------.
    | std::set, std::vector, std::deque, libport::ReservedVector.  |
    `-------------------------------------------------------------*/

#define CONTAINER(Name, Method, ExtraT, ExtraTDecl)                     \
    template <typename T ExtraTDecl>                                    \
    struct CxxConvert<Name<T ExtraT> >                                  \
    {                                                                   \
      typedef Name<T ExtraT> target_type;                               \
                                                                        \
      static target_type                                                \
        to(const rObject& o)                                            \
      {                                                                 \
        Name<T ExtraT> res;                                             \
        foreach (const rObject& elt, type_check<List>(o)->value_get())	\
          res.Method(CxxConvert<T>::to(elt));                           \
        return res;                                                     \
      }                                                                 \
                                                                        \
      static rObject                                                    \
        from(const target_type& v)                                      \
      {                                                                 \
        objects_type res;                                               \
        foreach (const T& elt, v)                                       \
          res.push_back(CxxConvert<T>::from(elt));                      \
        return new List(res);                                           \
      }                                                                 \
    };

#define comma ,
    CONTAINER(std::set, insert, /**/, /**/);
    CONTAINER(std::vector, push_back, /**/, /**/);
    CONTAINER(std::deque, push_back, /**/, /**/);
    CONTAINER(libport::ReservedVector, push_back, comma R, comma int R);
#undef comma
#undef CONTAINER

    template <>
    struct CxxConvert<objects_type>
    {
      typedef objects_type target_type;

      static target_type
      to(const rObject& o)
      {
        objects_type res;
        foreach (Object* elt, type_check<List>(o)->value_get())
          res.push_back(elt);
        return res;
      }

      static rObject
      from(const target_type& v)
      {
        objects_type res;
        foreach (Object* elt, v)
          res << elt;
        return new List(res);
      }
    };


    /*------------------.
    | boost::optional.  |
    `------------------*/

    template <typename T>
    struct CxxConvert<boost::optional<T> >
    {
      typedef boost::optional<T> target_type;
      static target_type
      to(const rObject& o)
      {
        if (o == nil_class)
          return target_type();
        return CxxConvert<T>::to(o);
      }

      static rObject
      from(const target_type& v)
      {
        if (v)
          return CxxConvert<T>::from(v.get());
        return nil_class;
      }
    };


    /*------------.
    | std::pair.  |
    `------------*/

    template <typename T1, typename T2>
    struct CxxConvert<std::pair<T1, T2> >
    {
      typedef std::pair<T1, T2> target_type;
      static target_type
      to(const rObject& o)
      {
        const List::value_type& list = type_check<List>(o)->value_get();
        if (list.size() != 2)
          runner::raise_primitive_error("expected a list of size 2");
        return std::make_pair(CxxConvert<T1>::to(list[0]),
                              CxxConvert<T2>::to(list[1]));
      }

      static rObject
      from(const target_type& v)
      {
        List::value_type content;
        content << CxxConvert<T1>::from(v.first)
                << CxxConvert<T2>::from(v.second);
        return new List(content);
      }
    };


    /*---------------------.
    | CONVERT_VALUE_TYPE.  |
    `---------------------*/

    // Conversion for CxxObjects that define a value_type, provide
    // a constructor for it, and a value_get() accessor.

# define CONVERT_VALUE_TYPE(Type)                       \
    template <>                                         \
    struct CxxConvert<Type::value_type>                 \
    {                                                   \
      typedef       Type::value_type& target_type;      \
      typedef const Type::value_type& source_type;      \
      static target_type                                \
      to(const rObject& o)                              \
      {                                                 \
        return type_check<Type>(o)->value_get();	\
      }                                                 \
                                                        \
      static rObject                                    \
      from(source_type v)                               \
      {                                                 \
        return new Type(v);                             \
      }                                                 \
    };


    /*--------------------.
    | to_urbi/from_urbi.  |
    `--------------------*/

    template <typename T>
    rObject to_urbi(const T& v)
    {
      return CxxConvert<T>::from(v);
    }

    template <typename T>
    typename CxxConvert<T>::target_type
    from_urbi(rObject v)
    {
      return CxxConvert<T>::to(v);
    }

    template<typename T>
    typename CxxConvert<T>::target_type
    from_urbi(rObject o, unsigned idx)
    {
      try
      {
        return CxxConvert<T>::to(o);
      }
      catch (const UrbiException& e)
      {
        runner::raise_argument_error(idx, e.value());
      }
    }
  }
}

#endif
