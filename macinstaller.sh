rm -rf ~/installer/Applications/*
cp -R /Users/daniel347x/newgene/NewGeneUI/build-NewGene-Desktop_Qt_5_7_0_clang_64bit-Release/NewGene.app ~/installer/Applications/
rm -rf ~/packages/input/*
rm -rf ~/packages/output/*
cd ~/installer/Applications/
dsymutil NewGene.app/Contents/MacOS/NewGene -o NewGene.app.dSYM
/Users/daniel347x/Qt/5.7/clang_64/bin/macdeployqt NewGene.app
cp /Users/daniel347x/boost_1_62_0/stage/lib/libboost_filesystem.dylib NewGene.app/Contents/Plugins/
cp /Users/daniel347x/boost_1_62_0/stage/lib/libboost_regex.dylib NewGene.app/Contents/Plugins/
cp /Users/daniel347x/boost_1_62_0/stage/lib/libboost_thread.dylib NewGene.app/Contents/Plugins/
cp /Users/daniel347x/boost_1_62_0/stage/lib/libboost_system.dylib NewGene.app/Contents/Plugins/
cp /Users/daniel347x/boost_1_62_0/stage/lib/libboost_date_time.dylib NewGene.app/Contents/Plugins/
cp /Users/daniel347x/boost_1_62_0/stage/lib/libboost_locale.dylib NewGene.app/Contents/Plugins/
install_name_tool -change "libboost_filesystem.dylib" "@executable_path/../Plugins/libboost_filesystem.dylib" NewGene.app/Contents/MacOS/NewGene
install_name_tool -change "libboost_regex.dylib" "@executable_path/../Plugins/libboost_regex.dylib" NewGene.app/Contents/MacOS/NewGene
install_name_tool -change "libboost_system.dylib" "@executable_path/../Plugins/libboost_system.dylib" NewGene.app/Contents/MacOS/NewGene
install_name_tool -change "libboost_date_time.dylib" "@executable_path/../Plugins/libboost_date_time.dylib" NewGene.app/Contents/MacOS/NewGene
install_name_tool -change "libboost_locale.dylib" "@executable_path/../Plugins/libboost_locale.dylib" NewGene.app/Contents/MacOS/NewGene
install_name_tool -change "libboost_thread.dylib" "@executable_path/../Plugins/libboost_thread.dylib" NewGene.app/Contents/MacOS/NewGene
install_name_tool -id "@executable_path/../Plugins/libboost_filesystem.dylib" NewGene.app/Contents/Plugins/libboost_filesystem.dylib
install_name_tool -id "@executable_path/../Plugins/libboost_regex.dylib" NewGene.app/Contents/Plugins/libboost_regex.dylib
install_name_tool -id "@executable_path/../Plugins/libboost_system.dylib" NewGene.app/Contents/Plugins/libboost_system.dylib
install_name_tool -id "@executable_path/../Plugins/libboost_date_time.dylib" NewGene.app/Contents/Plugins/libboost_date_time.dylib
install_name_tool -id "@executable_path/../Plugins/libboost_locale.dylib" NewGene.app/Contents/Plugins/libboost_locale.dylib
install_name_tool -id "@executable_path/../Plugins/libboost_thread.dylib" NewGene.app/Contents/Plugins/libboost_thread.dylib
install_name_tool -change "libboost_system.dylib" "@executable_path/../Plugins/libboost_system.dylib" NewGene.app/Contents/Plugins/libboost_filesystem.dylib
install_name_tool -change "libboost_system.dylib" "@executable_path/../Plugins/libboost_system.dylib" NewGene.app/Contents/Plugins/libboost_locale.dylib
install_name_tool -change "libboost_system.dylib" "@executable_path/../Plugins/libboost_system.dylib" NewGene.app/Contents/Plugins/libboost_thread.dylib
cd ~
pkgbuild --root ./installer/NewGene --ownership preserve --install-location ~/Documents/NewGene --identifier com.newgene.data --version 1.64 ./packages/input/NewGene-data-1.64.pkg
pkgbuild --component ./installer/Applications/NewGene.app --ownership preserve --install-location /Applications --identifier com.newgene.app --version 1.64 ./packages/input/NewGene-app-1.64.pkg
productbuild --synthesize --package ./packages/input/NewGene-data-1.64.pkg --package ./packages/input/NewGene-app-1.64.pkg ./packages/input/distribution-newgene-1.64.xml
productbuild --distribution ./packages/input/distribution-newgene-1.64.xml --package-path ./packages/input ./packages/output/NewGene-1.64.pkg
