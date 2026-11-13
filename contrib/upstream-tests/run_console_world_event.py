"""Exercise the real Map event path with an identified random bot on Windows.

Temporarily moves a reserved RNDBOT actor, checks its [WORLD_EVENT] server trace,
then requests return to its saved database position. Does not complete quests.
"""
import argparse
import ctypes
from ctypes import wintypes as w
import json
import math
import os
from pathlib import Path
import re
import subprocess
import sys
import time


def send(pid, command):
    k = ctypes.WinDLL('kernel32', use_last_error=True)
    k.FreeConsole()
    if not k.AttachConsole(pid):
        raise ctypes.WinError(ctypes.get_last_error())
    input_handle = None
    try:
        class Key(ctypes.Structure):
            _fields_ = [('down',w.BOOL),('repeat',w.WORD),('key',w.WORD),
                        ('scan',w.WORD),('char',w.WCHAR),('control',w.DWORD)]
        class Event(ctypes.Union):
            _fields_ = [('key',Key),('padding',ctypes.c_byte*16)]
        class Record(ctypes.Structure):
            _fields_ = [('type',w.WORD),('event',Event)]
        # Redirected CLI stdin can remain a pipe/invalid standard handle after
        # AttachConsole. Open the attached console's input buffer explicitly.
        k.CreateFileW.argtypes = [w.LPCWSTR,w.DWORD,w.DWORD,w.LPVOID,w.DWORD,w.DWORD,w.HANDLE]
        k.CreateFileW.restype = w.HANDLE
        k.CloseHandle.argtypes = [w.HANDLE]
        input_handle = k.CreateFileW('CONIN$', 0xC0000000, 3, None, 3, 0, None)
        if input_handle in (None, ctypes.c_void_p(-1).value):
            input_handle = None
            raise ctypes.WinError(ctypes.get_last_error())
        k.WriteConsoleInputW.argtypes = [w.HANDLE,ctypes.POINTER(Record),w.DWORD,ctypes.POINTER(w.DWORD)]
        records = []
        for char in command + '\r':
            for down in (True,False):
                record = Record()
                record.type = 1
                record.event.key = Key(down,1,13 if char=='\r' else 0,0,char,0)
                records.append(record)
        buffer = (Record*len(records))(*records)
        written = w.DWORD()
        if not k.WriteConsoleInputW(input_handle,buffer,len(records),ctypes.byref(written)):
            raise ctypes.WinError(ctypes.get_last_error())
        if written.value != len(records):
            raise RuntimeError('Incomplete console command')
    finally:
        if input_handle is not None:
            k.CloseHandle(input_handle)
        k.FreeConsole()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config',type=Path,required=True)
    parser.add_argument('--client',type=Path,required=True)
    parser.add_argument('--pid',type=int,required=True)
    parser.add_argument('--bot',required=True)
    parser.add_argument('--log',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--event',type=int)
    mode.add_argument('--height-phase',type=int)
    parser.add_argument('--map',type=int,required=True)
    parser.add_argument('--x',type=float,required=True)
    parser.add_argument('--y',type=float,required=True)
    parser.add_argument('--z',type=float,required=True)
    parser.add_argument('--min-created',type=int,default=0)
    parser.add_argument('--max-created',type=int,default=25)
    parser.add_argument('--min-height',type=float,default=0)
    parser.add_argument('--min-delta',type=float,default=0)
    args = parser.parse_args()
    if os.name != 'nt' or not re.fullmatch('[A-Za-z]+',args.bot):
        raise ValueError('Windows console and an ASCII bot name are required')
    if not all(math.isfinite(v) for v in (args.x,args.y,args.z)):
        raise ValueError('Nonfinite test location')
    config = args.config.read_text(encoding='utf-8-sig')

    def query(key,sql):
        match = re.search(r'^'+key+r'\s*=\s*"([^"]+)"',config,re.M)
        if not match:
            raise ValueError('Database connection missing: '+key)
        host,port,user,password,database = match[1].split(';')
        process = subprocess.run([str(args.client.resolve()),'--no-defaults','--protocol=TCP',
                                  '--host='+host,'--port='+port,'--user='+user,'--database='+database,
                                  '--batch','--skip-column-names'],input=sql,text=True,encoding='utf-8',
                                 capture_output=True,timeout=20,env=dict(os.environ,MYSQL_PWD=password))
        if process.returncode:
            raise RuntimeError('Test-actor database lookup failed')
        return process.stdout.strip()

    values = query('CharacterDatabaseInfo',
                   "SELECT guid,account,online,map,position_x,position_y,position_z,orientation FROM characters WHERE name='"+args.bot+"';").split('\t')
    if len(values)!=8:
        raise ValueError('Expected one existing test bot')
    account = query('LoginDatabaseInfo','SELECT username FROM account WHERE id='+str(int(values[1]))+';')
    if not re.fullmatch(r'RNDBOT\d+',account,re.I):
        raise ValueError('Refusing to move a personal character: reserved RNDBOT account required')
    report = {'passed':False,'coverage':'Map event dispatch and bounded spawn count' if args.event is not None else 'Phased gameobject floor height',
              'quest_playthrough':False,'bot':args.bot,'guid':int(values[0]),
              'saved_before':dict(zip(['map','x','y','z','orientation'],values[3:])),
              'event':args.event,'height_phase':args.height_phase,'return_to_saved_position_requested':False}
    moved = False
    try:
        if values[2]!='1':
            send(args.pid,'botscenario '+args.bot+' login')
            for _ in range(30):
                time.sleep(1)
                if query('CharacterDatabaseInfo','SELECT online FROM characters WHERE guid='+values[0]+';')=='1':
                    break
            else:
                raise RuntimeError('Random bot did not come online within30 seconds')
        send(args.pid,f'botscenario {args.bot} teleport {args.map} {args.x} {args.y} {args.z}')
        moved = True
        time.sleep(3)
        start = len(args.log.read_text(encoding='utf-8',errors='replace'))
        if args.event is not None:
            send(args.pid,f'botscenario {args.bot} worldevent {args.event}')
            pattern = re.compile(r'\[WORLD_EVENT\] bot='+re.escape(args.bot)+r' event='+str(args.event)+r' map=(\d+) before=(\d+) after=(\d+) created=(-?\d+)')
        else:
            send(args.pid,f'botscenario {args.bot} worldheight {args.height_phase} {args.x} {args.y} {args.z}')
            pattern = re.compile(r'\[WORLD_HEIGHT\] bot='+re.escape(args.bot)+r' map=(\d+) phase='+str(args.height_phase)+r' normal=([-\d.]+) phased=([-\d.]+)')
        match = None
        for _ in range(15):
            time.sleep(1)
            match = pattern.search(args.log.read_text(encoding='utf-8',errors='replace')[start:])
            if match:
                break
        if not match:
            raise RuntimeError('No matching completed mechanism trace; not a gameplay pass')
        if args.event is not None:
            report.update(actual_map=int(match[1]),before=int(match[2]),after=int(match[3]),created=int(match[4]))
            report['passed'] = int(match[1])==args.map and args.min_created<=int(match[4])<=args.max_created
        else:
            normal, phased = float(match[2]),float(match[3])
            report.update(actual_map=int(match[1]),normal_height=normal,phased_height=phased)
            report['passed'] = int(match[1])==args.map and math.isfinite(phased) and phased>=args.min_height and phased-normal>=args.min_delta
    except Exception as error:
        report['error'] = str(error)
    finally:
        if moved:
            try:
                send(args.pid,f'botscenario {args.bot} teleport {values[3]} {values[4]} {values[5]} {values[6]}')
                report['return_to_saved_position_requested'] = True
            except Exception:
                report['return_to_saved_position_requested'] = False
        args.report.parent.mkdir(parents=True,exist_ok=True)
        args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report))
    return 0 if report['passed'] else 1


if __name__=='__main__':
    sys.exit(main())
