import os
import sys
import re

import memdiff

LOG_FILE='/tmp/gdb.log'

MEM_DUMP_BASENAME = '_memory_dump_'

breakpoints = [
        'checkpoint.c:383', # after checkpoint (restore point)
        'HardFault_Handler',
        ]

memory_dump_ranges = [
        ('stack',   0x20007b90, 0x20007bf8),
        ('data',    0x20000000, 0x20000010),
        ('bss',     0x20000400, 0x200006c4),
        ]

def build_bin_filename(prefix, memname):
    fname = prefix + MEM_DUMP_BASENAME + memname + '.bin'
    return fname

def dump_memory(prefix):
    for mem_range in memory_dump_ranges:
        memname = mem_range[0]
        start_addr = str(mem_range[1])
        end_addr = str(mem_range[2])
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
                dump_memory('after-cp')
            else:
                dump_memory('after-restore')

                # print the memdiff
                for mem_range in memory_dump_ranges:
                    memname = mem_range[0]
                    print('memdiff ' + memname)
                    diff = memdiff.main(build_bin_filename('after-cp', memname), \
                            build_bin_filename('after-restore', memname), \
                            mem_range[1])
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
