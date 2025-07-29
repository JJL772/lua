
#ifndef __rtems__
#error This file should only be compiled on RTEMS
#endif

#include "lprefix.h"

#include <rtems.h>
#include <bsp.h>
#include <rtems/libi2c.h>
#include <rtems/libio.h>
#include <rtems/shell.h>

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"

static int lua_rtems_build_name(lua_State* L) {
  int a = luaL_checkinteger(L, 1);
  int b = luaL_checkinteger(L, 2);
  int c = luaL_checkinteger(L, 3);
  int d = luaL_checkinteger(L, 4);
  lua_pushinteger(L, rtems_build_name(a, b, c, d));
  return 1;
}

static int lua_rtems_build_id(lua_State* L) {
  int a = luaL_checkinteger(L, 1);
  int b = luaL_checkinteger(L, 2);
  int c = luaL_checkinteger(L, 3);
  int d = luaL_checkinteger(L, 4);
  lua_pushinteger(L, rtems_build_id(a, b, c, d));
  return 1;
}

static int lua_rtems_task_self(lua_State* L) {
  lua_pushinteger(L, rtems_task_self());
  return 1;
}

static int lua_rtems_event_send(lua_State* L) {
  rtems_id task = luaL_checkinteger(L, 1);
  uint32_t set = luaL_checkinteger(L, 2);
  lua_pushinteger(L,rtems_event_send(task, set));
  return 1;
}

