set LINUXHOST=117.50.172.197
set LINUXWORKPATH=/root/OSwork
set username=root

%.asm .c .h%
scp asmfunc.asm %username%@%LINUXHOST%:%LINUXWORKPATH%
scp os.c %username%@%LINUXHOST%:%LINUXWORKPATH%
scp linuxlink.ld %username%@%LINUXHOST%:%LINUXWORKPATH%
ssh -tt %username%@%LINUXHOST%<linuxbuild.sh
%将生成的sys.bin传送回来打包%
scp %username%@%LINUXHOST%:%LINUXWORKPATH%/sys.bin sys.bin
pause