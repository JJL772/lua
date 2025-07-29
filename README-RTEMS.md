# Lua with RTEMS Support

This fork includes support for RTEMS in Lua. It has been tested on i386 and PowerPC hardware.

## Language Changes

* Support for `!=` as an alias for `~=`
* Octal number literals with `0o` prefix. Ex: `0o777`

# API Additions

## os

### `os.truncate(file, size)`

Same functionality as `truncate(2)`

### `os.creat(file, mode)`

Same functionality as `creat(2)`

### `os.mkdir(dir, mode)`

Same functionality as `mkdir(2)`

### `os.chown(file, uid, gid)`

Same functionality as `chown(2)`

### `os.chmod(file, mode)`

Same functionality as `chmod(2)`

## rtems

This library encapsulates all RTEMS-related APIs

### `rtems.shell(cmd, ...)`

Execute a command in the RTEMS shell. All arguments after `cmd` are optional.

#### Examples:

```lua
rtems.shell("echo", "hello, world!")
```

### `rtems.shell_script(script, ...)`

Execute an RTEMS shell script with the provided argumetns. All arguments after `cmd` are optional.

### `rtems.build_name(a, b, c, d)`

Builds an RTEMS name from the provided chars.

### `rtems.build_id(a, b, c, d)`

Builds an RTEMS ID from the provided chars.

### `rtems.reset()`

Performs a system reset.

# API Removals

## os

### `os.exit`

Use `rtems.reset` instead.
