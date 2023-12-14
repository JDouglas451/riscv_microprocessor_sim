#!/usr/bin/env python3
# RISC-V Sim Host (or RISC-V sim shell, if you prefer)
# (c) 2016-2023, Bob Jones University
# Professionally mutilated by Joshua Douglas and Ryan Moffitt

import argparse
import ast
import csv
import ctypes
import hashlib
import mmap
import os
import re
import queue
import struct
import sys
import threading
import time
from typing import Optional

__version__ = "2023-11-09-1200"

try:
    import msvcrt

    class GetchThread(threading.Thread):
        def __init__(self, input_queue, notifier=None):
            super().__init__()
            self._inputq = input_queue
            self._notify = notifier
            self._done = threading.Event()

        def halt(self):
            self._done.set()

        def run(self):
            while not self._done.wait(0.01):
                if msvcrt.kbhit():
                    self._inputq.put(ord(msvcrt.getch()) & 0xff)
                    if self._notify:
                        self._notify()
except ImportError:
    import termios
    import tty
    import select

    class GetchThread(threading.Thread):
        def __init__(self, input_queue, notifier=None):
            super().__init__()
            self._inputq = input_queue
            self._notify = notifier
            self._done = threading.Event()

        def halt(self):
            self._done.set()

        def run(self):
            fd = sys.stdin.fileno()
            attrs = termios.tcgetattr(fd)
            tty.setcbreak(fd)
            try:
                while not self._done.wait(0.1):
                    rds, _, _ = select.select([fd], [], [], 0)
                    if rds:
                        data = os.read(fd, 16)
                        for b in data:
                            b &= 0xff
                            self._inputq.put(13 if b == 10 else b)  # Hack: send '\r' if the user hits '\n'
                        if self._notify:
                            self._notify()
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, attrs)


# Constants
#######################################################################

RGX_MEM_SIZE = re.compile(r"^(?P<size>\d+)(?P<scale>[kmg])b?$")

MEM_SCALE = {
    'k': 1024,
    'm': 1024**2,
    'g': 1024**3,
}

# According to the RISC-V spec, EEI's may support misaligned loads/stores
MEM_REQUIRE_ALIGNMENT = True

ELF_HEADER = struct.Struct("<4s5B7x2H1I3Q1I6H")
EH_MAGIC = 0
EH_CLASS = 1
EH_DATA = 2
EH_MACHINE = 7
EH_ENTRY = 9
EH_PHOFF = 10
EH_SHOFF = 11
EH_PHENTSIZE = 14
EH_PHENTNUM = 15
EH_SHENTSIZE = 16
EH_SHENTNUM = 17
EH_SHSTRNDX = 18
ET_EXEC = 2

PROGRAM_HEADER = struct.Struct("<2I6Q")
PH_TYPE = 0
PH_OFFSET = 2
PH_VADDR = 3
PH_FILESZ = 5
PH_MEMSZ = 6

PT_LOAD = 1

SECTION_HEADER = struct.Struct("<2I4Q2I2Q")
SH_NAME = 0
SH_OFFSET = 4
SH_SIZE = 5

# NOTE: ARM modes were removed without replacement

REG_NAME_MAP = {
    "x0":   0, "x1":   1, "x2":   2, "x3":   3,
    "x4":   4, "x5":   5, "x6":   6, "x7":   7,
    "x8":   8, "x9":   9, "x10": 10, "x11": 11,
    "x12": 12, "x13": 13, "x14": 14, "x15": 15,
    "x16": 16, "x17": 17, "x18": 18, "x19": 19,
    "x20": 20, "x21": 21, "x22": 22, "x23": 23,
    "x24": 24, "x25": 25, "x26": 26, "x27": 27,
    "x28": 28, "x29": 29, "x30": 30, "x31": 31,
    "zero": 0, "ra":   1, "sp":   2, "gp":   3,
    "tp":   4, "t0":   5, "t1":   6, "t2":   7,
    "s0":   8, "fp":   8, "s1":   9, "a0":  10,
    "a1":  11, "a2":  12, "a3":  13, "a4":  14,
    "a5":  15, "a6":  16, "a7":  17, "s2":  18,
    "s3":  19, "s4":  20, "s5":  21, "s6":  22,
    "s7":  23, "s8":  24, "s9":  25, "s10": 26,
    "s11": 27, "t3":  28, "t4":  29, "t5":  30, "t6": 31,
}

# Config flags
RC_NOTHING      = 0X00000000
RC_TRACE_LOG    = 0X00000001
RC_MPU_ON       = 0X00000002
RC_CACHE_ON     = 0X00000004

# Signal enum
RS_HALT = 0

# Poor-man's [thread-compatible] atexit alternative
SHUTDOWN = []
def shutdown():
    for func in SHUTDOWN:
        func()

def panic(msg, *args, **kwargs):
    """Called to abort all activity and die with an error message to stderr.
    
    Useful to handle fatal errors in Python callbacks invoked by C code, since
    otherwise ctypes swallows the Python exception and allows things to continue.
    """
    print("PANIC: " + msg, *args, **kwargs)
    shutdown()
    sys.exit(1)


