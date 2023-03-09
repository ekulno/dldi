#!/bin/bash
set -e 

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

DLDI=../../Release

rm -rf tmp
mkdir -p tmp

mkdir -p tmp/add-1.dldi tmp/add-2.dldi tmp/rem-1.dldi tmp/rem-2.dldi tmp/merged.dldi

$DLDI/dldi compose -a data/add-1.ttl tmp/add-1.dldi 
$DLDI/dldi compose -a data/add-2.ttl tmp/add-2.dldi 
$DLDI/dldi compose -a data/rem-1.ttl tmp/rem-1.dldi 
$DLDI/dldi compose -a data/rem-2.ttl tmp/rem-2.dldi 

$DLDI/dldi compose -a tmp/add-1.dldi -a tmp/add-2.dldi -s tmp/rem-1.dldi -s tmp/rem-2.dldi tmp/merged.dldi


function expect_num_out_lines(){
    if [[ $1 != $2 ]]; then 
        echo "Expected $2 but got $1 lines!"
        exit 1
    fi
}
# echo "###"
# $DLDI/dldi query triples -p http://example.com/pred2 tmp/merged.dldi
expect_num_out_lines $($DLDI/dldi query terms -s tmp/merged.dldi | wc -l) 3
expect_num_out_lines $($DLDI/dldi query terms -p tmp/merged.dldi | wc -l) 2
expect_num_out_lines $($DLDI/dldi query terms -s -p tmp/merged.dldi | wc -l) 5
expect_num_out_lines $($DLDI/dldi query terms -o tmp/merged.dldi | wc -l) 5
expect_num_out_lines $($DLDI/dldi query terms -s -o tmp/merged.dldi | wc -l) 6
expect_num_out_lines $($DLDI/dldi query terms -s -p -o tmp/merged.dldi | wc -l) 8
expect_num_out_lines $($DLDI/dldi query terms -s -o -r '"2' tmp/merged.dldi | wc -l) 2
expect_num_out_lines $($DLDI/dldi query triples tmp/merged.dldi | wc -l) 6
expect_num_out_lines $($DLDI/dldi query triples -s http://example.com/t1 -p http://example.com/pred1 tmp/merged.dldi | wc -l) 2
expect_num_out_lines $($DLDI/dldi query triples -p http://example.com/pred2 tmp/merged.dldi | wc -l) 1
echo "Success"