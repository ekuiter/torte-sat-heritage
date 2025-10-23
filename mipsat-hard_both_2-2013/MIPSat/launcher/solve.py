#!/usr/bin/python


# imports
# -----------------------------------------------------------------------------
import os               # path and process management
import resource         # process resources
import shutil           # copy files and directories
import signal           # process management
import sys              # argv, exit
import time             # time mgmt

import systools         # IPC process management
import timetools        # IPC timing management


# -----------------------------------------------------------------------------

# globals
# -----------------------------------------------------------------------------

CHECK_INTERVAL = 5           # how often we query the process group status
KILL_DELAY = 5               # how long we wait between SIGTERM and SIGKILL


# -----------------------------------------------------------------------------

# funcs
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# set_limit
#
# sets 'amount' as the maximum allowed capacity of the resource 'kind'
# -----------------------------------------------------------------------------
def set_limit(kind, amount):
    """
    sets 'amount' as the maximum allowed capacity of the resource 'kind'
    """

    try:
        resource.setrlimit(kind, (amount, amount))
    except OSError, e:
        print ("c %s in 'set_limit'" % e)


# -----------------------------------------------------------------------------
# kill_pgrp
#
# sends the signal sig to the process group pgrp
# -----------------------------------------------------------------------------
def kill_pgrp(pgrp, sig):
    """
    sends the signal sig to the process group pgrp
    """

    try:
        os.killpg(pgrp, sig)
    except OSError:
        pass


# -----------------------------------------------------------------------------
# run
#
# Time is measured in seconds and memory in bytes
#
# -----------------------------------------------------------------------------
def run (script, problem, randomseed, output, timeout, memory, solver_log_file, solver_err_file):

    # create a timer
    runtimer = timetools.Timer ()

    # Now, a child is created which will host the planner execution while this
    # process simply monitors the resource comsumption. If any is exceeded the
    # whole process group is killed
    with runtimer:

        child_pid = os.fork()
        if not child_pid:                                            # child's code
            os.setpgrp()
            set_limit(resource.RLIMIT_CPU, timeout)
            set_limit(resource.RLIMIT_AS, memory)
            set_limit(resource.RLIMIT_CORE, 0)
            if not output:
                for fd_no, filename in [(1, solver_log_file), (2, solver_err_file)]:
                    os.close(fd_no)
                    fd = os.open(filename, os.O_CREAT | os.O_TRUNC | os.O_WRONLY, 0666)
                    assert fd == fd_no, fd

            os.execl(script, script, problem, randomseed)

        real_time = 0
        while True:
            time.sleep(CHECK_INTERVAL)
            real_time += CHECK_INTERVAL

            group = systools.ProcessGroup(child_pid)

            # Generate the children information before the waitpid call to avoid a
            # race condition. This way, we know that the child_pid is a descendant.
            if os.waitpid(child_pid, os.WNOHANG) != (0, 0):
                break

            # get the total time and memory usage
            process_time = real_time
            total_time = group.total_time()

            # if multicore ain't enabled, the usual rules apply
            try_term = (total_time >= timeout or
                        real_time >= 1.5 * timeout)
            try_kill = (total_time >= timeout + KILL_DELAY or
                        real_time >= 1.5 * timeout + KILL_DELAY)

            term_attempted = False
            if try_term and not term_attempted:
                print ("c aborting children with SIGTERM...")
                print ("c children found: %s" % group.pids())
                kill_pgrp(child_pid, signal.SIGTERM)
                term_attempted = True
            elif term_attempted and try_kill:
                print ("c aborting children with SIGKILL...")
                print ("c children found: %s" % group.pids())
                kill_pgrp(child_pid, signal.SIGKILL)

        # Even if we got here, there may be orphaned children or something we may
        # have missed due to a race condition. Check for that and kill.
        group = systools.ProcessGroup(child_pid)
        if group:
            # If we have reason to suspect someone still lives, first try to kill
            # them nicely and wait a bit.
            print ("c aborting orphaned children with SIGTERM...")
            print ("c children found: %s" % group.pids())
            kill_pgrp(child_pid, signal.SIGTERM)
            time.sleep(1)

        # Either way, kill properly for good measure. Note that it's not clear if
        # checking the ProcessGroup for emptiness is reliable, because reading the
        # process table may not be atomic, so for this last blow, we don't do an
        # emptiness test.
        kill_pgrp(child_pid, signal.SIGKILL)


# -----------------------------------------------------------------------------
# check log
#
# Time is measured in seconds and memory in bytes
#
# -----------------------------------------------------------------------------
def check_log(solver_log_file):

    solved = False
    file = open (solver_log_file, 'r').readlines ()
    for iline in file:
        line = iline [:-1]

        if not solved:

            if line[0:9] == "s UNKNOWN":
                return False

            elif line[0] == "s" or line[0] == "v":
                solved = True
                print line

        else:
            print line

    return solved

# main
# -----------------------------------------------------------------------------
if __name__ == '__main__':

    timeout1 = 982
    timeout2 = 4013
    memory   = 8053063680

    # Check params
    if len(sys.argv) == 5 and sys.argv[3] == "-tmp":
        if os.path.isfile(sys.argv[1]):
            problem = os.path.abspath(sys.argv[1])
            randomseed = sys.argv[2]
            tmpdir = sys.argv[4]
        else:
            print ("The bench file does not exist: %s" % sys.argv[1])
            exit ()
    else:
        print "Usage: ./solve.py INSTANCE RANDOMSEED -tmp TEMPDIR"
        exit ()

    # Getting root path
    pathname = os.path.dirname(sys.argv[0])
    currentpath = os.path.abspath(pathname)
    rootpath = os.path.abspath(os.path.join(currentpath,".."))
    abstmpdir = os.path.abspath(tmpdir)
    solver_log_file = abstmpdir + "/solver.log"
    solver_err_file = abstmpdir + "/solver.err"

    # Configuring solvers path
    solver1 = rootpath + "/solver1/solve"
    solver2 = rootpath + "/solver2/solve"

    # Run the first solver
    run (solver1, problem, randomseed, False, timeout1, memory, solver_log_file, solver_err_file)

    # Check the output
    solved = check_log (solver_log_file)

    # Remove temporal files
    os.remove (solver_log_file)
    os.remove (solver_err_file)

    # If solver1 does not solve the problem, run the second solver
    if not solved:
        run (solver2, problem, randomseed, True, timeout2, memory, solver_log_file, solver_err_file)


# Local Variables:
# mode:python
# fill-column:80
# End:
