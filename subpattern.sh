#!/bin/bash

cd ~/IPMES/ipmes-rust
RUST_LOG=info ipmes-rust -w 3600 ../data/universal_patterns/SP12.json ../data/preprocessed/attack.csv