class ElfFile(object):
    """Simplistic ELF executable file loader for ARM(EL) binaries.
    """
    def __init__(self, filename):
        # Map the file into memory instead of slurping it into an array
        with open(filename, 'rb') as fd:
            self._raw = mmap.mmap(fd.fileno(), 0, access=mmap.ACCESS_READ)

        # Parse as a 32-bit ELF header
        fields = ELF_HEADER.unpack_from(self._raw)

        # Simple sanity checks
        assert (fields[EH_MAGIC] == b"\x7fELF"), "{0} is *not* an ELF file!".format(filename)
        assert (fields[EH_CLASS] == 2), "{0} is not a 64-bit ELF file!".format(filename)
        assert (fields[EH_DATA] == 1), "{0} uses big-endian data formatting!".format(filename)
        assert (fields[EH_MACHINE] == 0xF3), "{0} is not an RISCV binary!".format(filename)
        
        # Carve up section header table
        sh_table_size = fields[EH_SHENTSIZE] * fields[EH_SHENTNUM]
        sh_table_start = fields[EH_SHOFF]
        sh_table_end = sh_table_start + sh_table_size
        sh_table_data = self._raw[sh_table_start:sh_table_end]
        self._sections = [sh for sh in SECTION_HEADER.iter_unpack(sh_table_data)]
        
        # Find the section-name-string-table section and extract string names
        strtab_sh = self._sections[fields[EH_SHSTRNDX]]
        strtab_start = strtab_sh[SH_OFFSET]
        strtab_end = strtab_start + strtab_sh[SH_SIZE]
        strtab_data = self._raw[strtab_start:strtab_end]
        self._section_names = []
        for s in self._sections:
            name_start = s[SH_NAME]
            name_end = strtab_data.index(b'\0', name_start)
            self._section_names.append(strtab_data[name_start:name_end].decode('utf-8'))
        
        # Extract entry point
        self._entry = fields[EH_ENTRY]

        # Find all program header entries
        ph_table_size = fields[EH_PHENTSIZE] * fields[EH_PHENTNUM]
        ph_table_start = fields[EH_PHOFF]
        ph_table_end = ph_table_start + ph_table_size
        ph_table_data = self._raw[ph_table_start:ph_table_end]
        self._segments = [ph for ph in PROGRAM_HEADER.iter_unpack(ph_table_data)
                             if ph[PH_TYPE] == PT_LOAD]
    
    @property
    def entry(self):
        return self._entry

    @property
    def segments(self):
        for _, flags, src_off, dst_addr, _, src_size, dst_size, _ in self._segments:
            chunk = self._raw[src_off:src_off + src_size]
            if dst_size > src_size:
                chunk += b'\0'*(dst_size - src_size)
            yield (dst_addr, flags, chunk)

    def get_section(self, name):
        """Return the raw bytes of a named section (or None if there is no such section).
        """
        try:
            index = self._section_names.index(name)
        except ValueError:
            return None
        else:
            sh = self._sections[index]
            start = sh[SH_OFFSET]
            end = start + sh[SH_SIZE]
            return self._raw[start:end]


class RISCVSimElfCompatScript:
    """A bundle of name=value pair configuration settings that can be embedded in an ELF file.

    Used to inform the RISC-V Sim system what the expectations of the program are (e.g., starting mode,
    initial register values, etc.)

    If the script includes a command to set a register to "entry", "entry" is replaced by the
    address of the entry-point from the originating ELF file.
    """
    def __init__(self, riscvsim_section : bytes, elf_entry_point : int = 0):
        self._regs = []
        self._pc = None

        chunks = riscvsim_section.decode('utf-8').lower().split()
        for c in chunks:
            name, value = c.split('=', 1)
            if value == "entry":
                value = elf_entry_point
            else:
                value = int(value, 0)
            if name == "pc":
                self._pc = value
            else:
                reg = REG_NAME_MAP[name]
                self._regs.append((reg, value))

    def apply(self, rsk):
        """Apply a compatibiliy setting script to an already-initialized/reset kernel.
        """
        for reg, value in self._regs:
            rsk.reg_set(reg, value)
        if self._pc is not None:
            rsk.pc_set(self._pc)


class rskHostServices(ctypes.Structure):
    """Python analog to the rsk_host_services_t struct in C.
    
    Contains function pointers (ctypes callbacks) that allows a loaded
    kernel to call host-provided memory load/store functions and host-provided
    message logging functions.
    """

	# dword (*mem_load_dword)(dword address);
    MEM_LOAD_DWORD_TYPE = ctypes.CFUNCTYPE(ctypes.c_ulong, ctypes.c_ulong)

	# void (*mem_store_dword)(dword address, dword value);
    MEM_STORE_DWORD_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_ulong, ctypes.c_ulong)

	# word (*mem_load_word)(dword address);
    MEM_LOAD_WORD_TYPE = ctypes.CFUNCTYPE(ctypes.c_uint, ctypes.c_ulong)

	# void (*mem_store_word)(dword address, word value);
    MEM_STORE_WORD_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_ulong, ctypes.c_uint)

	# hword (*mem_load_hword)(dword address);
    MEM_LOAD_HWORD_TYPE = ctypes.CFUNCTYPE(ctypes.c_ushort, ctypes.c_ulong)

	# void (*mem_store_hword)(dword address, hword value);
    MEM_STORE_HWORD_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_ulong, ctypes.c_ushort)

	# byte (*mem_load_byte)(dword address);
    MEM_LOAD_BYTE_TYPE = ctypes.CFUNCTYPE(ctypes.c_ubyte, ctypes.c_ulong)

	# void (*mem_store_byte)(dword address, byte value);
    MEM_STORE_BYTE_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_ulong, ctypes.c_ubyte)

    # void (*log_trace)(unsigned step, dword pc, dword *registers);
    LOG_TRACE_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_uint, ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint64))

    # void (*log_msg)(const char *msg);
    LOG_MSG_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_char_p)

    # void (*panic)(const char *msg);
    PANIC_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_char_p)
    

    _fields_ = [
        ("mem_load_dword",  MEM_LOAD_DWORD_TYPE),
        ("mem_store_dword", MEM_STORE_DWORD_TYPE),
        ("mem_load_word",   MEM_LOAD_WORD_TYPE),
        ("mem_store_word",  MEM_STORE_WORD_TYPE),
        ("mem_load_hword",  MEM_LOAD_HWORD_TYPE),
        ("mem_store_hword", MEM_STORE_HWORD_TYPE),
        ("mem_load_byte",   MEM_LOAD_BYTE_TYPE),
        ("mem_store_byte",  MEM_STORE_BYTE_TYPE),
        ("log_trace",       LOG_TRACE_TYPE),
        ("log_msg",         LOG_MSG_TYPE),
        ("panic",           PANIC_TYPE),
    ]


