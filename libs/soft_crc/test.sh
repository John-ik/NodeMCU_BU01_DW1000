#!/bin/env bash

set -e # завершить при любой ошибке

# генератор генератора тестов
# HTML -> Bash -> Dlang unittest

url=https://reveng.sourceforge.io/crc-catalogue/all.htm
target=generator_D.sh

echo > "$target" '
exec > "src/generated_test_soft_crc.d"

function f(){
    if [[ $width = 8 ]]; then
        T=ubyte
    elif [[ $width = 16 ]]; then
        T=ushort
    elif [[ $width = 32 ]]; then
        T=uint
    elif [[ $width = 64 ]]; then
        T=ulong
    else
        return 0
    fi

    echo "@CRC_unit(\"$name\")"
    echo "unittest"
    echo "{"
    echo "    CRC!($T, $poly, $init, cast(Flag!\"refin\") $refin, cast(Flag!\"refout\") $refout, $xorout) crc;"
    echo "    crc.put(str);"
    echo "    assert(crc.get() == $check);"
    echo "}"
}
'
cat >> "$target" <<EOF
echo "// auto-generated from $url"
echo 'import soft_crc;'
echo 'import std.typecons : Flag, Yes, No;'
echo 'immutable str = cast(ubyte[9]) "123456789"; /// check byte string'
echo ''
EOF

wget "$url"
file=all.htm
xmllint --html --xpath '//p/code/text()' "$file" | sed 's/  /; /g; s/$/ f\;/;' >> "$target"
rm "$file"

echo "В $target записан скрипт-генератор юниттестов на D"
