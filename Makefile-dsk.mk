DSK = Penetrator.dsk

# For this one, see https://applecommander.github.io/
AC ?= ac.jar

REMOVES += $(DSK)

.PHONY: dsk
dsk: $(DSK)

$(DSK): penetrator-apple2.apple2.loader penetrator-apple2.apple2
	copy apple2\template.dsk $@
	java -jar $(AC) -p  $@ penetrat.system sys < penetrator-apple2.apple2.loader
	java -jar $(AC) -as $@ penetrat        bin < penetrator-apple2.apple2