class rskHostStats(ctypes.Structure):
    """Stats counters published by RISC-V Sim kernels.
    """

    _fields_ = [
        ("instructions", ctypes.c_uint),
        ("loads", ctypes.c_uint),
        ("stores", ctypes.c_uint),
        ("load_misses", ctypes.c_uint),
        ("store_misses", ctypes.c_uint)
    ]


class rskMockHost:
    """Fake RISC-V Sim host used for sanity testing mockup simulators.

    Basically just records calls to host API function pointers for later verification.
    """

    def __init__(self):
        self._hs = rskHostServices()
        self._calls = {}
        for aname, atype in rskHostServices._fields_:
            setattr(self._hs, aname, atype(getattr(self, aname)))
            self._calls[aname] = []

    @property
    def host_services(self):
        return self._hs

    def reset(self):
        """Delete all mock call records.
        """
        for call_list in self._calls.values():
            call_list.clear()

    def check(self, call_name, count=1, clear=True):
        """Assert/return a set number of call records for a given host API routine.
        """
        if len(self._calls[call_name]) != count:
            panic("Did not detect expected number/type of calls to '{0}'...".format(call_name))
        calls = self._calls[call_name][:]
        if clear:
            self._calls[call_name].clear()
        return calls if count != 1 else calls[0]

    def mem_load_dword(self, *args):
        self._calls["mem_load_dword"].append(args)
        return 0

    def mem_store_dword(self, *args):
        self._calls["mem_store_dword"].append(args)

    def mem_load_word(self, *args):
        self._calls["mem_load_word"].append(args)
        return 0

    def mem_store_word(self, *args):
        self._calls["mem_store_word"].append(args)

    def mem_load_hword(self, *args):
        self._calls["mem_load_hword"].append(args)
        return 0

    def mem_store_hword(self, *args):
        self._calls["mem_store_hword"].append(args)

    def mem_load_byte(self, *args):
        self._calls["mem_load_byte"].append(args)
        return 0

    def mem_store_byte(self, *args):
        self._calls["mem_store_byte"].append(args)

    def log_trace(self, *args):
        self._calls["log_trace"].append(args)

    def log_msg(self, *args):
        self._calls["log_msg"].append(args)

    def panic(self, *args):
        self._calls["panic"].append(args)


# No-op MMIO stub functions
###################################

def mmio_nop_load(address):
    return 0


def mmio_nop_store(address, value):
    pass


