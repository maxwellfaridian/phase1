#!/bin/bash

command rm -r ourResults/
mkdir ourResults
command rm -r diffResults/
mkdir diffResults/

echo "Starting TEST 00"
make test00
echo ""
./test00 > ourResults/ourtest00.txt
diff ourResults/ourtest00.txt testResults/test00.txt &> diffResults/test00diff.txt
diff ourResults/ourtest00.txt testResults/test00.txt
echo "Finishing TEST 00"
echo ""
echo ""
echo ""

echo "Starting TEST 01"
make test01
echo ""
./test01 > ourResults/ourtest01.txt
diff ourResults/ourtest01.txt testResults/test01.txt &> diffResults/test01diff.txt
diff ourResults/ourtest01.txt testResults/test01.txt
echo "Finishing TEST 01"
echo ""
echo ""
echo ""

echo "Starting TEST 02"
make test02
echo ""
./test02 > ourResults/ourtest02.txt
diff ourResults/ourtest02.txt testResults/test02.txt &> diffResults/test02diff.txt
diff ourResults/ourtest02.txt testResults/test02.txt
echo "Finishing TEST 02"
echo ""
echo ""
echo ""

echo "Starting TEST 03"
make test03
echo ""
./test03 > ourResults/ourtest03.txt
diff ourResults/ourtest03.txt testResults/test03.txt &> diffResults/test03diff.txt
echo "!!! CHECK DIFF !!!"
echo "Finishing TEST 03"
echo ""
echo ""
echo ""

echo "Starting TEST 04"
make test04
echo ""
./test04 > ourResults/ourtest04.txt
diff ourResults/ourtest04.txt testResults/test04.txt &> diffResults/test04diff.txt
echo "!!! CHECK DIFF !!!"
echo "Finishing TEST 04"
echo ""
echo ""
echo ""

echo "Starting TEST 05"
make test05
echo ""
./test05 > ourResults/ourtest05.txt
diff ourResults/ourtest05.txt testResults/test05.txt &> diffResults/test05diff.txt
diff ourResults/ourtest05.txt testResults/test05.txt
echo "Finishing TEST 05"
echo ""
echo ""
echo ""

echo "Starting TEST 06"
make test06
echo ""
./test06 > ourResults/ourtest06.txt
diff ourResults/ourtest06.txt testResults/test06.txt &> diffResults/test06diff.txt
diff ourResults/ourtest06.txt testResults/test06.txt
echo "Finishing TEST 06"
echo ""
echo ""
echo ""

echo "Starting TEST 07"
make test07
echo ""
./test07 > ourResults/ourtest07.txt
diff ourResults/ourtest07.txt testResults/test07.txt &> diffResults/test07diff.txt
diff ourResults/ourtest07.txt testResults/test07.txt
echo "Finishing TEST 07"
echo ""
echo ""
echo ""

echo "Starting TEST 08"
make test08
echo ""
./test08 > ourResults/ourtest08.txt
diff ourResults/ourtest08.txt testResults/test08.txt &> diffResults/test08diff.txt
diff ourResults/ourtest08.txt testResults/test08.txt
echo "Finishing TEST 08"
echo ""
echo ""
echo ""

echo "Starting TEST 09"
make test09
echo ""
./test09 > ourResults/ourtest09.txt
diff ourResults/ourtest09.txt testResults/test09.txt &> diffResults/test09diff.txt
diff ourResults/ourtest09.txt testResults/test09.txt
echo "Finishing TEST 09"
echo ""
echo ""
echo ""

echo "Starting TEST 10"
make test10
echo ""
./test10 > ourResults/ourtest10.txt
diff ourResults/ourtest10.txt testResults/test10.txt &> diffResults/test10diff.txt
echo "!!! CHECK DIFF !!!"
echo "Finishing TEST 10"
echo ""
echo ""
echo ""

echo "Starting TEST 11"
make test11
echo ""
./test11 > ourResults/ourtest11.txt
diff ourResults/ourtest11.txt testResults/test11.txt &> diffResults/test11diff.txt
diff ourResults/ourtest11.txt testResults/test11.txt
echo "Finishing TEST 11"
echo ""
echo ""
echo ""

