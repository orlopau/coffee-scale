# Adds the targets of the dev server (scripts/dev_server.py):
#   pio run -t serve   builds the firmware and serves it on the local network,
#                      so the scale can update from this computer (dev updates)
#   pio run -t record  receives recordings of the scale's load cell into
#                      test/recordings, without building anything
import os

Import("env")

dev_server = os.path.join(env.subst("$PROJECT_DIR"), "scripts", "dev_server.py")
recordings = os.path.join(env.subst("$PROJECT_DIR"), "test", "recordings")

env.AddCustomTarget(
    name="serve",
    dependencies="$BUILD_DIR/${PROGNAME}.bin",
    actions=['"$PYTHONEXE" "%s" "$BUILD_DIR/${PROGNAME}.bin"' % dev_server],
    title="Serve firmware",
    description="Build and serve the firmware for a dev update over Wi-Fi",
)

env.AddCustomTarget(
    name="record",
    dependencies=None,
    actions=['"$PYTHONEXE" "%s" --record "%s"' % (dev_server, recordings)],
    title="Receive recordings",
    description="Receive recordings of the load cell from the scale over Wi-Fi",
)