class RISCVSimShell:
    """Class representing the host system that contains an RISC-V Sim kernel.
    
    Provides simulated RAM & peripherals along with logging/tracing support.
    """
    
    MMIO_BASE = 0x80000000
    
    def __init__(self, mem_size, trace_log=None, debug_log=None, checksum=False, disasm_func=None, register_history=None):
        """Create an RISC-V Sim host system with <mem_size> bytes of RAM.
        
        If <trace_log> is not None, generate a trace log to that file (STDERR if <trace_log> is '-').
        If <debug_log> is not None, generate a debug log to that file (STDERR if <debug_log> is '-').

        If <disasm_func> is non-None, call it to include the disassembly of the instruction executed for each trace log record.
        """
        # Ensure sane/legal memory sizes (word-aligned)
        assert (0x1000 <= mem_size < self.MMIO_BASE)
        assert ((mem_size & 0b11) == 0)
        
        self._show_md5 = checksum
        self._disasm_func = disasm_func
        if trace_log:
            self._tlog = open(trace_log, "wt", encoding="utf-8") if trace_log != '-' else sys.stderr
        else:
            self._tlog = None
        
        if debug_log:
            self._dlog = open(debug_log, "wt", encoding="utf-8") if debug_log != '-' else sys.stderr
        else:
            self._dlog = None
        
        # Allocate a ctypes-array of bytes (equivalent to C unsigned char) that size
        RamType = ctypes.c_ubyte * mem_size
        self._ramlen = mem_size
        self._ram = RamType()
        
        print(register_history)
        # Used to cache previous register values for log_trace()
        self._register_history = register_history
        
        # Set up host services callbacks
        hs = rskHostServices()
        hs.mem_load_dword = rskHostServices.MEM_LOAD_DWORD_TYPE(self.mem_load_dword)
        hs.mem_store_dword = rskHostServices.MEM_STORE_DWORD_TYPE(self.mem_store_dword)
        hs.mem_load_word = rskHostServices.MEM_LOAD_WORD_TYPE(self.mem_load_word)
        hs.mem_store_word = rskHostServices.MEM_STORE_WORD_TYPE(self.mem_store_word)
        hs.mem_load_hword = rskHostServices.MEM_LOAD_HWORD_TYPE(self.mem_load_hword)
        hs.mem_store_hword = rskHostServices.MEM_STORE_HWORD_TYPE(self.mem_store_hword)
        hs.mem_load_byte = rskHostServices.MEM_LOAD_BYTE_TYPE(self.mem_load_byte)
        hs.mem_store_byte = rskHostServices.MEM_STORE_BYTE_TYPE(self.mem_store_byte)
        hs.log_trace = rskHostServices.LOG_TRACE_TYPE(self.log_trace)
        hs.log_msg = rskHostServices.LOG_MSG_TYPE(self.log_msg)
        hs.panic = rskHostServices.PANIC_TYPE(self.panic)
        self._hs = hs
        
        # Create MMIO and heartbeat registries
        self._mmio = {}
        self._beats = []
    
    # Host Services (callbacks)
    #############################
    
    @property
    def host_services(self):
        return self._hs
    
    def mem_load_dword(self, address : int) -> int:
        # Require dword alignment
        if MEM_REQUIRE_ALIGNMENT and (address & 0b111):
            panic("misaligned load dword @ {0:016x}".format(address))

        # Is this an MMIO load?
        if address >= self.MMIO_BASE:
            try:
                return self._mmio[address][0](address)
            except KeyError:
                panic("unimplemented MMIO load dword from {0:#x}".format(address))
        else:
            # Nope, RAM load; bounds check and return
            if address >= self._ramlen:
                panic("out-of-RAM load dword @ {0:016x}".format(address))
            else:
                value : int = (self._ram[address])
                value += (self._ram[address + 1] << 8)
                value += (self._ram[address + 2] << 16)
                value += (self._ram[address + 3] << 24)
                value += (self._ram[address + 4] << 32)
                value += (self._ram[address + 5] << 40)
                value += (self._ram[address + 6] << 48)
                value += (self._ram[address + 7] << 56)
                return value

    def mem_store_dword(self, address : int, value : int) -> int:
        # Require dword alignment
        if MEM_REQUIRE_ALIGNMENT and (address & 0b111):
            panic("misaligned store dword @ {0:016x}".format(address))
        
        # Is it MMIO?
        if address >= self.MMIO_BASE:
            try:
                self._mmio[address][1](address, value)
            except KeyError:
                panic("unimplemented MMIO store dword to {0:#x}".format(address))
        else:
            if address >= self._ramlen:
                panic("out-of-RAM store dword @ {0:016x}".format(address))
            else:
                self._ram[address] = (value)
                self._ram[address + 1] = (value >> 8) & 0xff
                self._ram[address + 2] = (value >> 16) & 0xff
                self._ram[address + 3] = (value >> 24) & 0xff
                self._ram[address + 4] = (value >> 32) & 0xff
                self._ram[address + 5] = (value >> 40) & 0xff
                self._ram[address + 6] = (value >> 48) & 0xff
                self._ram[address + 7] = (value >> 56) & 0xff
    
    def mem_load_word(self, address : int) -> int:
        # Require word alignment
        if MEM_REQUIRE_ALIGNMENT and (address & 0b11):
            panic("misaligned load word @ {0:016x}".format(address))


        # Is this an MMIO load?
        if address >= self.MMIO_BASE:
            try:
                return self._mmio[address][0](address)
            except KeyError:
                panic("unimplemented MMIO load word from {0:#x}".format(address))

        else:
            # Nope, RAM load; bounds check and return
            if address >= self._ramlen:
                panic("out-of-RAM load word @ {0:016x}".format(address))
            else:
                value : int = (self._ram[address])
                value += (self._ram[address + 1]  << 8)
                value += (self._ram[address + 2] << 16)
                value += (self._ram[address + 3] << 24)
                return value

    def mem_store_word(self, address : int, value : int) -> int:
        # Require word alignment
        if MEM_REQUIRE_ALIGNMENT and (address & 0b11):
            panic("misaligned store word @ {0:016x}".format(address))
        
        # Is it MMIO?
        if address >= self.MMIO_BASE:
            try:
                self._mmio[address][1](address, value)
            except KeyError:
                panic("unimplemented MMIO store word to {0:#x}".format(address))
        else:
            if address >= self._ramlen:
                panic("out-of-RAM store word @ {0:016x}".format(address))
            else:
                self._ram[address] = (value) & 0xff
                self._ram[address + 1] = (value >> 8) & 0xff
                self._ram[address + 2] = (value >> 16) & 0xff
                self._ram[address + 3] = (value >> 24) & 0xff

    def mem_load_hword(self, address : int) -> int:
        # Require hword alignment
        if MEM_REQUIRE_ALIGNMENT and (address & 0b1):
            panic("misaligned load hword @ {0:016x}".format(address))

        # Is this an MMIO load?
        if address >= self.MMIO_BASE:
            try:
                return self._mmio[address][0](address)
            except KeyError:
                panic("unimplemented MMIO load hword from {0:#x}".format(address))
        else:
            # Nope, RAM load; bounds check and return
            if address >= self._ramlen:
                panic("out-of-RAM load hword @ {0:016x}".format(address))
            else:
                value : int = (self._ram[address])
                value += (self._ram[address + 1] << 8)
                return value

    def mem_store_hword(self, address : int, value : int) -> int:
        # Require hword alignment
        if MEM_REQUIRE_ALIGNMENT and (address & 0b1):
            panic("misaligned store hword @ {0:016x}".format(address))
        
        # Is it MMIO?
        if address >= self.MMIO_BASE:
            try:
                self._mmio[address][1](address, value)
            except KeyError:
                panic("unimplemented MMIO store hword to {0:#x}".format(address))
        else:
            if address >= self._ramlen:
                panic("out-of-RAM store hword @ {0:016x}".format(address))
            else:
                self._ram[address] = (value) & 0xff
                self._ram[address + 1] = (value >> 8) & 0xff

    def mem_load_byte(self, address : int) -> int:
        # Is this an MMIO load?
        if address >= self.MMIO_BASE:
            try:
                return self._mmio[address][0](address)
            except KeyError:
                panic("unimplemented MMIO load byte from {0:#x}".format(address))
        else:
            # Nope, RAM load; bounds check and return
            if address >= self._ramlen:
                panic("out-of-RAM load byte @ {0:016x}".format(address))
            return self._ram[address]

    def mem_store_byte(self, address : int, value : int) -> int:
        # Is it MMIO?
        if address >= self.MMIO_BASE:
            try:
                self._mmio[address][1](address, value)
            except KeyError:
                panic("unimplemented MMIO store byte to {0:#x}".format(address))
        else:
            if address >= self._ramlen:
                panic("out-of-RAM store byte @ {0:016x}".format(address))
            else:
                self._ram[address] = value

    def log_trace(self, step : int, pc : int, gprs) -> None:
        if self._tlog:
            # Get checksum
            cksum = self.md5() if self._show_md5 else "-"*32

            # Begin log entry
            print("{0:06} {1:08x} {2}".format(step, pc, cksum), file=self._tlog)

            # Print changed registers
            col_width = 4
            col = 0
            print("\t\t", end='', file=self._tlog)
            for i in range(32):
                if self._register_history[i] != gprs[i]:
                    print("{0}={1:08x} ".format(i, gprs[i]), end='', file=self._tlog)

                    col += 1
                    if col == col_width:
                        print('\n\t\t', end='', file=self._tlog)
                        col = 0
                    self._register_history[i] = gprs[i]
            print("\n", end='', file=self._tlog)
            
            # Print dissasembly
            if self._disasm_func:
                iaddr = pc
                iword = self.mem_load_word(iaddr)
                print("\t\t({0})".format(self._disasm_func(iaddr, iword)), file=self._tlog)

            self._tlog.flush()

        for listener in self._beats:
            listener.heartbeat(step)

    def log_msg(self, msg):
        if self._dlog:
            print(msg.decode("ascii"), file=self._dlog)
            self._dlog.flush()
    
    def panic(self, msg):
        panic(msg.decode("ascii"))

    # Public (python-facing) utilities
    ###################################

    def register_mmio(self, mmio_offset, on_load=mmio_nop_load, on_store=mmio_nop_store):
        """Register a load/store handler pair for MMIO accesses at MMIO_BASE + <mmio_offset>.

        If either (or both!) <on_load> or <on_store> is not provided, no-op defaults are used.
        """
        self._mmio[self.MMIO_BASE + mmio_offset] = (on_load, on_store)

    def register_heartbeat(self, listener):
        """Register a <listener> object with a heartbeat(cycles) method for heartbeat notifications.

        When a simulator is running with tracing enabled, all heartbeat listeners will be notified
        with the current cycle count on each tracing callback.
        """
        self._beats.append(listener)
    
    def flush(self):
        if self._tlog:
            self._tlog.flush()
        if self._dlog:
            self._dlog.flush()

    def md5(self):
        """Return the MD5 checksum of all RAM as a hex string.
        """
        hash = hashlib.md5()
        if sys.byteorder == "little":
            # We simulate little-endian RISC-V, so the data is already in correct order--just hash it as-is (FAST)
            hash.update(self._ram)
        else:
            # Big-endian (SLOW)
            for i in range(self._ramlen):
                hash.update(bytes((self._ram[i],)))
        return hash.hexdigest()
    
    def load_elf(self, obj: ElfFile):
        for addr, _, blob in obj.segments:
            for i, b in enumerate(blob):
                self._ram[addr + i] = b
    
    def hexdump(self, start : int, length : int) -> None:
        ascii = []
        for i, addr in enumerate(range(start, start + length)):
            if not ascii:
                print("{0:08x}  ".format(i), end="")
            b = self._ram[addr]
            ascii.append(bytes((b,)))
            if (i + 1) & 0xf:
                term = " "
            else:
                adisp = ''.join(a if len(str(a)) == 1 else '.' for a in ascii)
                term = "  {0}\n".format(adisp)
                ascii = []
            print("{0:02x}".format(b), end=term)
        if i & 0xf:
            print("   " * (15 - (i & 0xf)), end=" ")
            print(''.join(a if a.isprintable() else '.' for a in ascii))
        
    def dump_ram_location(self):
        print("ProTip: RAM @ {0:#x}".format(ctypes.addressof(self._ram)))


