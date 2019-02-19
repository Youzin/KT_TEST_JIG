echo Ping test...
@ECHO OFF
set IPADDRESS=192.168.2.8
set INTERVAL=60
:PINGINTERVAL
ping %IPADDRESS% -n 60 
echo next ping
@ping 127.0.0.1 -n %INTERVAL% > nul
GOTO PINGINTERVAL