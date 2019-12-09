import os
import sys
import re

import memdiff

LOG_FILE='/tmp/gdb.log'

MEM_DUMP_BASENAME = '_memory_dump_'

breakpoints = [
        'gdb_break_me', # after checkpoint (restore point)
        'HardFault_Handler',
        'vm.c:210',
        ]

memory_dump_ranges = {
        'stack':        [0x00, 0x00],
        'data':         [0x00, 0x00],
        'bss':          [0x00, 0x00],
        'allocations':  [0x00, 0x00], # From end bss to end (start) stack
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

def setup_static_memory_regions():
    print('Setting up static memory regions (.data, .bss)')

    # Data section
    data_start_str = gdb.execute('print &_srelocate', to_string=True)
    data_end_str = gdb.execute('print &_erelocate_cp', to_string=True)
    data_start = int(re.search('.*(0x[^\s]*).*$', data_start_str)[1], 16)
    data_end = int(re.search('.*(0x[^\s]*).*$', data_end_str)[1], 16)

    # BSS section
    bss_start_str = gdb.execute('print &_sbss', to_string=True)
    bss_end_str = gdb.execute('print &_ebss_cp', to_string=True)
    bss_start = int(re.search('.*(0x[^\s]*).*$', bss_start_str)[1], 16)
    bss_end = int(re.search('.*(0x[^\s]*).*$', bss_end_str)[1], 16)

    # Stack end
    stack_end_str = gdb.execute('print &_estack', to_string=True)
    stack_end = int(re.search('.*(0x[^\s]*).*$', stack_end_str)[1], 16)

    # Allocations start (after bss)
    bss_nocp_end_str = gdb.execute('print &_ebss', to_string=True)
    bss_nocp_end = int(re.search('.*(0x[^\s]*).*$', bss_nocp_end_str)[1], 16)

    memory_dump_ranges['data'][0] = data_start
    memory_dump_ranges['data'][1] = data_end
    memory_dump_ranges['bss'][0] = bss_start
    memory_dump_ranges['bss'][1] = bss_end
    memory_dump_ranges['stack'][1] = stack_end
    memory_dump_ranges['allocations'][0] = bss_nocp_end

    print('.data start: ' + hex(data_start) + ' end: ' + hex(data_end))
    print('.bss start: ' + hex(bss_start) + ' end: ' + hex(bss_end))

def setup_dynamic_memory_regions():
    # Stack
    memory_dump_ranges['stack'][0] = get_sp()

    # Allocations (betweem the end of the bss and the top of the stack (SP)
    memory_dump_ranges['allocations'][1] = memory_dump_ranges['stack'][0]

    # Garbage Collection
    #gc_start_str = gdb.execute('print mp_state_ctx.mem.gc_pool_start', to_string=True)
    #gc_end_str = gdb.execute('print mp_state_ctx.mem.gc_pool_end', to_string=True)
    #gc_start = int(re.search('.*(0x[^\s]*).*$', gc_start_str)[1], 16)
    #gc_end = int(re.search('.*(0x[^\s]*).*$', gc_end_str)[1], 16)

    print('stack start: ' + hex(memory_dump_ranges['stack'][0]) \
            + ' end: ' + hex(memory_dump_ranges['stack'][1]))
    print('Allocations start: ' + hex(memory_dump_ranges['allocations'][0]) \
            + ' end: ' + hex(memory_dump_ranges['allocations'][1]))
    #print('GC pool start: ' + hex(gc_start) + ' end: ' + hex(gc_end))

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
                setup_dynamic_memory_regions()
                print('current SP: ' + hex(get_sp()))

                dump_memory('after-cp')
                print('Registers array:')
                gdb.execute('p /x registers')
                print('Registers:')
                gdb.execute('info reg')
                print('Allocation table:')
                gdb.execute('p allocations')
                gdb.execute('c')
            else:
                dump_memory('after-restore')
                print('current SP: ' + hex(get_sp()))
                #gdb.execute('mtb')

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
                print('Registers array:')
                gdb.execute('p /x registers')
                print('Registers:')
                gdb.execute('info reg')
                print('Allocation table:')
                gdb.execute('p allocations')
                gdb.execute('c')

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
            gdb.execute('mtb')

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
        elif location == breakpoints[2]:
            gdb.execute("bt")
            print('Registers:')
            gdb.execute('info reg')
            print('Code state:')
            gdb.execute('p *code_state')
            gdb.execute('c')

        else:
            gdb.execute("bt")

        #gdb.execute('c')

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
    setup_static_memory_regions()

main()