def input_playback(stream):
    """Parse a text input stream to yield a sequence of (timestamp: int, data: bytes) tuples.
    """
    pts = 0
    for line in stream:
        line = line.rstrip('\n')
        if line:    # only non-empty lines
            if line[0].isspace():
                ts = pts + 1
                dstr = line.lstrip()
            else:
                ts, dstr = line.split(None, maxsplit=1)
                ts = int(ts, 0)
            if ts <= pts:
                raise ValueError("cannot run the clock backwards!")
            data = ast.literal_eval('"{0}"'.format(dstr)).encode('ascii')
            yield ts, data
            pts = ts


class RISCVSimConsole:
    """Toy serial console device implementation for and RISC-V Sim system.

    output: always sent to a Python file object (default: sys.stdout)
    input: can be interactive (read from console using getch()) or
            recorded ("played back" from a formatted text stream)

    "Playback" format:
    [CYCLE_COUNT] CHARS

    * CYCLE_COUNT is an optional integer telling when in a program's execution
        this input becomes available (which may trigger a simulated interrupt)
    * CHARS is a string of ASCII chars (interpreted using Python's escape rules)
    * they must be separated by whitespace
    * if CYCLE_COUNT is ommitted, the line must begin with whitespace
        (and an implied cycle count of <previous + 1> is assumed, or 0 if there
        is no previous entry)
    """

    # MMIO offsets for accessing serial console
    WRITE_PORT_OFFSET = 0
    READ_PORT_OFFSET = 4

    def __init__(self, source=None, sink=sys.stdout, notifier=None):
        """Initialize a console object.

        source: if None (the default), use getch() to fetch raw input from the [real] console
                otherwise, treat the input as a file stream and parse it for playback
        sink: always a Python file-like object we write output to
        notifier: arg-less callable used to notify someone that we have new input
                    (must be a thread-safe notification, since it may be called from
                    a background thread)
        """
        self._notify = notifier
        self._sink = sink

        # Not attached to an RISC-V Sim system host yet
        self._host = None

        # Input flows through this...
        self._inputq = queue.Queue()

        # ...but what feeds it?
        if source is None:
            # The real console
            self._playback = False

            self._worker = GetchThread(self._inputq, self._notify)
            self._worker.start()
        else:
            # A playback file
            self._playback = True
            self._playsrc = input_playback(source)
            self._playpair = next(self._playsrc, None)

    def close(self):
        if not self._playback:
            self._worker.halt()

    def heartbeat(self, cycles):
        """Callback by which the host can notify us that a given number of cycles has been reached.

        (The callback will happen only if tracing is enabled in the underlying simulation kernel.)
        """
        if self._playback:
            send_irq = False
            while self._playpair and (self._playpair[0] <= cycles):
                for byte in self._playpair[1]:
                    self._inputq.put(byte)
                    send_irq = True
                self._playpair = next(self._playsrc, None)

            if send_irq and self._notify:
                self._notify()

    def attach(self, host: RISCVSimShell):
        """Attach this console to a given RISCVSimShell instance.

        Registers itself for MMIO and triggering input interrupts.
        """
        self._host = host
        host.register_mmio(self.READ_PORT_OFFSET, on_load=self._mmio_on_load)
        host.register_mmio(self.WRITE_PORT_OFFSET, on_store=self._mmio_on_store)
        host.register_heartbeat(self)

    def _mmio_on_load(self, address):
        """Read a pending byte from the console (return 0 if no such byte available).
        """
        try:
            return self._inputq.get(False)
        except queue.Empty:
            return 0

    def _mmio_on_store(self, address, value):
        """Push a character into the output stream.
        """
        c = chr((value & 0xff) if value != 13 else 10)
        self._sink.write(c)
        self._sink.flush()


