# Adds `pio run -t serve`: builds the firmware and serves it on the local
# network, so the scale can update from this computer (dev updates).
import os

Import("env")

serve_script = os.path.join(env.subst("$PROJECT_DIR"), "scripts", "serve_firmware.py")

env.AddCustomTarget(
    name="serve",
    dependencies="$BUILD_DIR/${PROGNAME}.bin",
    actions=['"$PYTHONEXE" "%s" "$BUILD_DIR/${PROGNAME}.bin"' % serve_script],
    title="Serve firmware",
    description="Build and serve the firmware for a dev update over Wi-Fi",
)
