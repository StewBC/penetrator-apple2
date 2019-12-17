copy apple2\template.dsk Penetrator.dsk
java -jar apple2\AppleCommander-win64-1.5.0.jar -p  Penetrator.dsk penetrat.system sys < penetrator-apple2.apple2.loader
java -jar apple2\AppleCommander-win64-1.5.0.jar -as Penetrator.dsk penetrat        bin < penetrator-apple2.apple2
apple2\Applewin.exe -d1 penetrator.dsk