class RISCVSimKernel:
    """Wrapper/facade class representing a loaded RISC-V Sim kernel implementation.
    
    Provides methods to handle all interactions with the kernel.
    """
    def __init__(self, loaded_library):
        self._dll = loaded_library
        self._has_disasm = True    # disasm is a backwards compatible extension to API version 1.0 (see `info()`)
        
        # rsk_info() -> char ** (NULL terminated list of NUL-terminated C strings)
        self._dll.rsk_info.restype = ctypes.POINTER(ctypes.c_char_p)
        self._dll.rsk_info.argtypes = ()
        
        # rsk_init(struct rsk_host_services_t *host) -> void
        self._dll.rsk_init.restype = None
        self._dll.rsk_init.argtypes = (ctypes.POINTER(rskHostServices),)
        
        # rsk_stats_report(struct rsk_stats *stats) -> void
        self._dll.rsk_stats_report.restype = None
        self._dll.rsk_stats_report.argtypes = (ctypes.POINTER(rskHostStats),)
        
        # rsk_reg_get(int index) -> ulong
        self._dll.rsk_reg_get.restype = ctypes.c_ulong
        self._dll.rsk_reg_get.argtypes = (ctypes.c_int,)
        
        # rsk_reg_set(int index, ulonglong value) -> void
        self._dll.rsk_reg_set.restype = None
        self._dll.rsk_reg_set.argtypes = (ctypes.c_int, ctypes.c_ulong)
        
        # rsk_pc_get(void) -> ulong
        self._dll.rsk_pc_get.restype = ctypes.c_ulong
        self._dll.rsk_pc_get.argtypes = ()
        
        # rsk_pc_set(ulong value) -> void
        self._dll.rsk_pc_set.restype = None
        self._dll.rsk_pc_set.argtypes = (ctypes.c_ulong,)
        
        # rsk_config_get(void) -> bitflags
        self._dll.rsk_config_get.restype = ctypes.c_uint
        self._dll.rsk_config_get.argtypes = ()
        
        # rsk_config_set(enum) -> void
        self._dll.rsk_config_set.restype = None
        self._dll.rsk_config_set.argtypes = (ctypes.c_uint,)
        
        # rsk_cpu_running() -> int
        self._dll.rsk_cpu_running.restype = ctypes.c_int
        self._dll.rsk_cpu_running.argtypes = ()
        
        # rsk_cpu_run(int cycles) -> int
        self._dll.rsk_cpu_run.restype = ctypes.c_int
        self._dll.rsk_cpu_run.argtypes = (ctypes.c_int,)
        
        # rsk_cpu_signal(enum signal) -> void
        self._dll.rsk_cpu_signal.restype = None
        self._dll.rsk_cpu_signal.argtypes = (ctypes.c_int,)
    
    def info(self):
        """Calls rsk_info() from the loaded kernel library.
        
        Returns a dictionary of {"name": "value"} parsed from the list of "name=value" strings.
        """
        info = {}
        cstrings = self._dll.rsk_info()
        if cstrings:
            i = 0
            while cstrings[i] is not None:
                fields = cstrings[i].decode('ascii').split('=', 1)
                if len(fields) == 2:
                    info[fields[0]] = fields[1]
                else:
                    info[fields[0]] = True
                i += 1

        # backwards-compatible addendum to API version 1.0: 
        # if we have a "disasm" feature, resolve/use the "rsk_disasm" library function for trace logs
        if "disasm" in info:
            self._has_disasm = True
            self._dll.rsk_disasm.restype = None
            self._dll.rsk_disasm.argtypes = (ctypes.c_ulong, ctypes.c_uint, ctypes.c_char_p, ctypes.c_size_t)

        return info

    def disasm(self, address: int, instruction: int) -> str:
        """Uses rsk_disasm(...) [if available!] to disassemble `instruction` (at `address` in RAM).

        If the kernel doesn't implement rsk_disasm(...), returns None instead.
        """
        if not self._has_disasm:
            return None

        blen = 128
        buff = ctypes.create_string_buffer(blen)
        self._dll.rsk_disasm(address, instruction, buff, blen)
        return ctypes.string_at(buff).decode().strip()
    
    def init(self, host_services : rskHostServices) -> None:
        """Calls rsk_init(...), passing in a structure of callbacks.
        """
        return self._dll.rsk_init(host_services)
    
    def stats(self) -> rskHostStats:
        """Calls rsk_stats_report(...) and returns the populated struct.
        """
        stats = rskHostStats()
        self._dll.rsk_stats_report(stats)
        return stats
    
    def config_get(self) -> int:
        """Pass-through to rsk_config_get()...
        """
        return self._dll.rsk_config_get()
    
    def config_set(self, config_flags : int) -> None:
        """Pass-through to rsk_config_set(...)...
        """
        self._dll.rsk_config_set(config_flags)
    
    def reg_get(self, index : int) -> int:
        """Calls rsk_reg_get(bank, index) and returns the result.
        """
        return self._dll.rsk_reg_get(index)
    
    def reg_set(self, index : int, value : int):
        """Calls rsk_reg_get(bank, index, value).
        """
        self._dll.rsk_reg_set(index, value)
    
    def pc_get(self) -> int:
        """Calls rsk_pc_get() and returns the result.
        """
        return self._dll.rsk_pc_get()
    
    def pc_set(self, value : int) -> None:
        """Calls rsk_pc_get(value).
        """
        self._dll.rsk_pc_set(value)
    
    def is_running(self) -> bool:
        """Calls rsk_cpu_running() and returns the result.
        """
        return self._dll.rsk_cpu_running() == 1
    
    def run(self, cycles : int) -> int:
        """Pass-through to rsk_cpu_run(cycles)...
        """
        return self._dll.rsk_cpu_run(cycles)
    
    def signal(self, signal : int) -> None:
        """Pass-through to rsk_cpu_signal(signal)...
        """
        self._dll.rsk_cpu_signal(signal)


