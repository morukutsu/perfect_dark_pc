cmake_minimum_required(VERSION 3.10)

# Tmp: fixup for working on MacOS
#set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-12")
#set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-12")

project(perfect_dark)

add_compile_definitions(ROMID=ntsc-final)
add_compile_definitions(PAL=0)
add_compile_definitions(VERSION=2)
add_compile_definitions(_LANGUAGE_C)
add_compile_definitions(ROM_SIZE=32)
add_compile_definitions(_MIPS_SZLONG=64)
add_compile_definitions(PIRACYCHECKS=0)
add_compile_definitions(AVOID_UB)

# MacOS x Clang: adding this to avoid the definition of the function "random" in stdlib.h
add_compile_definitions(_ANSI_SOURCE)

# PC Port options
add_compile_definitions(PC_PORT_AUDIO_ENABLED=0)

enable_language(C CXX)
set (CMAKE_CXX_STANDARD 11)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})

set(PDARK_SRC_DIR "../src")
set(PDARK_LIB_FILES 
    "${PDARK_SRC_DIR}/lib/ailist.c"
    "${PDARK_SRC_DIR}/lib/anim.c"
    "${PDARK_SRC_DIR}/lib/args.c"
    "${PDARK_SRC_DIR}/lib/audiodma.c"
    "${PDARK_SRC_DIR}/lib/audiomgr.c"
    "${PDARK_SRC_DIR}/lib/base.c"
    "${PDARK_SRC_DIR}/lib/boot.c"
    "${PDARK_SRC_DIR}/lib/collision.c"
    "${PDARK_SRC_DIR}/lib/crash.c"
    "${PDARK_SRC_DIR}/lib/debughud.c"
    "${PDARK_SRC_DIR}/lib/dma.c"
    "${PDARK_SRC_DIR}/lib/fault.c"
    "${PDARK_SRC_DIR}/lib/joy.c"
    "${PDARK_SRC_DIR}/lib/lib_034d0.c"
    "${PDARK_SRC_DIR}/lib/lib_04f60nb.c"
    "${PDARK_SRC_DIR}/lib/lib_15850.c"
    "${PDARK_SRC_DIR}/lib/lib_17ce0.c"
    "${PDARK_SRC_DIR}/lib/lib_2f490_c.c"
    "${PDARK_SRC_DIR}/lib/lib_2fc60.c"
    "${PDARK_SRC_DIR}/lib/lib_39c80.c"
    "${PDARK_SRC_DIR}/lib/lib_3e3e0.c"
    # Original main file replaced by the PC one
    #"${PDARK_SRC_DIR}/lib/main.c"
    "${PDARK_SRC_DIR}/lib/mema.c"
    "${PDARK_SRC_DIR}/lib/memp.c"
    "${PDARK_SRC_DIR}/lib/model.c"
    "${PDARK_SRC_DIR}/lib/mp3.c"
    "${PDARK_SRC_DIR}/lib/mtx.c"
    "${PDARK_SRC_DIR}/lib/mtx_c.c"
    "${PDARK_SRC_DIR}/lib/music.c"
    "${PDARK_SRC_DIR}/lib/path.c"
    "${PDARK_SRC_DIR}/lib/pimgr.c"
    "${PDARK_SRC_DIR}/lib/profile.c"
    "${PDARK_SRC_DIR}/lib/rdp.c"
    "${PDARK_SRC_DIR}/lib/reset.c"
    "${PDARK_SRC_DIR}/lib/rmon.c"
    "${PDARK_SRC_DIR}/lib/rng_c.c"
    "${PDARK_SRC_DIR}/lib/sched.c"
    "${PDARK_SRC_DIR}/lib/snd.c"
    "${PDARK_SRC_DIR}/lib/speaker.c"
    "${PDARK_SRC_DIR}/lib/str.c"
    "${PDARK_SRC_DIR}/lib/varsinit.c"
    "${PDARK_SRC_DIR}/lib/vi.c"
    "${PDARK_SRC_DIR}/lib/videbug.c"
    "${PDARK_SRC_DIR}/lib/vm.c"
    "${PDARK_SRC_DIR}/lib/modelasm_c.c"


    "${PDARK_SRC_DIR}/lib/ultra/audio/bnkf.c"
    "${PDARK_SRC_DIR}/lib/ultra/audio/cents2ratio.c"
    "${PDARK_SRC_DIR}/lib/ultra/audio/cseq.c"
    "${PDARK_SRC_DIR}/lib/ultra/audio/heap.c"
    "${PDARK_SRC_DIR}/lib/ultra/audio/sl.c"

    "${PDARK_SRC_DIR}/lib/naudio/n_auxbus.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_cspgetstate.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_csplayer.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_cspplay.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_cspsendmidi.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_cspsetbank.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_cspsetseq.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_cspsetvol.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_csq.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_drvrNew.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_env.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_event.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_load.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_mainbus.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_resample.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_resample2.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_reverb.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_save.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_seqplayer.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_seqpstop.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_sl.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_sndplayer.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synaddplayer.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synallocfx.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synallocvoice.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_syndelete.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synfilter11.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synfilter12.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synfilter13.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synfreevoice.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synsetfxmix.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synsetfxparam.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synsetpan.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synsetpitch.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synsetpriority.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synsetvol.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synstartvoiceparam.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synstopvoice.c"
    "${PDARK_SRC_DIR}/lib/naudio/n_synthesizer.c"
)

