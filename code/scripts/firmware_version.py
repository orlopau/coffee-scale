import os
import subprocess


def get_firmware_version():
    # e.g. FIRMWARE_VERSION=v1.8.0, so the scale sees the latest release as newer
    override = os.getenv("FIRMWARE_VERSION")
    if override:
        print("Firmware version from FIRMWARE_VERSION: " + override)
        return override
    ret = subprocess.run(["git", "describe", "--abbrev=0", "--match=firmware-*"], stdout=subprocess.PIPE, text=True)
    tag = ret.stdout.strip()
    print("Last firmware tag: " + tag)
    return tag.split("-")[1]


def get_build_flags():
    version = get_firmware_version()
    print("revision=" + version)
    flags = ["-D FIRMWARE_VERSION=\\\"" + version + "\\\""]
    # e.g. RELEASES_URL=http://192.168.1.20:8000/releases, the dev server's release
    releases = os.getenv("RELEASES_URL")
    if releases:
        print("Releases from RELEASES_URL: " + releases)
        flags.append("-D RELEASES_URL=\\\"" + releases + "\\\"")
    return flags


Import("env")
env.Append(BUILD_FLAGS=get_build_flags())