def mockup_tests(rsk : RISCVSimKernel) -> None:
    """Perform a round of mockup testing on a given kernel (starting with init testing).

    A properly-implemented mockup should withstand any number of these tests, one after the other.
    """
    import random

    mock = rskMockHost()

    # Validate rsk_init()
    rsk.init(mock.host_services)
    if rsk.config_get() != 0:
        panic("Config flags not zero'd by rsk_init...")
    for i in range(32):
        if rsk.reg_get(i) != 0:
            panic("Register {0} not zero'd by rsk_init...".format(i))
    if rsk.stats().instructions != 0:
        panic("Instruction count not reset to 0 by CPU initialization...")
    if mock.check("log_msg")[0] != b"CPU initialized":
        panic("Stipulated 'CPU initialized' log message not detected")
    mock.reset()

    # Does config setting work?
    rsk.config_set(RC_TRACE_LOG)
    if not (rsk.config_get() & RC_TRACE_LOG):
        panic("Unable to verify that rsk_config_set/rsk_config_set work...")

    # Does the zero register always contain zero?
    rsk.reg_set(0, 451)
    if rsk.reg_get(0) != 0:
        panic("Unable to verify that rsk_reg_set/rsk_reg_get work for x0...")

    # Does getting/setting the other registers work?
    regvalues = [random.randint(0, 2**64-1) for i in range(31)]
    for i, val in enumerate(regvalues, start=1):
        rsk.reg_set(i, val)
    for i, val in enumerate(regvalues, start=1):
        if rsk.reg_get(i) != val:
            panic("Unable to verify that rsk_reg_set/rsk_reg_get work for x1 - x31...")
    
    # Does setting pc work?
    value = random.randint(0, 2**64-1)
    rsk.pc_set(value)
    if rsk.pc_get() != value:
        panic("Unable to verify that rsk_pc_set/rsk_pc_get work...")

    # How about step counts?
    rsk.run(1)
    if rsk.stats().instructions != 1:
        panic("Unable to verify that rsk_cpu_run can increment instruction count by 1...")
    steps = random.randint(3, 100)
    rsk.run(steps)
    if rsk.stats().instructions != (steps + 1):
        panic("Unable to verify that rsk_cpu_run can increment instruction count by more than 1...")


def scaled_size(text : str) -> int:
    M = RGX_MEM_SIZE.match(text.strip().lower())
    if not M:
        raise ValueError(text)
    else:
        size, scale = M.group("size"), M.group("scale")
        if scale not in MEM_SCALE:
            raise ValueError(scale)
        else:
            return int(size) * MEM_SCALE[scale]

