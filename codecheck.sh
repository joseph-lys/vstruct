cpplint --linelength=120 --root=include include/*.h include/**/*.h
cpplint --linelength=120 test/*.h test/**/*.h test/*.cpp test/**/*.cpp
cppcheck test
cppcheck include


