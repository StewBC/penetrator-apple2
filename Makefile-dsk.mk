DSK = Penetrator.dsk

# For this one, see https://applecommander.github.io/
AC ?= ac.jar

# Unix or Windows
ifeq ($(shell echo),)
	CP = cp $1
else
	CP = copy $(subst /,\,$1)
endif

REMOVES += $(DSK)

.PHONY: dsk
dsk: $(DSK)

$(DSK): penetrator-apple2.apple2.loader penetrator-apple2.apple2
	$(call CP, apple2/template.dsk $@)
	java -jar $(AC) -p  $@ penetrat.system sys < penetrator-apple2.apple2.loader
	java -jar $(AC) -as $@ penetrat        bin < penetrator-apple2.apple2
