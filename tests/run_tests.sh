#!/bin/bash

help()
{
    echo "Usage: ./run_tests.sh [FLAGS]"
    echo "      -t <TYPE> --type <TYPE>"
    echo "          Type of tests to run: valid or invalid"
    echo "      -m <PATTERN> --match <PATTERN>"
    echo "          Match test filenames to PATTERN"
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            TYPE=$2
            shift
            shift
            ;;
        -m|--match)
            MATCH=$2
            shift
            shift
            ;;
        -h|--help)
            help
            exit 0
            ;;
        *)
            echo "Unknown flag $1"
            help
            exit 1
    esac
done

if [[ -z $MATCH ]]; then
    MATCH=""
fi

if [[ ! -z $TYPE && "$TYPE" != "valid" && "$TYPE" != "invalid" ]]; then
    echo "-t <TYPE>: has to be either \"valid\" or \"invalid\""
    exit 1
fi

PASSED=0
FAILED=0
TOTAL=0

RED='\033[0;31m'
YELLOW='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m'

DIR=$(realpath $(realpath $0)/../..)
LOG=$DIR/tests/test.log
BINARY=$DIR/tests/test

cd $DIR
make test > $LOG

cd $DIR/tests
echo "$(date)" >> $LOG
echo "=============" >> $LOG
echo "Running tomlibc tests" | tee $LOG
echo "==================" | tee $LOG

JSON=test.json
# valid
if [[ -z $TYPE || "$TYPE" == "valid" ]]; then
    echo "VALID" >> $LOG
    echo "=====" >> $LOG
    echo >> $LOG
    for test in $(ls valid/*$MATCH*.toml);
    do
        TOTAL=$(( TOTAL+1 ))
        echo $test >> $LOG
        echo "=================" >> $LOG
        echo >> $LOG
        $BINARY $test 2>>$LOG | tee $JSON >/dev/null
        test $? -eq 0 || echo "$test: [FAILED]"
        json="${test%.toml}.json"
        diff <(jq --sort-keys . $json 2>>$LOG) <(jq --sort-keys . $JSON 2>>$LOG) >> $LOG
        r=$?
        test $r -eq 0 && PASSED=$(( PASSED+1 )) || FAILED=$(( FAILED+1 ))
        test $r -eq 0 && printf "[${GREEN}PASSED${NC}]: ${test}\n" || printf "[${RED}FAILED${NC}]: ${test}\n"
        echo >> $LOG
    done
fi

# invalid
if [[ -z $TYPE || "$TYPE" == "invalid" ]]; then
    echo "INVALID" >> $LOG
    echo "=======" >> $LOG
    echo >> $LOG
    for test in $(ls invalid/*$MATCH*.toml);
    do
        TOTAL=$(( TOTAL+1 ))
        echo $test >> $LOG
        echo "=================" >> $LOG
        echo >> $LOG
        $BINARY $test >> $LOG 2>/dev/null
        r=$?
        test $r -eq 1 && PASSED=$(( PASSED+1 )) || FAILED=$(( FAILED+1 ))
        test $r -eq 1 && printf "[${GREEN}PASSED${NC}]: ${test}\n" || printf "[${RED}FAILED${NC}]: ${test}\n"
        echo >> $LOG
    done
fi

percentage=$(echo "scale=2; $PASSED*100/$TOTAL" | bc)
echo
printf "[${YELLOW}REPORT${NC}]: Passed ${PASSED} tests | Failed ${FAILED} tests | Percent: ${percentage}%%" | tee -a $LOG
echo

rm -f $JSON
