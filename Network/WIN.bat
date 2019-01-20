mkdir %WINDIR%\KHJ
copy command_server.exe %WINDIR%\KHJ\command_server.exe
copy libgcc_s_dw2-1.dll %WINDIR%\KHJ\
copy libgmp-10.dll %WINDIR%\KHJ\
copy libgomp-1.dll %WINDIR%\KHJ\
copy libmpc-2.dll %WINDIR%\KHJ\
copy libmpfr-1.dll %WINDIR%\KHJ\
copy libpthread-2.dll %WINDIR%\KHJ\
copy libssp-0.dll %WINDIR%\KHJ\
copy libgcc_s_dw2-1.dll %WINDIR%\KHJ\
copy mingwm10.dll %WINDIR%\KHJ\
copy libexpat-1.dll %WINDIR%\KHJ\

copy WINC.bat %WINDIR%\KHJ\WINC.bat
%WINDIR%\KHJ\command_server.exe