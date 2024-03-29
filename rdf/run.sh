#!/bin/bash

make clean
make -j32
./bin/tirdf IPMES/data/testcases/expect_SP12.csv IPMES/data/universal_patterns/SP12.json 3600 1 ./SP12.run.log  IPMES/data/universal_patterns/subpatterns/SP12.json


# ./bin/tirdf IPMES/data/testcases/unexpected_SP12.csv IPMES/data/universal_patterns/SP12.json 3600 1 ./SP12.run.split.log  IPMES/data/universal_patterns/subpatterns/SP12.json