# NOTE: ARM FIQ mode was removed without a replacement
def main(argv) -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("-c", "--checksum", dest="checksum", action="store_true", default=False,
                    help="Compute and display the RAM checksum with every trace log entry (SLOW).")
    ap.add_argument("-D", "--disasm", dest="disasm", action="store_true", default=False,
                    help="Include ARM instruction disassembly in each trace log record (if kernel supports).")
    ap.add_argument("-d", "--debug-log", dest="debug_log", metavar="FILE", default=None,
                    help="Save debug log messages to FILE (or STDERR if FILE is '-')")
    ap.add_argument("-i", "--console-input", dest="input_file", metavar="FILE", default=None,
                    help="Take console input from playback FILE instead of host TTY.")
    ap.add_argument("-p", "--pause", dest="pause", action="store_true", default=False,
                    help="Pause (after printing PID) to allow debugger attachment.")
    ap.add_argument("-r", "--ram", dest="mem_size", metavar="SIZE", type=scaled_size, default="32k",
                    help="Size of memory in bytes (k/m/g valid as scale suffixen)")
    ap.add_argument("-t", "--trace-log", dest="trace_log", metavar="FILE", default=None,
                    help="Save trace log messages to FILE (or STDERR if FILE is '-')")
    ap.add_argument("-k", "--cache", action="store_true", default=False,
                    help="Enable memory cache (if implemented by KERNEL)")
    ap.add_argument("-s", "--stats-log", dest="stats_log", metavar="CSV_FILE", default=None,
                    help="Append performance stats to CSV_FILE")
    ap.add_argument("kernel", metavar="KERNEL_BIN",
                    help="Name of/path to loadable RISC-V Sim kernel library.")
    ap.add_argument("module", metavar="RISCV_ELF_BIN", nargs="?",
                    help="Path to RISC-V ELF file to load into simulated RAM "
                         "(if none specified, nothing is loaded/executed, but mockup tests may be performed).")

    print("RISC-V Sim Host, Version:", __version__)
    args = ap.parse_args(argv)
    if args.mem_size < 32*1024:
        print("Memory size must be at least 32KB!")
        exit(1)

    # Load kernel and create RAM array
    rsk = RISCVSimKernel(ctypes.cdll.LoadLibrary(args.kernel))
    shell = RISCVSimShell(mem_size=args.mem_size,
                          trace_log=args.trace_log,
                          debug_log=args.debug_log,
                          checksum=args.checksum,
                          disasm_func=rsk.disasm if args.disasm else None)

    # If so asked, pause and wait for input at this point
    if args.pause:
        # And dump out useful debugging tips (our PID and the address of the RAM array)
        shell.dump_ram_location()
        input("press ENTER to go continue (pid={0})".format(os.getpid()))

    # Check core metadata
    info = rsk.info()
    try:
        if info["api"] != "1.0":
            print("Unsupported API version ({0}); aborting...".format(info["api"]))
            exit(1)
    except KeyError:
        print("No API version reported; aborting...")
        exit(1)
    try:
        print("Author({0}): {1}".format(args.kernel, info["author"]))
    except KeyError:
        print("No author reported; aborting...")
        exit(1)

    # Validate args/features; warn about possibly bad stuff
    if args.cache and ("cache" not in info):
        print("WARNING: --cache specified, but {0} does not implement 'cache'...".format(args.kernel))
    if ("usr" in info) and (args.mem_size < 64*1024):
        print("WARNING: {0} supports 'usr' mode; you should probably have at least 64KB of RAM...".format(args.kernel))

    # If there's something to load/run, do it...
    if args.module:
        elf = ElfFile(args.module)
        shell.load_elf(elf)
        print("MD5({0}): {1}".format(args.module, shell.md5()))

        console = RISCVSimConsole(source=open(args.input_file, "rt", encoding="ascii") if args.input_file else None,
                                notifier=rsk.signal_irq if 'irq' in info else None)
        console.attach(shell)
        SHUTDOWN.append(console.close)

        # Initialize the CPU
        rsk.init(shell.host_services)

        # Set config flags (if any)
        cflags = RC_NOTHING
        if args.trace_log or args.input_file:
            # In order for input file playback to be synchronized with clock cycles inside
            # the simulator, we MUST enable tracing (even if we don't have a trace log getting saved)
            cflags |= RC_TRACE_LOG
        if args.cache:
            cflags |= RC_CACHE_ON
        rsk.config_set(cflags)

        # Pre-configure processor based on embedded ".riscvsim" section in ELF file (if it exists)
        elf_compat = elf.get_section(".riscvsim")
        if elf_compat:
            script = RISCVSimElfCompatScript(elf_compat, elf.entry)
            script.apply(rsk)
            shell._register_history = [rsk.reg_get(i) for i in range(32)]

        print()
        print("-"*60)
        print()
        shell.flush()
        start = time.perf_counter()
        rsk.run(0)
        stop = time.perf_counter()
        # Print performance stats
        stats = rsk.stats()
        span = stop - start
        try:
            ips = round(stats.instructions / span, 1)
            print("{0:,} instructions in {1:.3} seconds ({2:,} IPS)".format(stats.instructions, span, ips))
        except ZeroDivisionError:
            print("{0:,} instructions in <mumble-mumble> seconds (<mumble-mumble> IPS)".format(stats.instructions))

        # If so requested, log raw performance stats
        if args.stats_log:
            with open(args.stats_log, "at", encoding="utf8", newline="") as fd:
                writer = csv.writer(fd)
                writer.writerow([
                    info["author"],
                    args.module,
                    span,
                    stats.instructions,
                    stats.loads,
                    stats.load_misses,
                    stats.stores,
                    stats.store_misses])

        # If caching was enabled, print cache stats
        if args.cache:
            print("Loads: {0:,} of which {1:,} missed (miss rate: {2:.2}%)".format(
                stats.loads, stats.load_misses, (stats.load_misses / stats.loads) * 100))
            print("Stores: {0:,} of which {1:,} missed (miss rate: {2:.2}%)".format(
                stats.stores, stats.store_misses, (stats.store_misses / stats.stores) * 100))
        else:
            # Otherwise, just show load and store counts
            print("Loads: {0:,}".format(stats.loads))
            print("Stores: {0:,}".format(stats.stores))

    elif "mockup" in info:
        print("Attribute 'mockup' detected; running mockup sanity checks...")

        # Run the basic test suite many times (at least 2 to verify proper init, and more
        # to hopefully catch any silly huge-resource-allocation bugs...)
        for i in range(100):
            mockup_tests(rsk)

        print("ALL SANITY CHECKS PASSED--WELL DONE")


if __name__ == "__main__":
    main(sys.argv[1:])
    shutdown()

