#!/bin/bash
cd $(dirname $0)

find src/ test/ pythonwrappers/ include/ \( -iname '*.cpp' -or -iname '*.hpp' \) \
    -and -not -iname bearmodule.* |
    while read file; do
        clang-format -i "$file"
        if [[ $(tail -c 1 "$file" | xxd -p) != 0a ]]; then
            echo >> "$file"
            echo "added newline to end of $file"
        fi
    done

cmake-format --autosort true --enable-sort true -i CMakeLists.txt {src,data,test,pythonwrappers,submodules}/CMakeLists.txt

black test/
