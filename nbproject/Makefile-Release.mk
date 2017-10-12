#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/get_config.o \
	${OBJECTDIR}/logger.o \
	${OBJECTDIR}/mobilegt_TunDataProcess.o \
	${OBJECTDIR}/mobilegt_TunReceiver.o \
	${OBJECTDIR}/mobilegt_TunnelConnectChecker.o \
	${OBJECTDIR}/mobilegt_TunnelDataProcess.o \
	${OBJECTDIR}/mobilegt_TunnelReceiver.o \
	${OBJECTDIR}/mobilegt_util.o \
	${OBJECTDIR}/mobilegt_vpnserver.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f1

# Test Object Files
TESTOBJECTFILES= \
	${TESTDIR}/tests/get_config_simpletest.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mobilegt_vpn

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mobilegt_vpn: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mobilegt_vpn ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/get_config.o: get_config.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/get_config.o get_config.cpp

${OBJECTDIR}/logger.o: logger.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/logger.o logger.cpp

${OBJECTDIR}/mobilegt_TunDataProcess.o: mobilegt_TunDataProcess.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunDataProcess.o mobilegt_TunDataProcess.cpp

${OBJECTDIR}/mobilegt_TunReceiver.o: mobilegt_TunReceiver.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunReceiver.o mobilegt_TunReceiver.cpp

${OBJECTDIR}/mobilegt_TunnelConnectChecker.o: mobilegt_TunnelConnectChecker.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunnelConnectChecker.o mobilegt_TunnelConnectChecker.cpp

${OBJECTDIR}/mobilegt_TunnelDataProcess.o: mobilegt_TunnelDataProcess.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunnelDataProcess.o mobilegt_TunnelDataProcess.cpp

${OBJECTDIR}/mobilegt_TunnelReceiver.o: mobilegt_TunnelReceiver.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunnelReceiver.o mobilegt_TunnelReceiver.cpp

${OBJECTDIR}/mobilegt_util.o: mobilegt_util.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_util.o mobilegt_util.cpp

${OBJECTDIR}/mobilegt_vpnserver.o: mobilegt_vpnserver.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_vpnserver.o mobilegt_vpnserver.cpp

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-tests-subprojects .build-conf ${TESTFILES}
.build-tests-subprojects:

${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/get_config_simpletest.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS}   


${TESTDIR}/tests/get_config_simpletest.o: tests/get_config_simpletest.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/get_config_simpletest.o tests/get_config_simpletest.cpp


${OBJECTDIR}/get_config_nomain.o: ${OBJECTDIR}/get_config.o get_config.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/get_config.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/get_config_nomain.o get_config.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/get_config.o ${OBJECTDIR}/get_config_nomain.o;\
	fi

${OBJECTDIR}/logger_nomain.o: ${OBJECTDIR}/logger.o logger.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/logger.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/logger_nomain.o logger.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/logger.o ${OBJECTDIR}/logger_nomain.o;\
	fi

${OBJECTDIR}/mobilegt_TunDataProcess_nomain.o: ${OBJECTDIR}/mobilegt_TunDataProcess.o mobilegt_TunDataProcess.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mobilegt_TunDataProcess.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunDataProcess_nomain.o mobilegt_TunDataProcess.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mobilegt_TunDataProcess.o ${OBJECTDIR}/mobilegt_TunDataProcess_nomain.o;\
	fi

${OBJECTDIR}/mobilegt_TunReceiver_nomain.o: ${OBJECTDIR}/mobilegt_TunReceiver.o mobilegt_TunReceiver.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mobilegt_TunReceiver.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunReceiver_nomain.o mobilegt_TunReceiver.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mobilegt_TunReceiver.o ${OBJECTDIR}/mobilegt_TunReceiver_nomain.o;\
	fi

${OBJECTDIR}/mobilegt_TunnelConnectChecker_nomain.o: ${OBJECTDIR}/mobilegt_TunnelConnectChecker.o mobilegt_TunnelConnectChecker.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mobilegt_TunnelConnectChecker.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunnelConnectChecker_nomain.o mobilegt_TunnelConnectChecker.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mobilegt_TunnelConnectChecker.o ${OBJECTDIR}/mobilegt_TunnelConnectChecker_nomain.o;\
	fi

${OBJECTDIR}/mobilegt_TunnelDataProcess_nomain.o: ${OBJECTDIR}/mobilegt_TunnelDataProcess.o mobilegt_TunnelDataProcess.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mobilegt_TunnelDataProcess.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunnelDataProcess_nomain.o mobilegt_TunnelDataProcess.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mobilegt_TunnelDataProcess.o ${OBJECTDIR}/mobilegt_TunnelDataProcess_nomain.o;\
	fi

${OBJECTDIR}/mobilegt_TunnelReceiver_nomain.o: ${OBJECTDIR}/mobilegt_TunnelReceiver.o mobilegt_TunnelReceiver.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mobilegt_TunnelReceiver.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_TunnelReceiver_nomain.o mobilegt_TunnelReceiver.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mobilegt_TunnelReceiver.o ${OBJECTDIR}/mobilegt_TunnelReceiver_nomain.o;\
	fi

${OBJECTDIR}/mobilegt_util_nomain.o: ${OBJECTDIR}/mobilegt_util.o mobilegt_util.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mobilegt_util.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_util_nomain.o mobilegt_util.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mobilegt_util.o ${OBJECTDIR}/mobilegt_util_nomain.o;\
	fi

${OBJECTDIR}/mobilegt_vpnserver_nomain.o: ${OBJECTDIR}/mobilegt_vpnserver.o mobilegt_vpnserver.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/mobilegt_vpnserver.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/mobilegt_vpnserver_nomain.o mobilegt_vpnserver.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/mobilegt_vpnserver.o ${OBJECTDIR}/mobilegt_vpnserver_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f1 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
