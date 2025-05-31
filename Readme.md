# Description

A C++ program optimized for reversing UTF-8, BIG5 and ASCII encodings in large txt file, with memory-mapped reversal and parallel processing

# Features
- Encoding Detection: Identifies UTF-8, Big5, and ASCII encodings using left-to-right byte reading.

- Memory Mapping: Uses boost::mapped_region for efficient file processing without loading the entire file.

- Parallel Processing: Implements a single-producer, multi-consumer model for thread-safe task execution.


# Encoding Rules
- ASCII: First byte in 0x20 to 0x7f.

- UTF-8: First byte ≥ 0xe0, followed by 2 bytes forming a 3-byte Chinese character.

- Big5: First byte outside ASCII range, paired with a second byte for a 2-byte character.

- Mixed Encoding: Supports UTF-8/Big5/ASCII by prioritizing UTF-8 (3 bytes), ASCII (1 byte), then Big5 (2 bytes).

# Installation

## Clone
```
git clone https://github.com/v54dt/zh_TW-character-flipper.git &&
cd zh_TW-character-flipper
```

## Build Dependency Image
```
docker build -f ./dockerfile/env.dockefile -t env .
```

## Build
```
docker container run -it -v `pwd`:/app/zh_TW-character-flipper env &&
cd zh_TW-character-flipper &&
cmake -S . -B build && cmake --build build
```
## Usage
```
cd build &&
./flipper --filepath ../test_file.txt --batch_size 4088 --type utf8
# or
# ./flipper --filepath ../test_file.txt --batch_size 4088 --type big5
```

## Dependency
- C++20 or later

- Boost (boost::mapped_region)

- Abseil

