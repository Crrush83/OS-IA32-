set LINUXHOST=117.50.172.197
set LINUXWORKPATH=/root/OSwork
set username=root

%.asm .c .h%

scp os.c %username%@%LINUXHOST%:%LINUXWORKPATH%
scp name.h %username%@%LINUXHOST%:%LINUXWORKPATH%
scp io.asm %username%@%LINUXHOST%:%LINUXWORKPATH%
scp graphic.c %username%@%LINUXHOST%:%LINUXWORKPATH%
scp dsctb.c %username%@%LINUXHOST%:%LINUXWORKPATH%
scp dsctb.h %username%@%LINUXHOST%:%LINUXWORKPATH%
scp dsctb.asm %username%@%LINUXHOST%:%LINUXWORKPATH%
scp time.h %username%@%LINUXHOST%:%LINUXWORKPATH%
scp time.c %username%@%LINUXHOST%:%LINUXWORKPATH%
scp time.asm %username%@%LINUXHOST%:%LINUXWORKPATH%
scp interupt.h %username%@%LINUXHOST%:%LINUXWORKPATH%
scp interupt.asm %username%@%LINUXHOST%:%LINUXWORKPATH%
scp hankaku.asm %username%@%LINUXHOST%:%LINUXWORKPATH%
scp linuxlink.ld %username%@%LINUXHOST%:%LINUXWORKPATH%
ssh -tt %username%@%LINUXHOST%<linuxbuild.sh
%将生成的sys.bin传送回来打包%
scp %username%@%LINUXHOST%:%LINUXWORKPATH%/sys.bin sys.bin
pause