echo "Starting TEST 12"
make test12
echo ""
./test12 > ourResults/ourtest12.txt
diff ourResults/ourtest12.txt testResults/test12.txt &> diffResults/test12diff.txt
diff ourResults/ourtest12.txt testResults/test12.txt
echo "Finishing TEST 12"
echo ""
echo ""
echo ""

echo "Starting TEST 13"
make test13
echo ""
./test13 > ourResults/ourtest13.txt
diff ourResults/ourtest13.txt testResults/test13.txt &> diffResults/test13diff.txt
echo "!!! CHECK DIFF !!!"
echo "Finishing TEST 13"
echo ""
echo ""
echo ""

echo "Starting TEST 14"
make test14
echo ""
./test14 > ourResults/ourtest14.txt
diff ourResults/ourtest14.txt testResults/test14.txt &> diffResults/test14diff.txt
echo "!!! CHECK DIFF !!!"
echo "Finishing TEST 14"
echo ""
echo ""
echo ""

echo "Starting TEST 15"
make test15
echo ""
./test15 > ourResults/ourtest15.txt
diff ourResults/ourtest15.txt testResults/test15.txt &> diffResults/test15diff.txt
echo "!!! CHECK DIFF !!!"
echo "Finishing TEST 15"
echo ""
echo ""
echo ""

echo "Starting TEST 16"
make test16
echo ""
./test16 > ourResults/ourtest16.txt
diff ourResults/ourtest16.txt testResults/test16.txt &> diffResults/test16diff.txt
echo "!!! CHECK DIFF !!!"
echo "Finishing TEST 16"
echo ""
echo ""
echo ""

echo "Starting TEST 17"
make test17
echo ""
./test17 > ourResults/ourtest17.txt
diff ourResults/ourtest17.txt testResults/test17.txt &> diffResults/test17diff.txt
diff ourResults/ourtest17.txt testResults/test17.txt
echo "Finishing TEST 17"
echo ""
echo ""
echo ""

echo "Starting TEST 18"
make test18
echo ""
./test18 > ourResults/ourtest18.txt
diff ourResults/ourtest18.txt testResults/test18.txt &> diffResults/test18diff.txt
diff ourResults/ourtest18.txt testResults/test18.txt
echo "Finishing TEST 18"
echo ""
echo ""
echo ""

echo "Starting TEST 19"
make test19
echo ""
./test19 > ourResults/ourtest19.txt
diff ourResults/ourtest19.txt testResults/test19.txt &> diffResults/test19diff.txt
diff ourResults/ourtest19.txt testResults/test19.txt
echo "Finishing TEST 19"
echo ""
echo ""
echo ""

echo "Starting TEST 20"
make test20
echo ""
./test20 > ourResults/ourtest20.txt
diff ourResults/ourtest20.txt testResults/test20.txt &> diffResults/test20diff.txt
diff ourResults/ourtest20.txt testResults/test20.txt
echo "Finishing TEST 20"
echo ""
echo ""
echo ""

echo "Starting TEST 21"
make test21
echo ""
./test21 > ourResults/ourtest21.txt
diff ourResults/ourtest21.txt testResults/test21.txt &> diffResults/test21diff.txt
diff ourResults/ourtest21.txt testResults/test21.txt
echo "Finishing TEST 21"
echo ""
echo ""
echo ""

echo "Starting TEST 22"
make test22
echo ""
./test22 > ourResults/ourtest22.txt
diff ourResults/ourtest22.txt testResults/test22.txt &> diffResults/test22diff.txt
diff ourResults/ourtest22.txt testResults/test22.txt
echo "Finishing TEST 22"
echo ""
echo ""
echo ""

# echo "Starting TEST 23"
# make test23
# echo ""
# ./test23 > ourResults/ourtest23.txt
# diff ourResults/ourtest23.txt testResults/test23.txt &> diffResults/test23diff.txt
# echo "!!! CHECK DIFF !!!"
# echo "Finishing TEST 23"
# echo ""
# echo ""
# echo ""




