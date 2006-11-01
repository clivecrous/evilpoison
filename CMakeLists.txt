project(evilpoison,c)

set(VERSION 0.1)

find_package(X11 REQUIRED)

# --- Edit the below definitions to customize your compile ---

# Uncomment to enable solid window drags.  This can be slow on old systems.
add_definitions(-DSOLIDDRAG)

# Enable a more informative and clear banner to be displayed on Ctrl+Alt+I.
add_definitions(-DINFOBANNER)

# Uncomment to show the same banner on moves and resizes.  Note this can
# make things very SLOW!
# add_definitions(-DINFOBANNER_MOVERESIZE)

# To support virtual desktops, uncomment the following line.
add_definitions(-DVWM)

# To support shaped windows properly, uncomment the following two lines:
add_definitions(-DSHAPE)

# Uncomment for mouse support.  You probably want this.
add_definitions(-DMOUSE)

# Uncomment for snap-to-border support (thanks, Neil Drumm)
# Start evilpoison with -snap num to enable (num is proximity in pixels to snap to)
add_definitions(-DSNAP)

# Uncomment to compile in certain text messages like help.  You want this too
# unless you *really* want to trim the bytes.
# Note that snprintf(3) is always part of the build.
add_definitions(-DSTDIO)

# You can save a few bytes if you know you won't need colour map support
# (e.g., for 16 or more bit displays)
add_definitions(-DCOLOURMAP)

# === You should not be editing anything below this line ===

add_definitions(-DVERSION='"${VERSION}"')

add_executable(evilpoison
  client.c  events.c  ewmh.c  main.c  misc.c  new.c  screen.c)

target_link_libraries(evilpoison ${X11_LIBRARIES})