set(PDARK_GAME_FILES
    "${PDARK_SRC_DIR}/textureconfig.c"
    "${PDARK_SRC_DIR}/game/acosasin.c"
    "${PDARK_SRC_DIR}/game/acosfasinf.c"
    "${PDARK_SRC_DIR}/game/activemenu.c"
    "${PDARK_SRC_DIR}/game/activemenutick.c"
    "${PDARK_SRC_DIR}/game/atan2f.c"
    "${PDARK_SRC_DIR}/game/bg.c"
    "${PDARK_SRC_DIR}/game/bgbss.c"
    "${PDARK_SRC_DIR}/game/body.c"
    "${PDARK_SRC_DIR}/game/bodyinit.c"
    "${PDARK_SRC_DIR}/game/bodyreset.c"
    "${PDARK_SRC_DIR}/game/bondbike.c"
    "${PDARK_SRC_DIR}/game/bondcutscene.c"
    "${PDARK_SRC_DIR}/game/bondeyespy.c"
    "${PDARK_SRC_DIR}/game/bondgrab.c"
    "${PDARK_SRC_DIR}/game/bondgun.c"
    "${PDARK_SRC_DIR}/game/bondgunreset.c"
    "${PDARK_SRC_DIR}/game/bondgunstop.c"
    "${PDARK_SRC_DIR}/game/bondhead.c"
    "${PDARK_SRC_DIR}/game/bondheadreset.c"
    "${PDARK_SRC_DIR}/game/bondmove.c"
    "${PDARK_SRC_DIR}/game/bondview.c"
    "${PDARK_SRC_DIR}/game/bondwalk.c"
    "${PDARK_SRC_DIR}/game/bossfile.c"
    "${PDARK_SRC_DIR}/game/bot.c"
    "${PDARK_SRC_DIR}/game/botact.c"
    "${PDARK_SRC_DIR}/game/botcmd.c"
    "${PDARK_SRC_DIR}/game/botinv.c"
    "${PDARK_SRC_DIR}/game/botinvinit.c"
    "${PDARK_SRC_DIR}/game/botmgr.c"
    "${PDARK_SRC_DIR}/game/botroom.c"
    "${PDARK_SRC_DIR}/game/buildtime.c"
    "${PDARK_SRC_DIR}/game/camdraw.c"
    "${PDARK_SRC_DIR}/game/camera.c"
    "${PDARK_SRC_DIR}/game/casingreset.c"
    "${PDARK_SRC_DIR}/game/casingtick.c"
    "${PDARK_SRC_DIR}/game/ceil.c"
    "${PDARK_SRC_DIR}/game/challenge.c"
    "${PDARK_SRC_DIR}/game/challengeinit.c"
    "${PDARK_SRC_DIR}/game/cheats.c"
    "${PDARK_SRC_DIR}/game/chr.c"
    "${PDARK_SRC_DIR}/game/chraction.c"
    "${PDARK_SRC_DIR}/game/chrai.c"
    "${PDARK_SRC_DIR}/game/chraicommands.c"
    "${PDARK_SRC_DIR}/game/chrmgr.c"
    "${PDARK_SRC_DIR}/game/chrmgrstop.c"
    "${PDARK_SRC_DIR}/game/collisionutils.c"
    "${PDARK_SRC_DIR}/game/crc.c"
    "${PDARK_SRC_DIR}/game/credits.c"
    "${PDARK_SRC_DIR}/game/debug1.c"
    "${PDARK_SRC_DIR}/game/debug2.c"
    "${PDARK_SRC_DIR}/game/debug3.c"
    "${PDARK_SRC_DIR}/game/dlights.c"
    "${PDARK_SRC_DIR}/game/dyntex.c"
    "${PDARK_SRC_DIR}/game/endscreen.c"
    "${PDARK_SRC_DIR}/game/env.c"
    "${PDARK_SRC_DIR}/game/explosions.c"
    "${PDARK_SRC_DIR}/game/explosionsreset.c"
    "${PDARK_SRC_DIR}/game/explosionsstop.c"
    "${PDARK_SRC_DIR}/game/file.c"
    "${PDARK_SRC_DIR}/game/filelist.c"
    "${PDARK_SRC_DIR}/game/filemgr.c"
    "${PDARK_SRC_DIR}/game/floor.c"
    "${PDARK_SRC_DIR}/game/fmb.c"
    "${PDARK_SRC_DIR}/game/footstep.c"
    "${PDARK_SRC_DIR}/game/gailists.c"
    "${PDARK_SRC_DIR}/game/game_006900.c"
    "${PDARK_SRC_DIR}/game/game_00b820.c"
    "${PDARK_SRC_DIR}/game/game_096360.c"
    "${PDARK_SRC_DIR}/game/game_096700.c"
    "${PDARK_SRC_DIR}/game/game_096b20.c"
    "${PDARK_SRC_DIR}/game/game_097a50.c"
    "${PDARK_SRC_DIR}/game/game_097aa0.c"
    "${PDARK_SRC_DIR}/game/game_0b0fd0.c"
    "${PDARK_SRC_DIR}/game/game_0b2150.c"
    "${PDARK_SRC_DIR}/game/game_13c510.c"
    "${PDARK_SRC_DIR}/game/game_152fa0.c"
    "${PDARK_SRC_DIR}/game/game_1531a0.c"
    "${PDARK_SRC_DIR}/game/game_175f90.c"
    "${PDARK_SRC_DIR}/game/game_176080.c"
    "${PDARK_SRC_DIR}/game/game_1a78b0.c"
    "${PDARK_SRC_DIR}/game/gamefile.c"
    "${PDARK_SRC_DIR}/game/getitle.c"
    "${PDARK_SRC_DIR}/game/getitlereset.c"
    "${PDARK_SRC_DIR}/game/gfxmemory.c"
    "${PDARK_SRC_DIR}/game/gfxreplace.c"
    "${PDARK_SRC_DIR}/game/gunfx.c"
    "${PDARK_SRC_DIR}/game/gunfxreset.c"
    "${PDARK_SRC_DIR}/game/healthbar.c"
    "${PDARK_SRC_DIR}/game/hudmsg.c"
    "${PDARK_SRC_DIR}/game/inv.c"
    "${PDARK_SRC_DIR}/game/invitems.c"
    "${PDARK_SRC_DIR}/game/invreset.c"
    "${PDARK_SRC_DIR}/game/lang.c"
    "${PDARK_SRC_DIR}/game/langinit.c"
    "${PDARK_SRC_DIR}/game/langreset.c"
    "${PDARK_SRC_DIR}/game/langtick.c"
    "${PDARK_SRC_DIR}/game/lv.c"
    "${PDARK_SRC_DIR}/game/mainmenu.c"
    "${PDARK_SRC_DIR}/game/menu.c"
    "${PDARK_SRC_DIR}/game/menugfx.c"
    "${PDARK_SRC_DIR}/game/menuitem.c"
    "${PDARK_SRC_DIR}/game/menustop.c"
    "${PDARK_SRC_DIR}/game/menutick.c"
    "${PDARK_SRC_DIR}/game/modeldef.c"
    "${PDARK_SRC_DIR}/game/modelmgr.c"
    "${PDARK_SRC_DIR}/game/modelmgrreset.c"
    "${PDARK_SRC_DIR}/game/mpstats.c"
    "${PDARK_SRC_DIR}/game/mtxf2lbulkasm_c.c"
    "${PDARK_SRC_DIR}/game/music.c"
    "${PDARK_SRC_DIR}/game/nbomb.c"
    "${PDARK_SRC_DIR}/game/objectives.c"
    "${PDARK_SRC_DIR}/game/objectivesreset.c"
    "${PDARK_SRC_DIR}/game/objectivesstop.c"
    "${PDARK_SRC_DIR}/game/options.c"
    "${PDARK_SRC_DIR}/game/pad.c"
    "${PDARK_SRC_DIR}/game/padhalllv.c"
    "${PDARK_SRC_DIR}/game/pak.c"
    "${PDARK_SRC_DIR}/game/pdmode.c"
    "${PDARK_SRC_DIR}/game/player.c"
    "${PDARK_SRC_DIR}/game/playermgr.c"
    "${PDARK_SRC_DIR}/game/playerreset.c"
    "${PDARK_SRC_DIR}/game/portal.c"
    "${PDARK_SRC_DIR}/game/portalconv_c.c"
    "${PDARK_SRC_DIR}/game/prop.c"
    "${PDARK_SRC_DIR}/game/propobj.c"
    "${PDARK_SRC_DIR}/game/propobjbss.c"
    "${PDARK_SRC_DIR}/game/propobjstop.c"
    "${PDARK_SRC_DIR}/game/propsnd.c"
    "${PDARK_SRC_DIR}/game/propsndreset.c"
    "${PDARK_SRC_DIR}/game/propsndstop.c"
    "${PDARK_SRC_DIR}/game/propsstop.c"
    "${PDARK_SRC_DIR}/game/proptick.c"
    "${PDARK_SRC_DIR}/game/quaternion.c"
    "${PDARK_SRC_DIR}/game/race.c"
    "${PDARK_SRC_DIR}/game/raceinit.c"
    "${PDARK_SRC_DIR}/game/radar.c"
    "${PDARK_SRC_DIR}/game/rng2_c.c"
    "${PDARK_SRC_DIR}/game/room.c"
    "${PDARK_SRC_DIR}/game/roomreset.c"
    "${PDARK_SRC_DIR}/game/roomtick.c"
    "${PDARK_SRC_DIR}/game/savebuffer.c"
    "${PDARK_SRC_DIR}/game/setup.c"
    "${PDARK_SRC_DIR}/game/setupcover.c"
    "${PDARK_SRC_DIR}/game/setuppads.c"
    "${PDARK_SRC_DIR}/game/setuputils.c"
    "${PDARK_SRC_DIR}/game/setupwaypoints.c"
    "${PDARK_SRC_DIR}/game/shards.c"
    "${PDARK_SRC_DIR}/game/shardsreset.c"
    "${PDARK_SRC_DIR}/game/shardsstop.c"
    "${PDARK_SRC_DIR}/game/shardstick.c"
    "${PDARK_SRC_DIR}/game/sight.c"
    "${PDARK_SRC_DIR}/game/sky.c"
    "${PDARK_SRC_DIR}/game/skyreset.c"
    "${PDARK_SRC_DIR}/game/skytick.c"
    "${PDARK_SRC_DIR}/game/smoke.c"
    "${PDARK_SRC_DIR}/game/smokeinit.c"
    "${PDARK_SRC_DIR}/game/smokereset.c"
    "${PDARK_SRC_DIR}/game/smokestop.c"
    "${PDARK_SRC_DIR}/game/sparks.c"
    "${PDARK_SRC_DIR}/game/sparksreset.c"
    "${PDARK_SRC_DIR}/game/sparkstick.c"
    "${PDARK_SRC_DIR}/game/splat.c"
    "${PDARK_SRC_DIR}/game/stagemusic.c"
    "${PDARK_SRC_DIR}/game/stagetable.c"
    "${PDARK_SRC_DIR}/game/stars.c"
    "${PDARK_SRC_DIR}/game/surfacereset.c"
    "${PDARK_SRC_DIR}/game/tex.c"
    "${PDARK_SRC_DIR}/game/texdecompress.c"
    "${PDARK_SRC_DIR}/game/texinit.c"
    "${PDARK_SRC_DIR}/game/texreset.c"
    "${PDARK_SRC_DIR}/game/texselect.c"
    "${PDARK_SRC_DIR}/game/tilesreset.c"
    "${PDARK_SRC_DIR}/game/timing.c"
    "${PDARK_SRC_DIR}/game/title.c"
    "${PDARK_SRC_DIR}/game/titleclear.c"
    "${PDARK_SRC_DIR}/game/titleinit.c"
    "${PDARK_SRC_DIR}/game/titlereset.c"
    "${PDARK_SRC_DIR}/game/training.c"
    "${PDARK_SRC_DIR}/game/trainingmenus.c"
    "${PDARK_SRC_DIR}/game/utils.c"
    "${PDARK_SRC_DIR}/game/varsreset.c"
    "${PDARK_SRC_DIR}/game/vmstats.c"
    "${PDARK_SRC_DIR}/game/vtxstore.c"
    "${PDARK_SRC_DIR}/game/vtxstorereset.c"
    "${PDARK_SRC_DIR}/game/wallhit.c"
    "${PDARK_SRC_DIR}/game/wallhitreset.c"
    "${PDARK_SRC_DIR}/game/weather.c"
    "${PDARK_SRC_DIR}/game/weatherreset.c"
    "${PDARK_SRC_DIR}/game/weathertick.c"
    "${PDARK_SRC_DIR}/game/modeldata/basic.c"
    "${PDARK_SRC_DIR}/game/modeldata/casing.c"
    "${PDARK_SRC_DIR}/game/modeldata/chr.c"
    "${PDARK_SRC_DIR}/game/modeldata/chrgun.c"
    "${PDARK_SRC_DIR}/game/modeldata/classicgun.c"
    "${PDARK_SRC_DIR}/game/modeldata/general.c"
    "${PDARK_SRC_DIR}/game/modeldata/player.c"
    "${PDARK_SRC_DIR}/game/modeldata/robot.c"
    "${PDARK_SRC_DIR}/game/modeldata/skedar.c"
    "${PDARK_SRC_DIR}/game/modeldata/skel06.c"
    "${PDARK_SRC_DIR}/game/modeldata/skel0a.c"
    "${PDARK_SRC_DIR}/game/modeldata/skel0b.c"
    "${PDARK_SRC_DIR}/game/modeldata/skel20.c"
    "${PDARK_SRC_DIR}/game/modeldata/uzi.c"
    "${PDARK_SRC_DIR}/game/modeldata/weapon.c"
    "${PDARK_SRC_DIR}/game/mplayer/ingame.c"
    "${PDARK_SRC_DIR}/game/mplayer/mpaicommands.c"
    "${PDARK_SRC_DIR}/game/mplayer/mplayer.c"
    "${PDARK_SRC_DIR}/game/mplayer/scenarios.c"
    "${PDARK_SRC_DIR}/game/mplayer/setup.c"
    "${PDARK_SRC_DIR}/game/stubs/game_000840.c"
    "${PDARK_SRC_DIR}/game/stubs/game_000850.c"
    "${PDARK_SRC_DIR}/game/stubs/game_000860.c"
    "${PDARK_SRC_DIR}/game/stubs/game_000870.c"
    "${PDARK_SRC_DIR}/game/stubs/game_0008e0.c"
    "${PDARK_SRC_DIR}/game/stubs/game_0008f0.c"
    "${PDARK_SRC_DIR}/game/stubs/game_000900.c"
    "${PDARK_SRC_DIR}/game/stubs/game_000910.c"
    "${PDARK_SRC_DIR}/game/stubs/game_00b180.c"
    "${PDARK_SRC_DIR}/game/stubs/game_00b200.c"
    "${PDARK_SRC_DIR}/game/stubs/game_013540.c"
    "${PDARK_SRC_DIR}/game/stubs/game_015260.c"
    "${PDARK_SRC_DIR}/game/stubs/game_015270.c"
    "${PDARK_SRC_DIR}/game/stubs/game_0153f0.c"
    "${PDARK_SRC_DIR}/game/stubs/game_015400.c"
    "${PDARK_SRC_DIR}/game/stubs/game_015410.c"
    "${PDARK_SRC_DIR}/game/stubs/game_102230.c"
    "${PDARK_SRC_DIR}/game/stubs/game_11eff0.c"
    "${PDARK_SRC_DIR}/game/stubs/game_175f50.c"
    "${PDARK_SRC_DIR}/game/stubs/game_176000.c"
)

