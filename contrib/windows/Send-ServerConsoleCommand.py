import ctypes,sys
from ctypes import wintypes as w
k=ctypes.WinDLL('kernel32',use_last_error=True)
k.FreeConsole()
if not k.AttachConsole(int(sys.argv[1])):raise ctypes.WinError(ctypes.get_last_error())
class Key(ctypes.Structure):
 _fields_=[('down',w.BOOL),('repeat',w.WORD),('key',w.WORD),('scan',w.WORD),('char',w.WCHAR),('control',w.DWORD)]
class Event(ctypes.Union):
 _fields_=[('key',Key),('padding',ctypes.c_byte*16)]
class Record(ctypes.Structure):
 _fields_=[('type',w.WORD),('event',Event)]
k.CreateFileW.restype=w.HANDLE
k.CreateFileW.argtypes=[w.LPCWSTR,w.DWORD,w.DWORD,w.LPVOID,w.DWORD,w.DWORD,w.HANDLE]
k.CloseHandle.argtypes=[w.HANDLE]
k.WriteConsoleInputW.argtypes=[w.HANDLE,ctypes.POINTER(Record),w.DWORD,ctypes.POINTER(w.DWORD)]
records=[]
for ch in sys.argv[2]+'\r':
 for down in (True,False):
  r=Record();r.type=1;r.event.key=Key(down,1,13 if ch=='\r' else 0,0,ch,0);records.append(r)
buffer=(Record*len(records))(*records);written=w.DWORD()
# STARTF_USESTDHANDLES (e.g. subprocess capture_output=True) preserves inherited
# pipe handles across AttachConsole. Open the attached console explicitly instead
# of accidentally sending INPUT_RECORDs to the parent's redirected stdin pipe.
console=k.CreateFileW('CONIN$',0xC0000000,3,None,3,0,None)
if console==w.HANDLE(-1).value:raise ctypes.WinError(ctypes.get_last_error())
try:
 if not k.WriteConsoleInputW(console,buffer,len(records),ctypes.byref(written)):raise ctypes.WinError(ctypes.get_last_error())
 if written.value!=len(records):raise RuntimeError('Incomplete console command')
finally:k.CloseHandle(console)
k.FreeConsole()
