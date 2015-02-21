## This .pro file is used to create a Makefile with the necessary rules
## to create an initial translations template and to update it whenever
## there are new translatable strings in the project that are ready to be
## exposed to translators
##
## See the ../README.translations file for more information

TEMPLATE = subdirs

PROJECTNAME = $$system(basename ../*.qmlproject)
PROJECTNAME = $$replace(PROJECTNAME,.qmlproject,)

DESKTOPFILE = ../io.papyros.files.desktop.in

SOURCECODE = ../*.qml

BUILDDIR = ../.build
DESKTOPFILETEMP = $${BUILDDIR}/com.ubuntu.filemanager_filemanager.desktop.in

message("")
message(" Project Name: $$PROJECTNAME ")
message(" Source Code: $$SOURCECODE ")
message("")
message(" Run 'make pot' to generate the pot file from source code. ")
message("")

## Generate pot file 'make pot'
potfile.target = pot
potfile.commands = xgettext \
                   -o $${PROJECTNAME}.pot \
		   --copyright=\"Canonical Ltd. \" \
		   --package-name $${PROJECTNAME} \
		   --qt --c++ --add-comments=TRANSLATORS \
		   --keyword=tr --keyword=tr:1,2 \
		   $${SOURCECODE} $${DESKTOPFILETEMP}
potfile.depends = desktopfile
QMAKE_EXTRA_TARGETS += potfile

## Do not use this rule directly. It's a dependency rule to
## generate an intermediate .js file to extract translatable
## strings from the .desktop file
desktopfile.target = desktopfile
desktopfile.commands = awk \'BEGIN { FS=\"=\" }; /Name/ {print \"var s = i18n.tr(\42\" \$$2 \"\42);\"}\' $${DESKTOPFILE} > $${DESKTOPFILETEMP}
desktopfile.depends = makebuilddir
QMAKE_EXTRA_TARGETS += desktopfile

## Dependency rule to create the temporary build dir
makebuilddir.target = makebuilddir
makebuilddir.commands = mkdir -p $${BUILDDIR}
QMAKE_EXTRA_TARGETS += makebuilddir

## Rule to clean the products of the build
clean.target = clean
clean.commands = rm -Rf $${BUILDDIR}
QMAKE_EXTRA_TARGETS += clean
