# include custom makefile
-include Makefile.custom

# general
FOLDER:=$(notdir $(shell pwd))
PROJECT_NAME?=${FOLDER}

CXX=g++
LD=g++
DEP=g++
WL_RP=-Wl,-rpath=

# files
SOURCES=$(wildcard src/*.cpp)
SOURCES:=$(filter-out $(wildcard src/*_moc.cpp),${SOURCES})
OBJS=$(patsubst src/%.cpp,obj/%${VERSION}.o,${SOURCES})
ifeq "${OBJS}" ""
	LIB_EMPTY_INDICATOR="(empty)"
endif

APP_SOURCES=$(wildcard application/*.cpp)
APPS_NO_PATH=$(patsubst application/%.cpp,%${VERSION},${APP_SOURCES})
APPS=$(patsubst application/%.cpp,bin/%${VERSION},${APP_SOURCES})
#APPS=$(patsubst %.cpp,%${VERSION},${APP_SOURCES})

DEP_FILES:=$(patsubst src/%.cpp,obj/%.src.dep,${SOURCES})
APP_DEP_FILES:=$(patsubst application/%.cpp,obj/%.app.dep,${APP_SOURCES})

DEP_FILES:=${DEP_FILES} ${APP_DEP_FILES}

# avoid single space in dep-files string!
ifeq "${DEP_FILES}" " "
DEP_FILES=
endif

all:

LIB=${PROJECT_NAME}${VERSION}
LIB_FILE=lib${LIB}.so
LIB_LNK=-Llib -l${LIB}
PC=$(patsubst %,%.pc,${LIB})


# basic flags
CXXFLAGS:=${CXXFLAGS} -Isrc -Iapplication -fPIC -Wall
#CXXCPP=
#LDFLAGS=


# extract quit flag
ifeq "${QUIET}" "TRUE"
AT=@
else
AT=
endif

ifeq "${COLORED}" "TRUE"
FBLA='\033[30m'
FRED='\033[31m'
FGRE='\033[32m'
FYEL='\033[33m'
FBLU='\033[34m'
FPUR='\033[35m'
FCYA='\033[36m'

BBLA='\033[40m'
BRED='\033[41m'
BGRE='\033[42m'
BYEL='\033[43m'
BBLU='\033[44m'
BPUR='\033[45m'
BCYA='\033[46m'

BBLI='\033[5m'
INV='\033[7m'
DEF='\033[0m'
endif

# other helpers
LIB_INSTALL_PREFIX=${INSTALL_PREFIX}/lib

ifeq "${VERSION}" "" 
INC_INSTALL_PREFIX=${INSTALL_PREFIX}/include/${PROJECT_NAME}
DOC_INSTALL_PREFIX=${INSTALL_PREFIX}/doc/${PROJECT_NAME}
else
INC_INSTALL_PREFIX=${INSTALL_PREFIX}/include/${PROJECT_NAME}/${VERSION}
DOC_INSTALL_PREFIX=${INSTALL_PREFIX}/doc/${PROJECT_NAME}/${VERSION}
endif
REL_INC_INSTALL_PREFIX=$(patsubst ${INSTALL_PREFIX}/%,%,${INC_INSTALL_PREFIX})


ECHO=@echo -e
ECHO_NO_AT=echo -e
INSTALL=${AT}install

# save flags from Makefile.custom
LDFLAGS_PC:=${LDFLAGS} -L${LIB_INSTALL_PREFIX} -l${LIB} -Wl,-rpath=${LIB_INSTALL_PREFIX}
CXXFLAGS_PC:=$(filter-out -Isrc -Iapplication -fPIC -Wall,${CXXFLAGS})
CXXCPP_PC:=${CXXCPP} -I${INC_INSTALL_PREFIX}

# parsing pkg-config files
ifneq "${PC_DEPS}" ""
CXXFLAGS:=${CXXFLAGS} $(shell pkg-config --cflags-only-I ${PC_DEPS})
CXXCPP:=${CXXCPP} $(shell pkg-config --cflags-only-other ${PC_DEPS})
#LDFLAGS:=$(shell pkg-config --libs-only-L ${PC_DEPS}) ${LDFLAGS} 
LDFLAGS:=$(patsubst -L%,${WL_RP}%,$(shell pkg-config --libs-only-L ${PC_DEPS})) ${LDFLAGS} 
LDFLAGS:=$(shell pkg-config --libs ${PC_DEPS}) ${LDFLAGS}
endif

LIB_LNK:=${LIB_LNK} -Wl,-rpath=${LIB_INSTALL_PREFIX}

# checking for debug flag
ifeq "${DEBUG}" "TRUE"
CXXFLAGS:=${CXXFLAGS} -g -O0
DEBUG_INDICATOR=${FRED}"(debug)"${DEF}
else
CXXFLAGS:=${CXXFLAGS} -O4 -march=native -mtune=native -funroll-loops
DEBUG_INDICATOR=${FRED}"(optimized)"${DEF}
endif

# check for moc headers and search for moc command
ifneq "${MOC_HEADERS}" ""
MOC_OBJS=$(patsubst %.h,obj/%${VERSION}_moc.o,${MOC_HEADERS})
MOC_SOURCES=$(patsubst %.h,src/%${VERSION}_moc.cpp,${MOC_HEADERS})
MOC=$(shell pkg-config --variable=moc_location QtCore)
OBJS:=${MOC_OBJS} ${OBJS}
endif


ifeq "${SHOW_PROGRESS}" "TRUE"

ifndef PROGRESS
# works for up to 400 targets 
twenty := x x x x x x x x x x x x x x x x x x x x
MAX := $(foreach x,$(twenty),$(twenty))

# T estimates how many targets we are building by replacing BUILD with a special string
T := $(shell $(MAKE) NO_RECURSION=TRUE -nrRf $(firstword $(MAKEFILE_LIST)) $(MAKECMDGOALS) \
            PROGRESS="@echo COUNTTHIS > /dev/null" | grep -c "echo COUNTTHIS > /dev/null")

# N is the number of pending targets in base 1, well in fact, base x :-)
N := $(wordlist 1,$T,$(MAX)) 

# auto-decrementing counter that returns the number of pending targets in base 10
counter = $(words $N)$(eval N := $(wordlist 2,$(words $N),$N))

# BUILD is now defined to show the progress, this also avoids redefining T in loop
PROGRESS = ${INV}'['`echo "100-100*(${counter}-1)/(${T}+1)" | bc -l | cut -d '.' -f 1 | sed "s|^[0-9][0-9]$$|_&|g" | sed "s|^[0-9]$$|__&|g"`'%]'${DEF}
FIRST_RUN=FALSE
endif

endif


# actually we want to build doc/documentation
.PHONY: doc ${PC}

all: ${MOC_SOURCES} lib/${LIB_FILE} ${APPS} ${PKG_CFG} custom-build-hook

# overwrite build rules with these from dependency file
# if obj/dependencies is not found a submake is spawned
# to create it (make rock's)


ifeq "${FIRST_RUN}" "FALSE"
ifneq "${MAKECMDGOALS}" "cleaner"

-include obj/dependencies

endif
endif

ifneq "${SHOW_PROGRESS}" "TRUE"

-include obj/dependencies

endif


custom-install-hook ::
	${ECHO} ${PROGRESS} ${FCYA}${BBLA}[install]${DEF} custom install

custom-build-hook ::
	${ECHO} ${PROGRESS} ${FCYA}${BBLA}[build]${DEF} custom build

custom-clean-hook ::
	${ECHO} ${PROGRESS} ${FCYA}${BBLA}[clean]${DEF} custom clean


obj/dependencies: ${RECURSIVE_ICL_PROJECTS} ${DEP_FILES} 
	${AT}if [ "${DEP_FILES}" != "" ] ; then cat ${DEP_FILES} > $@ ; else touch $@ ; fi

src/%${VERSION}_moc.cpp: src/%.h
	${ECHO} ${PROGRESS} ${BBLU}[moc]${DEF} $@
	${AT}${MOC} $< -o $@

obj/%${VERSION}_moc.o: src/%${VERSION}_moc.cpp 
	${ECHO} ${PROGRESS} ${BCYA}[obj]${DEF} $@ ${DEBUG_INDICATOR}
	${AT}${CXX} -c ${CXXFLAGS} ${CXXCPP} $< -o $@


obj/%${VERSION}.o: src/%.cpp 
	${ECHO} ${PROGRESS} ${BCYA}[obj]${DEF} $@ ${DEBUG_INDICATOR}
	${AT}${CXX} -c ${CXXFLAGS} ${CXXCPP} $< -o $@


bin/%${VERSION}: application/%.cpp lib/${LIB_FILE}
	${ECHO} ${PROGRESS} ${BGRE}[app]${DEF} $@
	${AT}${CXX} $< ${CXXFLAGS} ${CXXCPP} ${LIB_LNK} ${LDFLAGS} -o $@


lib/${LIB_FILE}: ${OBJS} 
	${ECHO} ${PROGRESS} ${BRED}[lib]${DEF} $@ ${FRED}${LIB_EMPTY_INDICATOR}${DEF}
	${AT}${LD} -shared ${OBJS} ${LDFLAGS}  -o $@

obj/%.src.dep: src/%.cpp 
	${ECHO} ${PROGRESS} ${BPUR}[dep]${DEF} $@
	${AT}${DEP} ${CXXFLAGS} ${CXXCPP} -MT `echo $< | sed "s|src/|obj/|g" | sed "s|.cpp|${VERSION}.o|g"` $< -MM -o ${@}_tmp && mv ${@}_tmp $@

obj/%.app.dep: application/%.cpp
	${ECHO} ${PROGRESS} ${BPUR}[dep]${DEF} $@
	${AT}${DEP} ${CXXFLAGS} ${CXXCPP} -MT `echo $< | sed "s|.cpp|${VERSION}|g"` $< -MM -o ${@}_tmp && mv ${@}_tmp $@

doc:
	${AT}(cd doc && doxygen 2>&1 | while read LINE ; do ${ECHO_NO_AT} ${BPUR}[doxygen]${DEF} $${LINE} ; done)

clean: custom-clean-hook
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} objects
	${AT}rm -rf obj/*.o
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} moc files
	${AT}rm -rf ${MOC_SOURCES}
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} apps
	${AT}rm -rf ${APPS}
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} ${LIB_FILE}
	${AT}rm -rf lib/${LIB_FILE}
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} ${PC}
	${AT}rm -rf ${PC}

cleaner: clean
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} temporary files
	${AT}rm -rf *~ src/*~ application/*~
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} dependency files
	${AT}rm -rf obj/*.dep obj/dependencies
	${ECHO} ${PROGRESS} ${FBLA}${BYEL}[delete]${DEF} documentation
	${AT}rm -rf doc/documentation


${PC}:
	${ECHO} ${PROGRESS} ${BYEL}[create]${DEF} $@
	${AT}rm -rf $@
	${ECHO} prefix=${INSTALL_PREFIX} >> $@
	${ECHO} exec_prefix=$$\{prefix\} >> $@
	${ECHO} bindir=$$\{prefix\}/bin >> $@
	${ECHO} libdir=$$\{prefix\}/lib >> $@
	${ECHO} datadir$$\{prefix\}/data >> $@
	${ECHO} includedir=$$\{prefix\}${REL_INC_INSTALL_PREFIX} >> $@
	${ECHO} installation=${MAKECMDGOALS} >> $@
	${ECHO} package=${LIB} >> $@
	${ECHO} >> $@
	${ECHO} Name: ${LIB} >> $@
	${ECHO} Description: ${PACKAGE_DESCRIPTION} >> $@
	${ECHO} Version: ${VERSION} >> $@
	${ECHO} >> $@
	${ECHO} Requires: ${PC_DEPS} >> $@
	${ECHO} >> $@
	${ECHO} Libs: ${LDFLAGS_PC} >> $@
	${ECHO} >> $@
	${ECHO} Cflags: ${CXXFLAGS_PC} ${CXXCPP_PC} >> $@


create-install-dirs:
	${AT}for D in ${INSTALL_PREFIX} \
		      ${INC_INSTALL_PREFIX} \
		      ${DOC_INSTALL_PREFIX} \
		      ${LIB_INSTALL_PREFIX}/pkgconfig \
	              ${INSTALL_PREFIX}/bin \
		      ${INSTALL_PREFIX}/doc \
		      ${INSTALL_PREFIX}/doc/html ; do \
			if [ ! -d $$D ] ; then \
				mkdir -m 775 -p $$D ; \
			fi ; \
	 done


installlink: all ${PC} uninstall create-install-dirs custom-install-hook
	${AT}for T in `find src/ -name *.h` ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[link]${DEF} $$T ; \
		ln -s `pwd`/$$T ${INC_INSTALL_PREFIX}/ ; \
	done
	${AT}for T in ${APPS} ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[link]${DEF} $$T ; \
		ln -s `pwd`/$$T ${INSTALL_PREFIX}/bin ; 	\
	done
	${ECHO} ${PROGRESS} ${BBLU}[link]${DEF} ${LIB_FILE}
	${AT}ln -s `pwd`/lib/${LIB_FILE} ${LIB_INSTALL_PREFIX}
	${ECHO} ${PROGRESS} ${BBLU}[link]${DEF} ${PC} 
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/pkgconfig/${PC}
	${AT}ln -s `pwd`/${PC} ${LIB_INSTALL_PREFIX}/pkgconfig/
	${AT}if [ -d doc/documentation ] ; then \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[link]${DEF} documentation ; \
		for T in doc/documentation/html/* ; do \
			ln -s `pwd`/$$T ${DOC_INSTALL_PREFIX}/ ; \
		done ; \
	else \
		${ECHO_NO_AT} ${PROGRESS} ${BBLU}[skip]${DEF} documentation ;	\
	fi

install: all ${PC} create-install-dirs custom-install-hook
	${AT}for T in `find src/ -name *.h` ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BYEL}[install]${DEF} $$T ; \
		install -D -p -m 664 `pwd`/$$T ${INC_INSTALL_PREFIX}/ ; \
	done
	${AT}for T in ${APPS} ; do \
		${ECHO_NO_AT} ${PROGRESS} ${BYEL}[install]${DEF} $$T ; \
		install -D -p -m 775 `pwd`/$$T ${INSTALL_PREFIX}/bin ; 	\
	done
	${ECHO} ${PROGRESS} ${BYEL}[install]${DEF} ${LIB_FILE}
	${AT}install -D -p -m 664 `pwd`/lib/${LIB_FILE} ${LIB_INSTALL_PREFIX}
	${ECHO} ${PROGRESS} ${BYEL}[install]${DEF} ${PC} 
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/pkgconfig/${PC}
	${AT}install -D -p -m 664 `pwd`/${PC} ${LIB_INSTALL_PREFIX}/pkgconfig/
	${AT}if [ -d doc/documentation ] ; then \
		${ECHO_NO_AT} ${PROGRESS} ${BYEL}[install]${DEF} documentation ; \
		for T in doc/documentation/html/* ; do \
			install -D -p -m 664 `pwd`/$$T ${DOC_INSTALL_PREFIX} ; \
		done ; \
	else \
		${ECHO_NO_AT} ${PROGRESS} ${BYEL}[skip]${DEF} documentation ;	\
	fi

uninstall:
	${ECHO} ${PROGRESS} ${BRED}[uninstall]${DEF} ${PFX}
	${AT}rm -rf ${INC_INSTALL_PREFIX}
	${AT}for T in ${APPS_NO_PATH} ; do \
		rm -rf ${INSTALL_PREFIX}/bin/$${T} ; \
	done
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/${LIB_FILE}
	${AT}rm -rf ${LIB_INSTALL_PREFIX}/pkgconfig/${PC}
	${AT}rm -rf ${DOC_INSTALL_PREFIX}



$(call NOTIFY_PACKAGE,IPP,         )
$(call NOTIFY_PACKAGE,QT,          )
$(call NOTIFY_PACKAGE,LIBDC,       )
$(call NOTIFY_PACKAGE,UNICAP,      )
$(call NOTIFY_PACKAGE,LIBZ,        )
$(call NOTIFY_PACKAGE,LIBJPEG,     )
$(call NOTIFY_PACKAGE,SVS,         )
$(call NOTIFY_PACKAGE,IMAGEMAGICK, )
$(call NOTIFY_PACKAGE,XCF,         )

info:
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}${BBLI}ICL-Configuration${DEF}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}ICL${DEF}"         " `pkg-config --variable=prefix icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| IPP         " `pkg-config --variable=IPP_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| QT          " `pkg-config --variable=QT_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| LIBDC       " `pkg-config --variable=LIBDC_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| UNICAP      " `pkg-config --variable=UNICAP_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| LIBZ        " `pkg-config --variable=LIBZ_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| LIBJPEG     " `pkg-config --variable=LIBJPEG_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| SVS         " `pkg-config --variable=SVS_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| IMAGEMAGICK " `pkg-config --variable=IMAGEMAGICK_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| XCF         " `pkg-config --variable=XCF_ROOT icl`
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| Flags"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}DEBUG${DEF}"       " ${DEBUG}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}LDFLAGS${DEF}"     " ${LDFLAGS}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}CXXFLAGS${DEF}"    " ${CXXFLAGS}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "| "${BRED}CXXCPP${DEF}"      " ${CXXCPP}
	${ECHO}	${FPUR}${BBLA}[info]${DEF} "+----------------------------------------------"
