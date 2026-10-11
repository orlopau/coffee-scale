Import("env")

# The native builds use whatever gcc the computer has. Newer ones default to C23,
# where QRCode's `typedef unsigned char bool` doesn't compile, and from gcc 16 to
# C++20, which rejects initializers like the ones in data/recipes.h. Pinned so the
# tests build with any gcc. build_flags can't do this, they go to C and C++ alike.
env.Append(CFLAGS=["-std=gnu17"], CXXFLAGS=["-std=gnu++17"])
