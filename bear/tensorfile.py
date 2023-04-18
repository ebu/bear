import numpy as np
import struct
import json

"""
A tensorfile is an alternative to things like HDF5 for storing
multi-dimensional arrays. It behaves like a JSON file, but with efficient
storage for large arrays.

features:
- implementable with minimal dependencies: a JSON parser
- mmap-friendly: no compression, and allows for variable endianness so that
  conversion can be avoided.
- supports arbitrary alignments, of both bases and strides
- stores tensors of floats and integers; everything else is in json

This file contains a reader and writer for this format. A C++ reader can be
found in ../visr_bear/src/tensorfile.{cpp,hpp} .

# file format

A tensorfile consists of:
- 4 byte tag: TENF
- 4 byte version number, currently always 0 (unsigned LE)
- 8 byte data size (unsigned LE)
- 8 byte metadata size (unsigned LE)
- 'data size' bytes of data indexed by json metadata
- 'metadata size' bytes of JSON metadata, storing a user-defined structure
  with arrays represented as offsets into the file with sizes and strides

Arrays are stored in the JSON metadata as objects with the following keys:

- "_tenf_type": the string "array"

- "dtype": numpy "typestr" style data type as described in
  https://numpy.org/doc/stable/reference/arrays.interface.html
  This is a string like "<u8", containing:

  - The byte ordering, < for little endian, > for big endian, or | for 1-byte
    types which have no endianness.
  - The data type; i for signed integers, u for unsigned integers, or f for
    floats.
  - The number of bytes, e.g. 8 for double precision floating point.

- "shape": list of integers, the number of elements in each dimension

- "stride": list of integers, the number of bytes to move to find the next
  element in the corresponding dimension; must be the same length as shape

- "base": the position of the first byte of the first element in the file
  (relative to the start of the file, not the start of the data)

Therefore an element with a given index can be found at byte:

    base + sum(i * s for i, s in zip(index, stride))

A file can store many such objects in the JSON metadata block, in an arbitrary
structure along with non-array data.
"""

_TAG = b"TENF"
_TYPE_KEY = "_tenf_type"


def _write_array(f, data, byte_order, alignment):
    dtype = data.dtype.newbyteorder(byte_order)
    arr = np.ascontiguousarray(data, dtype)

    assert all(stride % dtype.itemsize == 0 for stride in arr.strides)

    padding = (alignment - f.tell() % alignment) % alignment
    f.write(b"\0" * padding)

    base = f.tell()
    arr_bytes = arr.data.tobytes()
    f.write(arr_bytes)

    return {
        _TYPE_KEY: "array",
        "dtype": dtype.str,
        "strides": arr.strides,
        "shape": arr.shape,
        "base": base,
    }


def _read_array(metadata, data_bytes):
    strides, shape = metadata["strides"], metadata["shape"]
    base = metadata["base"]
    dtype = np.dtype(metadata["dtype"])
    length = (
        sum(stride_i * (shape_i - 1) for stride_i, shape_i in zip(strides, shape))
        + dtype.itemsize
    )
    return np.lib.stride_tricks.as_strided(
        np.frombuffer(data_bytes[base : base + length], dtype=dtype),
        shape=shape,
        strides=strides,
        writeable=False,
    )


_writers = [
    (np.ndarray, _write_array),
]

_readers = {
    "array": _read_array,
}


def _pack_data(f, data, byte_order, alignment):
    def do_call(func, data):
        """call wrapper to keep non-data arguments the same"""
        return func(f, data, byte_order=byte_order, alignment=alignment)

    for types, write_func in _writers:
        if isinstance(data, types):
            return do_call(write_func, data)

    if isinstance(data, dict):
        return {key: do_call(_pack_data, value) for key, value in data.items()}
    elif isinstance(data, (list, tuple)):
        return [do_call(_pack_data, value) for value in data]
    else:
        return data


def write(f, data, byte_order="<", alignment=32):
    """Write a data structure to a file.
    Parameters:
        f (file-like): file to write, opened in binary mode
        data (dictionary, list or array): data structure to write to f
        byte_order ('<' or '>'): byte order to write in
        alignment: alignment of the start of arrays relative to the start of
        the file
    """
    f.write(_TAG)
    f.write(struct.pack("<IQQ", 0, 0, 0))

    data_start = f.tell()
    json_data = _pack_data(f, data, byte_order=byte_order, alignment=alignment)
    data_len = f.tell() - data_start

    json_binary = json.dumps(json_data).encode("utf8")
    json_len = len(json_binary)
    f.write(json_binary)

    f.seek(8)
    f.write(struct.pack("<QQ", data_len, json_len))


def _unpack_data(metadata, data_bytes):
    if isinstance(metadata, dict):
        if _TYPE_KEY in metadata:
            type_str = metadata[_TYPE_KEY]
            reader = _readers[type_str]
            return reader(metadata, data_bytes)
        else:
            out = dict()
            for key, value in metadata.items():
                out[key] = _unpack_data(value, data_bytes)
            return out
    elif isinstance(metadata, list):
        out = []
        for value in metadata:
            out.append(_unpack_data(value, data_bytes))
        return out
    else:
        return metadata


def read(f):
    f_bytes = np.memmap(f, mode="r").data
    assert f_bytes[:4] == _TAG

    (version,) = struct.unpack("<I", f_bytes[4:8])
    assert version == 0

    data_size, metadata_size = struct.unpack("<QQ", f_bytes[8:24])

    metadata_bytes = f_bytes[24 + data_size : 24 + data_size + metadata_size]
    metadata = json.loads(bytes(metadata_bytes).decode("utf8"))

    return _unpack_data(metadata, f_bytes)


def _main():
    import argparse

    parser = argparse.ArgumentParser()

    subparsers = parser.add_subparsers(help="sub-command", required=True)

    def show(args):
        contents = read(args.file)

        def to_json(x):
            if isinstance(x, dict):
                return {k: to_json(v) for k, v in x.items()}
            elif isinstance(x, list):
                return [to_json(v) for v in x]
            elif isinstance(x, np.ndarray):
                if x.size < 100:
                    return x.tolist()
                else:
                    return f"array with shape {x.shape} and type {x.dtype}"
            else:
                return x

        import json

        print(json.dumps(to_json(contents), indent=4))

    show_parser = subparsers.add_parser(
        "show", help="show the contents of a tensorfile"
    )
    show_parser.add_argument("file", type=argparse.FileType(mode="rb"))
    show_parser.set_defaults(func=show)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    _main()