set(PDARK_INFLATE_FILES 
    "${PDARK_SRC_DIR}/inflate/inflate.c"
)

set(PDARK_PC_ZIP_FILES 
    "zip/rarezip.c"
    "zip/gzip_bits.c"
    "zip/gzip_deflate.c"
    "zip/gzip_trees.c"
)

set(PDARK_PC_FILES 
    "main.c"
    "rom.c"
    "rzip.c"
    "ultra/ultra64.c"
    "ultra/gfx.c"
    "print.c"
    "byteswap.c"
    "offsetsmap.c"

    "gfx/gfxpc.c"
    "gfx/hashmap.c"
    "gfx/gbi_addon.c"
)

set(PDARK_ULTRA_LIB_FILES
    "${PDARK_SRC_DIR}/lib/ultra/gu/align.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/coss.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/frustum.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/lookat.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/lookatref.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/mtxutil.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/normalize.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/ortho.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/perspective.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/rotate.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/scale.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/sins.c"
    "${PDARK_SRC_DIR}/lib/ultra/gu/translate.c"

    "${PDARK_SRC_DIR}/lib/ultra/io/vitbl.c"
)

set(PDARK_IMGUI_FILES
    "imgui/imgui.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_widgets.cpp"
    "imgui/imgui_tables.cpp"
    "imgui/imgui_demo.cpp"
    "imgui/cimgui.cpp"
    "imgui/cimgui.h"
)

