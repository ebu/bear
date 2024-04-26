#include "tensorfile.hpp"

#include <sstream>

// Prevent mio's window.h include assigning problematic macros
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "mio.hpp"

namespace tensorfile {

class MMap : public mio::ummap_source {
  using mio::ummap_source::ummap_source;
};

namespace detail {

  /// n-dimensional array with a specific type which holds a smart pointer to keep the memory alive
  template <typename T, typename PtrT>
  class NDArrayTHold : public NDArrayT<T> {
   public:
    NDArrayTHold(
        std::string dtype, std::vector<size_t> shape, std::vector<size_t> strides, T *base_ptr, PtrT ptr)
        : NDArrayT<T>(std::move(dtype), std::move(shape), std::move(strides), base_ptr), ptr(std::move(ptr))
    {
    }

   private:
    PtrT ptr;
  };

  template <typename T>
  T read_unsigned(const unsigned char *data)
  {
    T acc = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
      acc += ((T)data[i]) << (i * 8);
    }
    return acc;
  }

  enum ByteOrder {
    UNKNOWN = 0,
    LITTLE = 1,
    BIG = 2,
    EITHER = 3,
  };

  ByteOrder parse_byte_order(const std::string &type)
  {
    if (type[0] == '<')
      return ByteOrder::LITTLE;
    else if (type[0] == '>')
      return ByteOrder::BIG;
    else if (type[0] == '|')
      // "not applicable", i.e. 1-byte types which work on either
      return ByteOrder::EITHER;
    else
      throw format_error("unknown byte order");
  }

  std::string parse_type(const std::string &type) { return type.substr(1, std::string::npos); }

  /// get the current system byte order
  ByteOrder get_byte_order()
  {
    uint32_t x = 0x04030201;
    uint32_t y = read_unsigned<uint32_t>((unsigned char *)&x);

    if (y == 0x04030201)
      return ByteOrder::LITTLE;
    else if (y == 0x01020304)
      return ByteOrder::BIG;
    else
      return ByteOrder::UNKNOWN;
  }

  void copy_bswap(void *dest, const void *src, size_t n, size_t size)
  {
    char *dest_c = (char *)dest;
    const char *src_c = (const char *)src;
    for (size_t i = 0; i < n; i++) {
      for (size_t j = 0; j < size; j++) {
        dest_c[i * size + j] = src_c[i * size + ((size - 1) - j)];
      }
    }
  }

  /// knows how to unpack a particular type of array, polymorphic so that we
  /// can easily chose at run-time which one to use. This doesn't make sense to
  /// go in the array types, because depending on whether we have to copy or
  /// not the smart pointer type will be different
  struct TypeHandler {
    virtual std::shared_ptr<NDArray> operator()(std::string dtype,
                                                std::vector<size_t> shape,
                                                std::vector<size_t> strides,
                                                std::shared_ptr<MMap> mmap,
                                                size_t offset) = 0;
    virtual ~TypeHandler() = default;
  };

  template <typename T>
  struct TypeHandlerT : public TypeHandler {
   public:
    std::shared_ptr<NDArray> operator()(std::string dtype,
                                        std::vector<size_t> shape,
                                        std::vector<size_t> strides,
                                        std::shared_ptr<MMap> mmap,
                                        size_t offset) override
    {
      // strides in the file are in bytes, so divide by the type size
      for (auto &stride : strides) {
        if (stride % sizeof(T) != 0) throw format_error("stride is not a multiple of the type size");
        stride /= sizeof(T);
      }

      ByteOrder storage_order = parse_byte_order(dtype);
      ByteOrder current_order = get_byte_order();

      if (current_order == ByteOrder::UNKNOWN) throw std::logic_error("unknown byte order");

      bool aligned = (size_t)(mmap->data() + offset) % alignof(T) == 0;

      if (aligned && (storage_order & current_order)) {
        using ArrayT = NDArrayTHold<T, std::shared_ptr<MMap>>;
        return std::make_shared<ArrayT>(std::move(dtype),
                                        std::move(shape),
                                        std::move(strides),
                                        (T *)(mmap->data() + offset),
                                        std::move(mmap));
      } else {
        size_t num_elements = 1;
        for (size_t i = 0; i < shape.size(); i++) num_elements += strides[i] * (shape[i] - 1);

        std::vector<T> storage(num_elements);
        if (storage_order & current_order)
          memcpy(storage.data(), mmap->data() + offset, num_elements * sizeof(T));
        else
          copy_bswap(storage.data(), mmap->data() + offset, num_elements, sizeof(T));

        using ArrayT = NDArrayTHold<T, std::vector<T>>;
        return std::make_shared<ArrayT>(
            std::move(dtype), std::move(shape), std::move(strides), storage.data(), std::move(storage));
      }
    }
  };

  // get the handler for a particular type
  std::unique_ptr<TypeHandler> get_type_handler(const std::string &dtype)
  {
    std::string type = parse_type(dtype);
    if (type == "f8")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<double>());
    else if (type == "f4")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<float>());
    else if (type == "u8")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<uint64_t>());
    else if (type == "u4")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<uint32_t>());
    else if (type == "u2")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<uint16_t>());
    else if (type == "u1")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<uint8_t>());
    else if (type == "i8")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<int64_t>());
    else if (type == "i4")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<int32_t>());
    else if (type == "i2")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<int16_t>());
    else if (type == "i1")
      return std::unique_ptr<TypeHandler>(new TypeHandlerT<int8_t>());
    else
      return std::unique_ptr<TypeHandler>();
  }

}  // namespace detail

std::shared_ptr<NDArray> TensorFile::unpack(const nlohmann::json &v) const
{
  if (std::string(v.at("_tenf_type").template get<std::string>()) == "array") {
    std::vector<size_t> shape = v.at("shape").template get<std::vector<size_t>>();

    std::vector<size_t> strides = v.at("strides").template get<std::vector<size_t>>();

    if (strides.size() != shape.size()) throw format_error("mismatched strides and shape");

    std::string dtype = v.at("dtype").template get<std::string>();
    size_t offset = v.at("base").template get<size_t>();

    auto type_handler = detail::get_type_handler(dtype);
    return (*type_handler)(std::move(dtype), std::move(shape), std::move(strides), mmap, offset);

  } else {
    throw format_error("unknown type");
  }
}

TensorFile read(const std::string &path)
{
  auto mmap = std::make_shared<MMap>(path);
  const unsigned char *data = mmap->data();
  size_t header_len = 4 + 4 + 8 + 8;
  if (mmap->length() < header_len) throw format_error("file not long enough");

  if (std::string((const char *)data, 4) != "TENF") throw format_error("magic number not found");

  auto version = detail::read_unsigned<uint32_t>(data + 4);
  if (version != 0) throw format_error("unknown version number");
  auto data_len = detail::read_unsigned<uint64_t>(data + 8);
  auto metadata_len = detail::read_unsigned<uint64_t>(data + 16);

  if (mmap->length() < header_len + data_len + metadata_len) throw format_error("file not long enough");
  if (metadata_len == 0) throw format_error("no JSON metadata found");

  const char *metadata_start = reinterpret_cast<const char *>(data) + header_len + data_len;
  const char *metadata_end = metadata_start + metadata_len;
  nlohmann::json metadata;
  try {
    metadata = nlohmann::json::parse(metadata_start, metadata_end);
  } catch (const nlohmann::json::parse_error &e) {
    throw format_error(std::string{"could not parse JSON metadata: "} + e.what());
  }

  return TensorFile(std::move(mmap), std::move(metadata));
}

}  // namespace tensorfile
