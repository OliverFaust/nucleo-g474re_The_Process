#!/usr/bin/env python3
"""measure.py <elf> <uart-log> [seconds]: flash + UART log (nucleo_run.py), then read over SWD
(hotplug, no reset): stack watermarks, thread priorities and names, FreeRTOS heap counters, sbrk.
Tools: NUCLEO_RUN (CSP4CMSIS tests/hw_nucleo_g474/nucleo_run.py) and STM32_PROGRAMMER_CLI from the
environment.
Layout: TCB_t (FreeRTOS 10.3.1) uxPriority +0x2c, pcTaskName +0x34; HelloProcess m_stack +0x10
(1024 B), m_tcb +0x410 (from the Debug ELF's debug info; independent of optimisation)."""
import os, re, subprocess, sys
elf, log = sys.argv[1], sys.argv[2]
secs = sys.argv[3] if len(sys.argv) > 3 else '20'
CLI = os.environ.get('STM32_PROGRAMMER_CLI', 'STM32_Programmer_CLI')
print(subprocess.run(['python3', os.environ['NUCLEO_RUN'], elf, log, secs],
                     capture_output=True, text=True).stdout.strip().splitlines()[-1])
syms, sizes = {}, {}
for l in subprocess.run(['arm-none-eabi-nm', '-C', '-S', elf], capture_output=True, text=True).stdout.splitlines():
    p = l.split(' ', 3)
    if len(p) == 4: syms[p[3]], sizes[p[3]] = int(p[0], 16), int(p[1], 16)
    elif len(p) == 3: syms[p[2]] = int(p[0], 16)
def words(addr, n):
    out = subprocess.run([CLI, '-c', 'port=SWD', 'mode=HOTPLUG', '-r32', hex(addr), str(n * 4)],
                         capture_output=True, text=True).stdout
    out = re.sub(r'\x1b\[[0-9;]*m', '', out)
    w = []
    for l in out.splitlines():
        if re.match(r'^0x[0-9A-Fa-f]{8} :', l): w += [int(x, 16) for x in l.split(':', 1)[1].split()]
    return w[:n]
def stack(name, addr, nbytes):
    w = words(addr, nbytes // 4); free = 0
    for x in w:
        if x != 0xA5A5A5A5: break
        free += 1
    print(f'stack {name:12s} {nbytes:5d} B: used {nbytes - 4 * free:5d} B, never touched {4 * free:5d} B')
def tcb(name, addr):
    w = words(addr, 0x44 // 4)
    nm = b''.join(x.to_bytes(4, 'little') for x in w[0x34 // 4:0x44 // 4]).split(b'\0')[0].decode()
    print(f'thread {name:12s} priority {w[0x2c // 4]:2d}  name "{nm}"')
hello = syms['MainApp_Task(void*)::hello']
stack('HelloProcess', hello + 0x10, 1024)
stack('MainApp', syms['mainAppStack'], sizes['mainAppStack'])
stack('defaultTask', syms['defaultTaskBuffer'], sizes['defaultTaskBuffer'])
tcb('HelloProcess', hello + 0x410)
tcb('MainApp', syms['mainAppControlBlock'])
tcb('defaultTask', syms['defaultTaskControlBlock'])
h = words(syms['xFreeBytesRemaining'], 4)
print(f'FreeRTOS heap: free {h[0]} B, minimum ever free {h[1]} B, allocations {h[2]}, frees {h[3]}')
print(f'newlib sbrk: __sbrk_heap_end = {words(syms["__sbrk_heap_end"], 1)[0]:#010x}'
      f' (_end = {syms.get("_end", 0):#010x})')
