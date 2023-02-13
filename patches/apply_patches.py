from os.path import join, isfile

Import("env")

root_dir = env['PROJECT_DIR']
pio_env = env['PIOENV']
libdeps_dir = env['PROJECT_LIBDEPS_DIR']

patchflag_path = join(libdeps_dir,pio_env,"Arduino_BHY2Host", ".patching-done")

# patch file only if we didn't do it before
if not isfile(join(patchflag_path)):
    original_file = join(libdeps_dir,pio_env,"Arduino_BHY2Host","src", "Arduino_BHY2Host.h")
    patched_file = join(root_dir,"patches", "add-nano33iot-arduino-bhy2host.patch")

    assert isfile(original_file) and isfile(patched_file)

    env.Execute("patch %s %s" % (original_file, patched_file))

    def _touch(path):
        with open(path, "w") as fp:
            fp.write("")

    env.Execute(lambda *args, **kwargs: _touch(patchflag_path))