import os

lang = os.getenv('COMPILE_LANG')

Import("env")

if lang == 'de':
    env.Append(BUILD_FLAGS=["-D LANG_DE"])
