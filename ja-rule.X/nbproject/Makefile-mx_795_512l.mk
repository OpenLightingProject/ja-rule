#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-mx_795_512l.mk)" "nbproject/Makefile-local-mx_795_512l.mk"
include nbproject/Makefile-local-mx_795_512l.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=mx_795_512l
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon.c ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon_pic32mx.c ../../../../../../opt/microchip/harmony/v1_02/framework/system/ports/src/sys_ports.c ../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2/bsp_sys_init.c ../src/app.c ../src/system_config/mx_795_512l/framework/system/clk/src/sys_clk_static.c ../src/system_config/mx_795_512l/framework/system/ports/src/sys_ports_static.c ../src/system_config/mx_795_512l/system_init.c ../src/system_config/mx_795_512l/system_interrupt.c ../src/main.c ../src/system_config/mx_795_512l/system_tasks.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1655588004/sys_devcon.o ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o ${OBJECTDIR}/_ext/1311845467/sys_ports.o ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o ${OBJECTDIR}/_ext/1360937237/app.o ${OBJECTDIR}/_ext/923701816/sys_clk_static.o ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o ${OBJECTDIR}/_ext/2068394399/system_init.o ${OBJECTDIR}/_ext/2068394399/system_interrupt.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/2068394399/system_tasks.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1655588004/sys_devcon.o.d ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o.d ${OBJECTDIR}/_ext/1311845467/sys_ports.o.d ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o.d ${OBJECTDIR}/_ext/1360937237/app.o.d ${OBJECTDIR}/_ext/923701816/sys_clk_static.o.d ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o.d ${OBJECTDIR}/_ext/2068394399/system_init.o.d ${OBJECTDIR}/_ext/2068394399/system_interrupt.o.d ${OBJECTDIR}/_ext/1360937237/main.o.d ${OBJECTDIR}/_ext/2068394399/system_tasks.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1655588004/sys_devcon.o ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o ${OBJECTDIR}/_ext/1311845467/sys_ports.o ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o ${OBJECTDIR}/_ext/1360937237/app.o ${OBJECTDIR}/_ext/923701816/sys_clk_static.o ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o ${OBJECTDIR}/_ext/2068394399/system_init.o ${OBJECTDIR}/_ext/2068394399/system_interrupt.o ${OBJECTDIR}/_ext/1360937237/main.o ${OBJECTDIR}/_ext/2068394399/system_tasks.o

# Source Files
SOURCEFILES=../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon.c ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon_pic32mx.c ../../../../../../opt/microchip/harmony/v1_02/framework/system/ports/src/sys_ports.c ../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2/bsp_sys_init.c ../src/app.c ../src/system_config/mx_795_512l/framework/system/clk/src/sys_clk_static.c ../src/system_config/mx_795_512l/framework/system/ports/src/sys_ports_static.c ../src/system_config/mx_795_512l/system_init.c ../src/system_config/mx_795_512l/system_interrupt.c ../src/main.c ../src/system_config/mx_795_512l/system_tasks.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-mx_795_512l.mk dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX795F512L
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1655588004/sys_devcon.o: ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1655588004 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon.o.d 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1655588004/sys_devcon.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1655588004/sys_devcon.o.d" -o ${OBJECTDIR}/_ext/1655588004/sys_devcon.o ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon.c   
	
