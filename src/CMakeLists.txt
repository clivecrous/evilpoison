# --- Edit the below definitions to customize your compile ---

# Uncomment to compile in certain text messages like help.  You want this too
# unless you *really* want to trim the bytes.
# Note that snprintf(3) is always part of the build.
add_definitions(-DSTDIO)

# === You should not be editing anything below this line ===

add_definitions(-DVERSION='"${VERSION}"')

find_package( X11 )

add_executable(evilpoison
  bind.c client.c command.c commandline.c dictionary.c
  events.c evilpoison_commands.c ewmh.c
  main.c misc.c new.c screen.c settings.c)

target_link_libraries(evilpoison ${X11_LIBRARIES} pthread)