static int lua_rtems_event_recv(lua_State* L) {
  uint32_t set = luaL_checkinteger(L, 1);
  uint32_t opts = luaL_checkinteger(L, 2);
  uint32_t ticks = luaL_checkinteger(L, 3);

  rtems_option a;
  rtems_event_set out;
  if (rtems_event_receive(set, opts, ticks, &out) != RTEMS_SUCCESSFUL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushinteger(L, out);
  return 1;
}

static int lua_rtems_shell_exec(lua_State* L) {
  const char* cmd = luaL_checkstring(L, 1);
  if (!cmd) return 0;

  const char* args[33] = {0};
  args[0] = cmd;

  int i;
  for (i = 2; i < 32; ++i) {
    if (lua_type(L, i) == LUA_TNONE)
      break;
    args[i-1] = luaL_checkstring(L, i);
  }

  rtems_shell_cmd_t* pcmd = rtems_shell_lookup_cmd(cmd);
  int r = pcmd ? pcmd->command(i-1, (char**)args) : -1;
  lua_pushinteger(L, r);
  return 1;
}

static int lua_rtems_shell_script(lua_State* L) {
  const char* file = luaL_checkstring(L, 1);
  if (!file) return 0;

  const char* args[33] = {0};
  args[0] = file;

  int i;
  for (i = 2; i < 32; ++i) {
    if (lua_type(L, i) == LUA_TNONE)
      break;
    args[i-1] = luaL_checkstring(L, i);
  }

  int r = rtems_shell_script_file(i-1, (char**)args);
  lua_pushinteger(L, r);
  return 1;
}

/**
 * rtems.wr8
 * \param addr Starting address
 * \param value 8-bit value to write
 * \param num Number of elements to write
 */
static int lua_rtems_wr8(lua_State* L) {
  lua_Integer addr = luaL_checkinteger(L, 1);
  lua_Integer value = luaL_checkinteger(L, 2);
  int isnum = 0;
  lua_Integer num = lua_tointegerx(L, 3, &isnum);
  if (!isnum)
    num = 1;

  uint8_t* p = (uint8_t*)(uintptr_t)addr;
  for (int i = 0; i < num; ++i)
    *(p++) = value & 0xFF;

  return 0;
}

/**
 * rtems.wr16
 * \param addr Starting address
 * \param value 16-bit value to write
 * \param num Number of elements to write (NOT BYTES)
 */
static int lua_rtems_wr16(lua_State* L) {
  lua_Integer addr = luaL_checkinteger(L, 1);
  lua_Integer value = luaL_checkinteger(L, 2);
  int isnum = 0;
  lua_Integer num = lua_tointegerx(L, 3, &isnum);
  if (!isnum)
    num = 1;

  uint16_t* p = (uint16_t*)(uintptr_t)addr;
  for (int i = 0; i < num; ++i)
    *(p++) = value & 0xFFFF;

  return 0;
}

/**
 * rtems.wr32
 * \param addr Starting address
 * \param value 32-bit value to write
 * \param num Number of elements to write (NOT BYTES)
 */
static int lua_rtems_wr32(lua_State* L) {
  lua_Integer addr = luaL_checkinteger(L, 1);
  lua_Integer value = luaL_checkinteger(L, 2);
  int isnum = 0;
  lua_Integer num = lua_tointegerx(L, 3, &isnum);
  if (!isnum)
    num = 1;

  uint32_t* p = (uint32_t*)(uintptr_t)addr;
  for (int i = 0; i < num; ++i)
    *(p++) = value & 0xFFFFFFFF;

  return 0;
}

/***************** rtems_semaphore *****************/

static int lua_rtems_create_sem(lua_State* L) {
  rtems_name name = luaL_checkinteger(L, 1);
  uint32_t count = luaL_checkinteger(L, 2);
  uint32_t attrs = luaL_checkinteger(L, 3);
  uint32_t prio = luaL_checkinteger(L, 4);

  rtems_id id;
  if (rtems_semaphore_create(name, count, attrs, prio, &id) != RTEMS_SUCCESSFUL) {
    lua_pushnil(L);
    return 1;
  }

  rtems_id* pid = lua_newuserdata(L, sizeof(rtems_id));
  *pid = id;

  luaL_getmetatable(L, "rtems_semaphore");
  lua_setmetatable(L, -2);
  return 1;
}

static int lua_rtems_sem_release(lua_State* L) {
  rtems_id* id = lua_touserdata(L, 1);
  lua_pushinteger(L, rtems_semaphore_release(*id));
  return 1;
}

static int lua_rtems_sem_obtain(lua_State* L) {
  rtems_id* id = lua_touserdata(L, 1);
  uint32_t opts = luaL_checkinteger(L, 2);
  uint32_t timeout = luaL_checkinteger(L, 3);
  lua_pushinteger(L, rtems_semaphore_obtain(*id, opts, timeout));
  return 1;
}

static int lua_rtems_sem_delete(lua_State* L){ 
  rtems_id* id = lua_touserdata(L, 1);
  rtems_semaphore_delete(*id);
  return 0;
}

static const luaL_Reg sem_meta[] = {
  {"__gc",        lua_rtems_sem_delete},
  {"obtain",      lua_rtems_sem_obtain},
  {"release",     lua_rtems_sem_release},
  {NULL, NULL},
};


/***************** rtems_message_queue *****************/

static int lua_rtems_mqueue_create(lua_State* L) {
  rtems_id name = luaL_checkinteger(L, 1);
  uint32_t count = luaL_checkinteger(L, 2);
  uint32_t sz = luaL_checkinteger(L, 3);
  uint32_t attrs = luaL_checkinteger(L, 4);
  
  rtems_id ident;
  if (rtems_message_queue_create(name, count, sz, attrs, &ident) != RTEMS_SUCCESSFUL) {
    lua_pushnil(L);
    return 1;
  }
  
  rtems_id* id = lua_newuserdata(L, sizeof(rtems_id));
  *id = ident;
  
  luaL_getmetatable(L, "rtems_message_queue");
  lua_setmetatable(L, -2);
  return 1;
}

static int lua_rtems_mqueue_destroy(lua_State* L) {
  rtems_id id = *(rtems_id*)lua_touserdata(L, 1);
  rtems_message_queue_delete(id);
  return 0;
}

static int lua_rtems_mqueue_send(lua_State* L) {
  rtems_id id = *(rtems_id*)lua_touserdata(L, 1);
  lua_Number nv = 0;
  const char* sv = NULL;
  uint8_t bval = 0;
  const void* v = NULL;
  size_t sz = 0;

  switch(lua_type(L, 2)) {
  case LUA_TNUMBER:
    nv = luaL_checkinteger(L, 2); 
    v = &nv; sz = sizeof(nv);
    break;
  case LUA_TBOOLEAN:
    bval = luaL_checkinteger(L, 2);
    v = &bval; sz = sizeof(bval);
    break;
  case LUA_TSTRING:
    sv = luaL_checkstring(L, 2);
    v = sv; sz = sizeof(sv);
    break;
  case LUA_TTABLE:
    return luaL_error(L, "Unsupported type");
  }
  lua_pushinteger(L, rtems_message_queue_send(id, v, sz));
  return 1;
}

static const luaL_Reg message_queue_meta[] = {
  {"__gc",        lua_rtems_mqueue_destroy},
  {"send",        lua_rtems_mqueue_send},
  {NULL, NULL},
};

#define SET_GLOBAL_ENUM(_x) do { \
  lua_pushinteger(L, _x); \
  lua_setglobal(L, #_x); \
  } while(0)

static void lua_rtems_register_globals(lua_State* L) {
  /* Misc globals */
  SET_GLOBAL_ENUM(RTEMS_LOCAL);
  SET_GLOBAL_ENUM(RTEMS_GLOBAL);
  SET_GLOBAL_ENUM(RTEMS_NO_TIMEOUT);
  SET_GLOBAL_ENUM(RTEMS_EVENT_ANY);
  SET_GLOBAL_ENUM(RTEMS_EVENT_ALL);
  SET_GLOBAL_ENUM(RTEMS_BINARY_SEMAPHORE);
  SET_GLOBAL_ENUM(RTEMS_COUNTING_SEMAPHORE);
  SET_GLOBAL_ENUM(RTEMS_COUNTING_SEMAPHORE);
  SET_GLOBAL_ENUM(RTEMS_FIFO);
  SET_GLOBAL_ENUM(RTEMS_PRIORITY);
  SET_GLOBAL_ENUM(RTEMS_PRIORITY_CEILING);
  SET_GLOBAL_ENUM(RTEMS_NO_PRIORITY_CEILING);
  SET_GLOBAL_ENUM(RTEMS_INHERIT_PRIORITY);
  SET_GLOBAL_ENUM(RTEMS_NO_INHERIT_PRIORITY);
  SET_GLOBAL_ENUM(RTEMS_SIMPLE_BINARY_SEMAPHORE);
  
  /* FS types */
  SET_GLOBAL_ENUM(RTEMS_FILESYSTEM_TYPE_DOSFS);
  SET_GLOBAL_ENUM(RTEMS_FILESYSTEM_TYPE_FTPFS);
  SET_GLOBAL_ENUM(RTEMS_FILESYSTEM_TYPE_TFTPFS);
  SET_GLOBAL_ENUM(RTEMS_FILESYSTEM_TYPE_IMFS);
  SET_GLOBAL_ENUM(RTEMS_FILESYSTEM_TYPE_JFFS2);
  SET_GLOBAL_ENUM(RTEMS_FILESYSTEM_TYPE_NFS);
  SET_GLOBAL_ENUM(RTEMS_FILESYSTEM_TYPE_RFS);

  /* Register event enums */
  for (uint32_t i = 0; i < 32; ++i) {
    lua_pushinteger(L, 1<<i);
    char evname[32];
    snprintf(evname, sizeof(evname), "RTEMS_EVENT_%d", i);
    lua_setglobal(L, evname);
  }
}

static void register_object(lua_State* L, const char* meta, const luaL_Reg* regs) {
  luaL_newmetatable(L, meta);
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2); /* push table */
  lua_settable(L, -3);  /* set __index */
  luaL_setfuncs(L, regs, 0);
}

static int lua_rtems_init(lua_State* L) {
  lua_rtems_register_globals(L);

  register_object(L, "rtems_message_queue", message_queue_meta);
  register_object(L, "rtems_semaphore", sem_meta);

  return 1;
}

static const luaL_Reg rtemslib[] = {
  {"build_name",          lua_rtems_build_name},
  {"build_id",            lua_rtems_build_id},
  {"task_self",           lua_rtems_task_self},
  {"event_send",          lua_rtems_event_send},
  {"event_recv",          lua_rtems_event_recv},
  {"create_semaphore",    lua_rtems_create_sem},
  {"create_message_queue", lua_rtems_mqueue_create},
  {"shell",               lua_rtems_shell_exec},
  {"shell_script",        lua_rtems_shell_script},
  {"wr8",                 lua_rtems_wr8},
  {"wr16",                lua_rtems_wr16},
  {"wr32",                lua_rtems_wr32},
  {NULL, NULL}
};

LUAMOD_API int luaopen_rtems(lua_State *L) {
  lua_rtems_init(L);
  luaL_newlib(L, rtemslib);
  return 1;
}
