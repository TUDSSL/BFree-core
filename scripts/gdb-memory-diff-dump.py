import os
import sys
import re

import memdiff

LOG_FILE='/tmp/gdb.log'

MEM_DUMP_BASENAME = '_memory_dump_'

breakpoints = [
        'gdb_break_me', # after checkpoint (restore point)
        'HardFault_Handler',
        ]

memory_dump_ranges = {
        'stack':    [0x20007b90, 0x20007bf8],
        'data':     [0x20000000, 0x20000010],
        'bss':      [0x20000400, 0x200006c4],
        }

def user_reset():
    gdb.execute('mon reset')
    gdb.execute('c')

# Fake a power failure reset
def pf_reset():
    # Clear the special "safe_boot" flag, as if power was lost
    gdb.execute('call port_set_saved_word(0)')
    # Reset and continue
    user_reset()

def build_bin_filename(prefix, memname):
    fname = prefix + MEM_DUMP_BASENAME + memname + '.bin'
    return fname

def dump_memory(prefix):
    for memname in memory_dump_ranges:
        start_addr = str(memory_dump_ranges[memname][0])
        end_addr = str(memory_dump_ranges[memname][1])
        cmnd_str = 'dump binary memory ' \
            + build_bin_filename(prefix, memname) \
            + ' ' + start_addr + ' ' + end_addr
        print('Dumping memory: ' + cmnd_str)
        gdb.execute(cmnd_str)

def setup():
    print('Running GDB from: %s\n' %(gdb.PYTHONDIR))
    gdb.execute("set pagination off")
    gdb.execute("set print pretty")
    gdb.execute('set logging file %s'%(LOG_FILE))
    gdb.execute('set logging on')
    print('\nSetup complete !!\n')

def set_breakpoints():
    for bp in breakpoints:
        try:
            gdb.Breakpoint(bp)
            print('break ' + bp)
        except:
            print('Error inserting breakpoint ' + bp)
    print('\nDone setting breakpoints')

def get_sp():
    sp_str = gdb.execute('print $sp', to_string=True)
    sp = int(re.search('.*(0x.*)$', sp_str)[1], 16)
    #print('current SP: ' + hex(sp))
    return sp;

def breakpoint_handler(event):
    if not isinstance(event, gdb.BreakpointEvent):
        return

    print('EVENT: %s' % (event))
    location = event.breakpoint.location
    if location in breakpoints:
        #print('location: ' + location)
        if location == breakpoints[0]:
            is_restore_str = gdb.execute('print checkpoint_svc_restore', to_string=True)
            is_restore = int(re.search('.*=\s*(.*)$', is_restore_str)[1])
            if is_restore == 0:
                # Get the current stack pointer to dump the stack
                # Update it so the restore uses the same one
                sp = get_sp()
                print('current SP: ' + hex(sp))
                try:
                    # The stack pointer is the low address (it grows down)
                    memory_dump_ranges['stack'][0] = sp
                except KeyError:
                    pass

                dump_memory('after-cp')
                print('Registers array:')
                gdb.execute('p /x registers')
            else:
                dump_memory('after-restore')
                print('current SP: ' + hex(get_sp()))

                # print the memdiff
                for memname in memory_dump_ranges:
                    print('memdiff ' + memname)
                    diff = memdiff.main(build_bin_filename('after-cp', memname), \
                            build_bin_filename('after-restore', memname), \
                            memory_dump_ranges[memname][0])
                    if diff is not None:
                        for diff_addr in diff:
                            sym_cmd = 'info symbol ' + hex(diff_addr)
                            #print('Sumbol command: ' + sym_cmd)
                            gdb.execute(sym_cmd)

        elif location == breakpoints[1]:
            gdb.execute("bt")
            dump_memory('after-hardfault')
            sp = get_sp()
            pc_hardfault = sp + 6*4;

            print('current SP: ' + hex(sp))
            print('SP + 6*32: ' + hex(pc_hardfault))
            pc_hardfault_str = gdb.execute('x ' + hex(pc_hardfault), to_string=True)
            print('Hardfault Cause PC: ' + pc_hardfault_str)
            gdb.execute("info reg")
            print('Registers array:')
            gdb.execute('p /x registers')

            # print the memdiff
            for memname in memory_dump_ranges:
                print('memdiff ' + memname)
                diff = memdiff.main(build_bin_filename('after-cp', memname), \
                        build_bin_filename('after-hardfault', memname), \
                        memory_dump_ranges[memname][0])
                if diff is not None:
                    for diff_addr in diff:
                        sym_cmd = 'info symbol ' + hex(diff_addr)
                        #print('Sumbol command: ' + sym_cmd)
                        gdb.execute(sym_cmd)
        else:
            gdb.execute("bt")

        gdb.execute('c')

def register_breakpoint_handler():
    gdb.events.stop.connect(breakpoint_handler)
    print('\nRegistered breakpoint handler')

def unregister_breakpoint_handler():
    gdb.events.stop.disconnect(breakpoint_handler)
    print('\nUnregistered breakpoint handler')

def main():
    setup()
    set_breakpoints()
    register_breakpoint_handler()

main()