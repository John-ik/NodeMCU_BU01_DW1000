set -e

build_dir='build/d'
sources="libs/soft_crc/src/soft_crc.d"

echo 'Build D (ldc)'
echo "build_dir = $build_dir"
echo "sources = $sources"

ldc -c -betterC -ICore/Src/ -P-ICore/Inc/ \
    -march arm -mcpu cortex-m3 -gc \
    --od "$build_dir" --mixin "$build_dir/mixins.not.d" \
    $sources

echo 'make .h:'

for file in $sources; do
    target="$file.h"
    echo '#pragma once' > "$target"
    echo '#include "stdint.h"' >> "$target"
    ldc -o- -betterC -ICore/Src/ -P-ICore/Inc/ --HC $file | sed -n 's/extern "C"/extern/p' >> "$target"
    echo "write $file header to $target"
done