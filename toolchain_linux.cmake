SET( CMAKE_SYSTEM_NAME Linux)
SET( TOOLCHAIN Linux)
SET( LIBRARY_PREFIX lib)
SET( LIBRARY_SUFFIX .so)
message(STATUS "Toolchain Ubuntu")


# set cmake toolchains 
SET( CMAKE_MAKE_PROGRAM /usr/bin/make    CACHE PATH "Linux Make Program" )
SET( CMAKE_SH           /bin/sh      CACHE PATH "Linux shell Program" )
SET( CMAKE_AR           /usr/bin/ar      CACHE PATH "Linux ar Program" )
SET( CMAKE_RANLIB       /usr/bin/ranlib      CACHE PATH "Linux ranlib Program" )
SET( CMAKE_NM           /usr/bin/nm      CACHE PATH "Linux nm Program" )
SET( CMAKE_OBJCOPY      /usr/bin/objcopy CACHE PATH "Linux objcopy Program" )
SET( CMAKE_OBJDUMP      /usr/bin/objdump CACHE PATH "Linux objdump Program" )
SET( CMAKE_LINKER       /usr/bin/ld     CACHE PATH "Linux Linker Program" )
SET( CMAKE_STRIP        /usr/bin/strip   CACHE PATH "Linux Strip Program" )
SET( CMAKE_C_COMPILER  /usr/bin/gcc )
SET( CMAKE_CXX_COMPILER  /usr/bin/g++)

SET( CMAKE_FIND_ROOT_PATH /usr/bin)
SET( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
SET( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
SET( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )


add_definitions("-DLINUX")
add_definitions("-DKANZI_V8_API=")
add_definitions("-DKZ_SUPPORT_GRAPHICS_API_OPENGL=1")
add_definitions("-DKZ_RUNTIME_GRAPHICS_BACKEND=1")
add_definitions("-DKZ_DLOAD_GLES_SYMBOLS=32")
add_definitions("-DNODEBUG")
add_definitions("-DKANZI_FEATURE_3D")
add_definitions("-DPNG_NO_MMX_CODE")
add_definitions("-DX11")
add_definitions("-DGLX")
add_definitions("-DKZ_SUPPORT_GRAPHICS_CONTEXT_API_GLX=1")
add_definitions("-DKZ_DLOAD_GL_SYMBOLS=13")

set(CMAKE_CXX_FLAGS "-fexceptions -O2 -fPIC -Wall -Wextra -Wno-overlength-strings -fno-strict-aliasing -Wno-maybe-uninitialized -Wno-long-long -pthread")
