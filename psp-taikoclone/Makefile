TARGET = test
OBJS = src/test.o \
	   src/load_texture.o \
	   src/drawing.o \
	   src/tjaparser.o \
	   src/ui/ui_utils.o \
	   src/ui/song_select_ui.o \
	   src/song_select.o \
	   src/helper/fat.o \
	   src/helper/MiniConv.o \
	   src/helper/mem64.o \
	   src/helper/opendir.o \
	   src/note.o \
	   src/animation.o \
	   src/textures.o \
	   src/frame.o \

YOURLIBS = 
BUILD_PRX = 1

# aalib stuff
# #####################33
OBJS_AALIB = src/aalib/pspaalib.o \
			 src/aalib/pspaalibat3.o \
			 src/aalib/pspaalibeffects.o \
			 src/aalib/pspaalibogg.o \
			 src/aalib/pspaalibscemp3.o \
			 src/aalib/pspaalibwav.o
LIBS_AALIB = -lpspaudio -lpspmp3 -lvorbisidec -lpspatrac3
OBJS += $(OBJS_AALIB)

#######################################

# helper stuff
# ###############################
OBJS_HELPER = src/helper/dictionary.o \
			  src/helper/iniparser.o \
			  src/helper/metadata_parser.o \
			  src/helper/mem64.o \
			  src/helper/fat.o \
			  src/helper/opendir.o
OBJS += $(OBJS_HELPER)

INCDIR = 
CFLAGS = -G0 -Wall -O2 -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
CPPFLAGS = -DTEST_ANIMATION
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
STDLIBS = -losl -ljpeg -lpng -lz \
		  -lpspsdk -lpspctrl -lpspumd -lpsprtc -lpsppower -lpspgu -lpspaudiolib -lpspaudio -lm 

LIBS = $(STDLIBS)$(YOURLIBS)$(LIBS_AALIB)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Taiko no shirouto
PSP_EBOOT_ICON = test.png

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
