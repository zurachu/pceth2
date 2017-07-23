@echo off
if "%1" == "clean" goto clean
@echo on

unpack000 -bm2
del F*.BM2 THLF*.BM2 BAR*.BM2 BUTTON*.BM2 NE*.BM2 OPTION*.BM2
del ALBUM.BM2 BG.BM2 DATE.BM2 FRAME.BM2 GENERAL.BM2 MAPSELECT.BM2 MEMOCA.BM2 SAKURA.BM2 SAVELOAD.BM2 TITLE.BM2 YESNO.BM2
th2bm2cmv *.BM2
pceth2bmp *.bmp
pceth2map MAP01.bmp MAP0000.bmp THUM01A.bmp THUM01M.bmp B054000T.bmp
pceth2map MAP02.bmp MAP0200.bmp THUM02A.bmp THUM02M.bmp B031000T.bmp
pceth2map MAP03.bmp MAP0200.bmp THUM03A.bmp THUM03M.bmp B033000T.bmp
pceth2map MAP04.bmp MAP0300.bmp THUM04A.bmp THUM04M.bmp B069000T.bmp
pceth2map MAP05.bmp MAP0300.bmp THUM05A.bmp THUM05M.bmp B046000T.bmp
pceth2map MAP06.bmp MAP0400.bmp THUM06A.bmp THUM06M.bmp B040000T.bmp
pceth2map MAP07.bmp MAP0400.bmp THUM07A.bmp THUM07M.bmp B001000T.bmp
pceth2map MAP08.bmp MAP0100.bmp THUM08A.bmp THUM08M.bmp B025000T.bmp
pceth2map MAP09.bmp MAP0100.bmp THUM09A.bmp THUM09M.bmp B010000T.bmp
pceth2map MAP10.bmp MAP0100.bmp THUM10A.bmp THUM10M.bmp B026000T.bmp
pceth2map MAP11.bmp MAP0100.bmp THUM11A.bmp THUM11M.bmp B016000T.bmp
pceth2map MAP12.bmp MAP0100.bmp THUM12A.bmp THUM12M.bmp B021000T.bmp
pceth2map MAP13.bmp MAP0100.bmp THUM13A.bmp THUM13M.bmp B024000T.bmp
pceth2map MAP14.bmp MAP0100.bmp THUM14A.bmp THUM14M.bmp B023000T.bmp
pceth2map MAP15.bmp MAP0100.bmp THUM15A.bmp THUM15M.bmp B011000T.bmp
pceth2map MAP16.bmp MAP0100.bmp THUM16A.bmp THUM16M.bmp B013000T.bmp
pceth2map MAP17.bmp MAP0100.bmp THUM17A.bmp THUM17M.bmp B014000T.bmp
pceth2map MAP18.bmp MAP0100.bmp THUM18A.bmp THUM18M.bmp B012000T.bmp
pceth2map MAP19.bmp MAP0100.bmp THUM19A.bmp THUM19M.bmp B009000T.bmp
pceth2map MAP20.bmp MAP0100.bmp THUM20A.bmp THUM20M.bmp B015000T.bmp
pceth2map MAP21.bmp MAP0100.bmp THUM21A.bmp THUM21M.bmp B009000T.bmp
del *T.bmp THUM*.bmp
pgd16cmv -b *.bmp

copy /Y CAL\* .\

th2wav_mkbat > th2wav_setpath.bat
call th2wav_setpath
del *L.WAV SE_NONAME*.WAV NAME*.WAV
FOR %%A IN (*.WAV) DO dppcmcnv -b -fv8000 %%A

unpack000 -bin
@echo off
FOR %%A IN (EV_????MORNING.BIN) DO CALL :RENAME_EV_BIN %%A 0
FOR %%A IN (EV_????INTERVAL.BIN) DO CALL :RENAME_EV_BIN %%A 1
FOR %%A IN (EV_????SCHOOL_HOURS.BIN) DO CALL :RENAME_EV_BIN %%A 2
FOR %%A IN (EV_????LUNCH_BREAK.BIN) DO CALL :RENAME_EV_BIN %%A 3
FOR %%A IN (EV_????AFTER_SCHOOL.BIN) DO CALL :RENAME_EV_BIN %%A 4
FOR %%A IN (EV_????NIGHT.BIN) DO CALL :RENAME_EV_BIN %%A 6
@echo on
pceth2bin2 -s *.BIN

make -C mml
copy /Y mml\*.pmd .\

par c -C -T pceth2.par -f *.pgx *.pgd *.ppd *.scp *.pmd

:clean
@echo on
del *.BM2 *.bmp *.pg* *.WAV *.ppd *.BIN *.scp *.pmd

@echo off
goto end

rem BINファイルのリネーム
:RENAME_EV_BIN
SET FNAME=%1
SET NUM=%2
MOVE %FNAME% %FNAME:~0,7%%NUM%.BIN
EXIT /b

:end
