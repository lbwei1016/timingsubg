#!/bin/bash

if [ $1 -eq 1 ]; then

    make clean
    make -j32
fi

./bin/tirdf IPMES/data/preprocessed/attack.csv IPMES/data/universal_patterns/SP12_regex.json 3600 1 ./SP12_regex.run.log  IPMES/data/universal_patterns/subpatterns/SP12_regex.json

# ./bin/tirdf IPMES/data/temp/expect_no_SP8_regex.csv IPMES/data/universal_patterns/SP8_regex.json 3600 1 ./SP8_regex.run.log  IPMES/data/universal_patterns/subpatterns/SP8_regex.json > output.txt

#./bin/tirdf IPMES/data/testcases/expect_SP12.csv IPMES/data/universal_patterns/SP12.json 3600 1 ./SP12.run.log  IPMES/data/universal_patterns/subpatterns/SP12.json
#./bin/tirdf IPMES/data/temp/expect_SP8_regex.csv IPMES/data/universal_patterns/SP8_regex.json 3600 1 ./SP8.run.log  IPMES/data/universal_patterns/subpatterns/SP8_regex.json

# ./bin/tirdf IPMES/data/testcases/unexpected_SP12.csv IPMES/data/universal_patterns/SP12_regex.json 3600 1 ./SP12.run.log  IPMES/data/universal_patterns/subpatterns/SP12_regex.json