message(${CMAKE_SOURCE_DIR})
set(PDARK_PC_VERTEX_SHADER "shaders/vert.glsl")
set(PDARK_PC_FRAGMENT_SHADER "shaders/frag.glsl")

configure_file(${PDARK_PC_VERTEX_SHADER} shaders/vert.glsl COPYONLY)
configure_file(${PDARK_PC_FRAGMENT_SHADER} shaders/frag.glsl COPYONLY)

#add_compile_options(-fno-builtin -g)
add_compile_options(-g)

add_executable(perfect_dark 
    "${PDARK_PC_FILES}" 
    "${PDARK_LIB_FILES}"
    "${PDARK_GAME_FILES}"
    "${PDARK_INFLATE_FILES}"
    "${PDARK_ULTRA_LIB_FILES}"
    "${PDARK_IMGUI_FILES}"
)

target_link_libraries(perfect_dark m ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

target_include_directories(perfect_dark PUBLIC
                           "../src/include"
                            )
                            
target_include_directories(perfect_dark PUBLIC
                           "./ultra"
                            )

target_include_directories(perfect_dark PUBLIC
                           "../src/lib/ultra"
                           "../src/lib/ultra/audio"
                            )

target_include_directories(perfect_dark PUBLIC
                           "./imgui"
                            )

# Game source files are able to include some files from the PC port
target_include_directories(perfect_dark PUBLIC
                           "./"
                            )

# Ultra libs
target_include_directories(perfect_dark PUBLIC
                           "../include"
                           "../include/PR"
                           )
# TODO: use rom id here
target_include_directories(perfect_dark PUBLIC
                           "../src/generated/ntsc-final"
                            )

target_include_directories(perfect_dark PUBLIC
                           "../ld"
                            )