${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o: ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon_pic32mx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1655588004 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o.d 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o.d" -o ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon_pic32mx.c   
	
${OBJECTDIR}/_ext/1311845467/sys_ports.o: ../../../../../../opt/microchip/harmony/v1_02/framework/system/ports/src/sys_ports.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1311845467 
	@${RM} ${OBJECTDIR}/_ext/1311845467/sys_ports.o.d 
	@${RM} ${OBJECTDIR}/_ext/1311845467/sys_ports.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1311845467/sys_ports.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1311845467/sys_ports.o.d" -o ${OBJECTDIR}/_ext/1311845467/sys_ports.o ../../../../../../opt/microchip/harmony/v1_02/framework/system/ports/src/sys_ports.c   
	
${OBJECTDIR}/_ext/82266220/bsp_sys_init.o: ../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2/bsp_sys_init.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/82266220 
	@${RM} ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/82266220/bsp_sys_init.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/82266220/bsp_sys_init.o.d" -o ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o ../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2/bsp_sys_init.c   
	
${OBJECTDIR}/_ext/1360937237/app.o: ../src/app.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/app.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/app.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/app.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1360937237/app.o.d" -o ${OBJECTDIR}/_ext/1360937237/app.o ../src/app.c   
	
${OBJECTDIR}/_ext/923701816/sys_clk_static.o: ../src/system_config/mx_795_512l/framework/system/clk/src/sys_clk_static.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/923701816 
	@${RM} ${OBJECTDIR}/_ext/923701816/sys_clk_static.o.d 
	@${RM} ${OBJECTDIR}/_ext/923701816/sys_clk_static.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/923701816/sys_clk_static.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/923701816/sys_clk_static.o.d" -o ${OBJECTDIR}/_ext/923701816/sys_clk_static.o ../src/system_config/mx_795_512l/framework/system/clk/src/sys_clk_static.c   
	
${OBJECTDIR}/_ext/1421599976/sys_ports_static.o: ../src/system_config/mx_795_512l/framework/system/ports/src/sys_ports_static.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1421599976 
	@${RM} ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o.d 
	@${RM} ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1421599976/sys_ports_static.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1421599976/sys_ports_static.o.d" -o ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o ../src/system_config/mx_795_512l/framework/system/ports/src/sys_ports_static.c   
	
${OBJECTDIR}/_ext/2068394399/system_init.o: ../src/system_config/mx_795_512l/system_init.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2068394399 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_init.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2068394399/system_init.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/2068394399/system_init.o.d" -o ${OBJECTDIR}/_ext/2068394399/system_init.o ../src/system_config/mx_795_512l/system_init.c   
	
${OBJECTDIR}/_ext/2068394399/system_interrupt.o: ../src/system_config/mx_795_512l/system_interrupt.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2068394399 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_interrupt.o.d 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_interrupt.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2068394399/system_interrupt.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/2068394399/system_interrupt.o.d" -o ${OBJECTDIR}/_ext/2068394399/system_interrupt.o ../src/system_config/mx_795_512l/system_interrupt.c   
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c   
	
${OBJECTDIR}/_ext/2068394399/system_tasks.o: ../src/system_config/mx_795_512l/system_tasks.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2068394399 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_tasks.o.d 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_tasks.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2068394399/system_tasks.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/2068394399/system_tasks.o.d" -o ${OBJECTDIR}/_ext/2068394399/system_tasks.o ../src/system_config/mx_795_512l/system_tasks.c   
	
else
${OBJECTDIR}/_ext/1655588004/sys_devcon.o: ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1655588004 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon.o.d 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1655588004/sys_devcon.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1655588004/sys_devcon.o.d" -o ${OBJECTDIR}/_ext/1655588004/sys_devcon.o ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon.c   
	
${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o: ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon_pic32mx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1655588004 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o.d 
	@${RM} ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o.d" -o ${OBJECTDIR}/_ext/1655588004/sys_devcon_pic32mx.o ../../../../../../opt/microchip/harmony/v1_02/framework/system/devcon/src/sys_devcon_pic32mx.c   
	
${OBJECTDIR}/_ext/1311845467/sys_ports.o: ../../../../../../opt/microchip/harmony/v1_02/framework/system/ports/src/sys_ports.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1311845467 
	@${RM} ${OBJECTDIR}/_ext/1311845467/sys_ports.o.d 
	@${RM} ${OBJECTDIR}/_ext/1311845467/sys_ports.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1311845467/sys_ports.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1311845467/sys_ports.o.d" -o ${OBJECTDIR}/_ext/1311845467/sys_ports.o ../../../../../../opt/microchip/harmony/v1_02/framework/system/ports/src/sys_ports.c   
	
${OBJECTDIR}/_ext/82266220/bsp_sys_init.o: ../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2/bsp_sys_init.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/82266220 
	@${RM} ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/82266220/bsp_sys_init.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/82266220/bsp_sys_init.o.d" -o ${OBJECTDIR}/_ext/82266220/bsp_sys_init.o ../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2/bsp_sys_init.c   
	
${OBJECTDIR}/_ext/1360937237/app.o: ../src/app.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/app.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/app.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/app.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1360937237/app.o.d" -o ${OBJECTDIR}/_ext/1360937237/app.o ../src/app.c   
	
${OBJECTDIR}/_ext/923701816/sys_clk_static.o: ../src/system_config/mx_795_512l/framework/system/clk/src/sys_clk_static.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/923701816 
	@${RM} ${OBJECTDIR}/_ext/923701816/sys_clk_static.o.d 
	@${RM} ${OBJECTDIR}/_ext/923701816/sys_clk_static.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/923701816/sys_clk_static.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/923701816/sys_clk_static.o.d" -o ${OBJECTDIR}/_ext/923701816/sys_clk_static.o ../src/system_config/mx_795_512l/framework/system/clk/src/sys_clk_static.c   
	
${OBJECTDIR}/_ext/1421599976/sys_ports_static.o: ../src/system_config/mx_795_512l/framework/system/ports/src/sys_ports_static.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1421599976 
	@${RM} ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o.d 
	@${RM} ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1421599976/sys_ports_static.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1421599976/sys_ports_static.o.d" -o ${OBJECTDIR}/_ext/1421599976/sys_ports_static.o ../src/system_config/mx_795_512l/framework/system/ports/src/sys_ports_static.c   
	
${OBJECTDIR}/_ext/2068394399/system_init.o: ../src/system_config/mx_795_512l/system_init.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2068394399 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_init.o.d 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_init.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2068394399/system_init.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/2068394399/system_init.o.d" -o ${OBJECTDIR}/_ext/2068394399/system_init.o ../src/system_config/mx_795_512l/system_init.c   
	
${OBJECTDIR}/_ext/2068394399/system_interrupt.o: ../src/system_config/mx_795_512l/system_interrupt.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2068394399 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_interrupt.o.d 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_interrupt.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2068394399/system_interrupt.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/2068394399/system_interrupt.o.d" -o ${OBJECTDIR}/_ext/2068394399/system_interrupt.o ../src/system_config/mx_795_512l/system_interrupt.c   
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d" -o ${OBJECTDIR}/_ext/1360937237/main.o ../src/main.c   
	
${OBJECTDIR}/_ext/2068394399/system_tasks.o: ../src/system_config/mx_795_512l/system_tasks.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2068394399 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_tasks.o.d 
	@${RM} ${OBJECTDIR}/_ext/2068394399/system_tasks.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2068394399/system_tasks.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -O1 -I"../src" -I"../src/system_config/mx_795_512l" -I"../../../../../../opt/microchip/harmony/v1_02/framework" -I"../../../../../../opt/microchip/harmony/v1_02/bsp/pic32mx_eth_sk2" -MMD -MF "${OBJECTDIR}/_ext/2068394399/system_tasks.o.d" -o ${OBJECTDIR}/_ext/2068394399/system_tasks.o ../src/system_config/mx_795_512l/system_tasks.c   
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../../../../../../opt/microchip/harmony/v1_02/bin/framework/peripheral/PIC32MX795F512L_peripherals.a  
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mdebugger -D__MPLAB_DEBUGGER_PK3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}    ../../../../../../opt/microchip/harmony/v1_02/bin/framework/peripheral/PIC32MX795F512L_peripherals.a       -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC02000:0x1FC02FEF -mreserve=boot@0x1FC02000:0x1FC024FF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,--defsym=_min_heap_size=0,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../../../../../../opt/microchip/harmony/v1_02/bin/framework/peripheral/PIC32MX795F512L_peripherals.a 
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}    ../../../../../../opt/microchip/harmony/v1_02/bin/framework/peripheral/PIC32MX795F512L_peripherals.a      -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=0,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"
	${MP_CC_DIR}/xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/ja-rule.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/mx_795_512l
	${RM} -r dist/mx_795_512l

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
