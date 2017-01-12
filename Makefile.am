
SUBDIRS = src/THIRDPARTY/Tesseract \
src/THIRDPARTY/dlib-18.4 \
src/TOOLS/GROUNDTRUTH_GENERATION/select_random \
src/TOOLS/GROUNDTRUTH_GENERATION/select_training \
src/TOOLS/GROUNDTRUTH_GENERATION \
src/FINDER/COMMON \
src/FINDER/APP

EXTRA_DIST = autogen.sh data thesis final_results design/design_doc test_sets
toolsSrcDir=src/TOOLS/GROUNDTRUTH_GENERATION
toolsDestDir=$(DESTDIR)$(bindir)

#Warning requires tabs below (really annoying). The following adds the extra stuff to make life easier (removable by uninstalling)
install-exec-hook: 
	cp $(toolsSrcDir)/MathFinderGtGen/MathFinderGtGen $(toolsDestDir)/MathFinderGtGen; \
	cp $(toolsSrcDir)/select_random/select_random $(toolsDestDir)/MathFinderselect_random; \
	cp $(toolsSrcDir)/select_training/select_training $(toolsDestDir)/MathFinderselect_training; \
	cp $(toolsSrcDir)/convert_pdfpages $(toolsDestDir)/MathFinderconvert_pdfpages; \
	cp $(toolsSrcDir)/removealpha $(toolsDestDir)/MathFinderremovealpha; \
	cp $(toolsSrcDir)/rename_pngs $(toolsDestDir)/MathFinderrename_pngs; \
	cp $(toolsSrcDir)/writefilenames $(toolsDestDir)/MathFinderwritefilenames; \
	cd src/THIRDPARTY/Tesseract/java; make ScrollView.jar; chmod 755 ScrollView.jar; \
	cp ScrollView.jar $(toolsDestDir); \
	cd ..; tar -xzvf tesseract-ocr-3.02.eng.tar.gz; cd tesseract-ocr; cp tessdata /usr/local/share -R;
	
# Get rid of my stuff, Deciding to leave tessdata in case its been updated or used for other purposes
uninstall-hook:
	rm -f $(toolsDestDir)/MathFinder*
	rm -f $(toolsDestDir)/ScrollView.jar

