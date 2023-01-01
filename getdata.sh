#!/bin/bash

# beware train and validation datasets are not disjunct and they differ in the 3rd column
make_col3_unknown() {
    local unknown_val=$1
    local from_file=$2
    local to_file=$3
    while IFS=';' read -ra fields; do 
        if ! [ -z "${fields[*]}" ]; then
            fields[2]=$unknown_val
            let n=${#fields[@]}
            let n1=${#fields[@]}-1
            for (( i=0 ; i<$n1 ; i++ ));
            do
                printf "%s;" ${fields[$i]}
            done
            printf "%s\n" ${fields[$n1]}
        fi
    done <$from_file >$to_file
}

# read train data
rm -f data/da.zip ||:
rm -f data/produkt* ||:
rm -f data/train* ||:
wget "https://www.dwd.de/DE/leistungen/_config/leistungsteckbriefPublication.zip?view=nasPublication&nn=16102&imageFilePath=157242051950877752011408908330930139598321949458353665338080907323067407358570939094063695252196106887026681320814526060536595251033063153662108661981192575715848535028223489905255018373803265100435386215906807083070634698592787432294483667495414514899300397184707292279&download=true" -O data/da.zip
rm -f data/train* ||:
unzip -c data/da.zip "produkt*tag*" > data/train_raw.txt
tail -n +4 data/train_raw.txt > data/train0.txt # skip head rows
make_col3_unknown "-999" data/train0.txt data/train.txt


# read validation data
rm -f data/dc.zip ||:
rm -f data/produkt*tag* ||:
rm -f data/validation* ||:
wget "https://www.dwd.de/DE/leistungen/_config/leistungsteckbriefPublication.zip?view=nasPublication&nn=16102&imageFilePath=157242051950877752011408908330930139598321949458353665338080907323067407358570939094063695252196106887026681320814526060536595251033063153662108661981192575715848535028223489905255018373803265100435386215906807083070634698592787432294483667495414514770162778492648647799&download=true" -O data/dc.zip
unzip -c data/dc.zip "produkt*tag*" > data/validation_raw.txt
tail -n +4 data/validation_raw.txt > data/validation0.txt
make_col3_unknown "-999" data/validation0.txt data/validation.txt

# glue train and validation to the complete data
rm -f data/complete* ||:
cat -s data/train.txt data/validation.txt | awk '!/^$/'| sort -u > data/complete.txt