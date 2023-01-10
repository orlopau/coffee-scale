import subprocess

def get_firmware_specifier_build_flag():
    ret = subprocess.run(["git", "describe", "--abbrev=0", "--match=firmware-*"], stdout=subprocess.PIPE, text=True)
    tag = ret.stdout.strip()
    print("Last firmware tag: " + tag)

    version = tag.split("-")[1]; 
    build_flag = "-D FIRMWARE_VERSION=\\\"" + version + "\\\""
    print ("revision=" + version)
    return (build_flag)

Import("env")
env.Append(
    BUILD_FLAGS=[get_firmware_specifier_build_flag()]
)