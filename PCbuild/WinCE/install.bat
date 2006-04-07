set DEST=dev:\program files\python24
set BUILD=%CFG%420

cecopy binaries\%BUILD%\python24.dll "%DEST%"
cecopy binaries\%BUILD%\python.exe "%DEST%"
cecopy binaries\lib\full-python24.zip "%DEST%\python24.zip"
cecopy binaries\%BUILD%\pywintypes.dll "%DEST%"

cecopy binaries\%BUILD%\_socket.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\_testcapi.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\_tkinter.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\pyexpat.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\select.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\unicodedata.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\winsound.pyd "%DEST%\DLLs"

cecopy binaries\%BUILD%\win32event.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\win32gui.pyd "%DEST%\DLLs"
cecopy binaries\%BUILD%\win32process.pyd "%DEST%\DLLs"
rem cecopy binaries\%BUILD%\wince.pyd "%DEST%\DLLs"

cecopy ..\..\lib\os.py "%DEST%\Lib"

