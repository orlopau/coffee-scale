# create-fonts(folder, fontname)
define create-fonts
	for size in 10 20 ; do \
		./otf2bdf ./$(1)/$(2).ttf -v -n -r 100 -l "0_127" -p $$size -o ./$(1)/$(2)-lat-$$size.bdf ; \
		./bdfconv -v -f 1 -m "32-127" ./$(1)/$(2)-lat-$$size.bdf -o ../include/fonts/$(1)_$(2)_$$size.h \
			-n $(1)_$(2)_$$size -d ./$(1)/$(2)-lat-$$size.bdf ; \
		convert bdf.tga -trim -auto-orient ././$(1)/$(2)_$$size.png ; \
		rm bdf.tga ; \
	done
endef

font-roboto:
	$(call create-fonts,roboto,regular)

font-roboto-mono:
	$(call create-fonts,roboto_mono,regular)

HEADER := ../include/fonts/fonts.h

header:
	touch $(HEADER)

	echo "#pragma once" > $(HEADER)
	echo "" >> $(HEADER)
	for file in $$(find ../include/fonts -type f ! -name "fonts.h" -name "*.h") ; do \
		echo "#include \"$$(basename $$file)\"" >> $(HEADER) ; \
	done
	echo "" >> $(HEADER)

clean:
	rm ./**/*.bdf ../include/fonts/*.h ./**/*.png

all: font-roboto font-roboto-mono header