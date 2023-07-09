#include "cpp_config.hpp"
#include "msgpack/adaptor/array_ref.hpp"
#include "msgpack/adaptor/bool.hpp"
#include "msgpack/adaptor/carray.hpp"
#include "msgpack/adaptor/char_ptr.hpp"
#include "msgpack/adaptor/define.hpp"
#include "msgpack/adaptor/deque.hpp"
#include "msgpack/adaptor/ext.hpp"
#include "msgpack/adaptor/fixint.hpp"
#include "msgpack/adaptor/float.hpp"
#include "msgpack/adaptor/int.hpp"
#include "msgpack/adaptor/list.hpp"
#include "msgpack/adaptor/map.hpp"
#include "msgpack/adaptor/msgpack_tuple.hpp"
#include "msgpack/adaptor/nil.hpp"
#include "msgpack/adaptor/pair.hpp"
#include "msgpack/adaptor/raw.hpp"
#include "msgpack/adaptor/set.hpp"
#include "msgpack/adaptor/size_equal_only.hpp"
#include "msgpack/adaptor/string.hpp"
#include "msgpack/adaptor/v4raw.hpp"
#include "msgpack/adaptor/vector.hpp"
#include "msgpack/adaptor/vector_bool.hpp"
#include "msgpack/adaptor/vector_char.hpp"
#include "msgpack/adaptor/vector_unsigned_char.hpp"
#include "msgpack/adaptor/wstring.hpp"

#if defined(MSGPACK_USE_CPP03)

#include "adaptor/tr1/unordered_map.hpp"
#include "adaptor/tr1/unordered_set.hpp"

#else  // defined(MSGPACK_USE_CPP03)

#include "msgpack/adaptor/cpp11/array.hpp"
#include "msgpack/adaptor/cpp11/array_char.hpp"
#include "msgpack/adaptor/cpp11/array_unsigned_char.hpp"
#include "msgpack/adaptor/cpp11/chrono.hpp"
#include "msgpack/adaptor/cpp11/forward_list.hpp"
#include "msgpack/adaptor/cpp11/reference_wrapper.hpp"
#include "msgpack/adaptor/cpp11/shared_ptr.hpp"
#include "msgpack/adaptor/cpp11/timespec.hpp"
#include "msgpack/adaptor/cpp11/tuple.hpp"
#include "msgpack/adaptor/cpp11/unique_ptr.hpp"
#include "msgpack/adaptor/cpp11/unordered_map.hpp"
#include "msgpack/adaptor/cpp11/unordered_set.hpp"

#if MSGPACK_HAS_INCLUDE(<optional>)
#include "adaptor/cpp17/optional.hpp"
#endif // MSGPACK_HAS_INCLUDE(<optional>)

#if MSGPACK_HAS_INCLUDE(<string_view>)
#include "adaptor/cpp17/string_view.hpp"
#endif // MSGPACK_HAS_INCLUDE(<string_view>)

#include "msgpack/adaptor/cpp17/byte.hpp"
#include "msgpack/adaptor/cpp17/carray_byte.hpp"
#include "msgpack/adaptor/cpp17/vector_byte.hpp"

#endif // defined(MSGPACK_USE_CPP03)

#if defined(MSGPACK_USE_BOOST)

#include "adaptor/boost/fusion.hpp"
#include "adaptor/boost/msgpack_variant.hpp"
#include "adaptor/boost/optional.hpp"
#include "adaptor/boost/string_ref.hpp"
#include "adaptor/boost/string_view.hpp"

#endif // defined(MSGPACK_USE_BOOST)
