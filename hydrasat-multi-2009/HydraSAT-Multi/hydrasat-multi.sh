#!/bin/bash

# How to run this script: 
#
# DIR/hydrasat-multi.sh DIR BENCHNAME MEMLIMIT RANDOMSEED
# 
#    Path and filenames DIR and BENCHNAME are assumed to contain no spaces.
#

DIR=${1}              # Path to the hydrasat binary, same as solver directory
BENCHNAME=${2}
MEMLIMIT=${3}         # In MiB
RANDOMSEED=${4}
SOLVER1=${DIR}/bin/hydrasat_cache4_flat
SOLVER2=${DIR}/bin/hydrasat_cachewatchedprop_celements4_rev525
SOLVER3=${DIR}/bin/hydrasat_cache2_flat

##############################################################################

MEMLIMIT_HYDRA=$(( ( ${MEMLIMIT} * 9 ) / 10 ))

# Options for the different solver-configurations
#
OPTIONS1="-rs ${RANDOMSEED} -co MemoryLimit ${MEMLIMIT_HYDRA}"
OPTIONS2="-rs ${RANDOMSEED} -co MemoryLimit ${MEMLIMIT_HYDRA} -co component_unit_propagation CacheWatchedPropagation"
OPTIONS3="-rs ${RANDOMSEED} -co MemoryLimit ${MEMLIMIT_HYDRA}"
 
##############################################################################

# Timeouts
#
TIMEOUT1=700
TIMEOUT2=800

# Used as soft ulimit for the solvers.
# Reserve something (5MiB) for the script.
#
MEM_ULIMIT_SOLVERS=$(( (${MEMLIMIT} - 5) * 1024 ))  ## In kbytes

##############################################################################
# Starting Hydrasat in first configuration OPTIONS1
# with the first timeout TIMEOUT1

ulimit -Sv $MEM_ULIMIT_SOLVERS
ulimit -St $TIMEOUT1 

echo c Trying configuration 1 >&2
$SOLVER1 $BENCHNAME $OPTIONS1
EXIT_STATUS=$? 

# Check wether the exit status = 10 (SAT) or the exit status  = 20 (UNSAT).
# In any one of these two cases we will exit from the script.
#
if (( ${EXIT_STATUS} == 20 || ${EXIT_STATUS} == 10 ))
then
    exit $EXIT_STATUS
fi

##############################################################################
# ... second configuration
ulimit -St $TIMEOUT2

echo c Trying configuration 2 >&2
$SOLVER2 $BENCHNAME $OPTIONS2
EXIT_STATUS=$? 

# Check wether the exit status = 10 (SAT) or the exit status  = 20 (UNSAT).
# In any one of these two cases we will exit from the script.
#
if (( ${EXIT_STATUS} == 20 || ${EXIT_STATUS} == 10 ))
then
    exit $EXIT_STATUS
fi

##############################################################################
# Final configuration with OPTIONS3
#
# 
# Just the SOFT ulimit is set here to unlimited, such that, if hard limits 
# are present, the calls to ulimit succeed with setting to these hard limits.
#
ulimit -St unlimited
ulimit -Sv unlimited
echo c Trying configuration 3 >&2
$SOLVER3 $BENCHNAME $OPTIONS3
