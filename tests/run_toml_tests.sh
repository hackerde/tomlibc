#!/bin/bash

help()
{
    echo "Usage: ./run_tests.sh [FLAGS]"
    echo "      -t <TYPE> --type <TYPE>"
    echo "          Type of tests to run: valid or invalid"
    echo "      -m <PATTERN> --match <PATTERN>"
    echo "          Match test filenames to PATTERN"
    echo "      -r --regression"
    echo "          Calculate and show regression report"
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
        -r|--regression)
            REGRESSION=1
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

TOML_VERSION=1.0.0

PASSED=()
FAILED=()
TOTAL=0

RED='\033[0;31m'
YELLOW='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m'

DIR=$(realpath $(realpath $0)/../..)
LOG=$DIR/tests/toml-test.log
OLD_STATUS=$DIR/tests/toml-test.status
NEW_STATUS=$OLD_STATUS.new
BINARY=$DIR/tests/test

cd $DIR
> $NEW_STATUS
make test > $LOG

# git clone https://github.com/toml-lang/toml-test.git tests/toml-tests
TOML_TEST_DIR=$DIR/tests/toml-test/tests
cd $DIR/tests
echo "$(date)" >> $LOG
echo "=============" >> $LOG
echo "Running toml-test tests" | tee $LOG
echo "==================" | tee $LOG

JSON=toml-test.json
# valid
if [[ -z $TYPE || "$TYPE" == "valid" ]]; then
    echo "VALID" >> $LOG
    echo "=====" >> $LOG
    echo >> $LOG
    for test in $(cat $TOML_TEST_DIR/files-toml-$TOML_VERSION);
    do
        if [[ 
            "$test" == "valid/"* && 
            "$test" == *"$MATCH"* && 
            "$test" == *".toml" && 
            -f $TOML_TEST_DIR/$test 
        ]]; then
            TOTAL=$(( TOTAL+1 ))
            echo $test >> $LOG
            echo "=================" >> $LOG
            echo >> $LOG
            $BINARY < $TOML_TEST_DIR/$test 2>>$LOG | tee $JSON >/dev/null
            test $? -eq 0 || echo "$test: [FAILED]"
            t=$TOML_TEST_DIR/$test
            json="${t%.toml}.json"
            diff <(jq --sort-keys . $json 2>>$LOG) <(jq --sort-keys . $JSON 2>>$LOG) >> $LOG
            r=$?
            if [[ $r -eq 0 ]]; then
                PASSED+=( $test )
                printf "[${GREEN}PASSED${NC}]: ${test}\n"
                echo $test >> $NEW_STATUS
            else
                FAILED+=( $test )
                printf "[${RED}FAILED${NC}]: ${test}\n"
            fi
            echo >> $LOG
        fi
    done
fi

# invalid
if [[ -z $TYPE || "$TYPE" == "invalid" ]]; then
    echo "INVALID" >> $LOG
    echo "=======" >> $LOG
    echo >> $LOG
    for test in $(cat $TOML_TEST_DIR/files-toml-$TOML_VERSION);
    do
        if [[
            "$test" == "invalid/"* && 
            "$test" == *"$MATCH"* && 
            "$test" == *".toml" && 
            -f $TOML_TEST_DIR/$test
        ]]; then
            TOTAL=$(( TOTAL+1 ))
            echo $test >> $LOG
            echo "=================" >> $LOG
            echo >> $LOG
            $BINARY < $TOML_TEST_DIR/$test >> $LOG 2>/dev/null
            r=$?
            if [[ $r -eq 1 ]]; then
                PASSED+=( $test )
                printf "[${GREEN}PASSED${NC}]: ${test}\n"
                echo $test >> $NEW_STATUS
            else
                FAILED+=( $test )
                printf "[${RED}FAILED${NC}]: ${test}\n"
            fi
            echo >> $LOG
        fi
    done
fi

if [[ $TOTAL -gt 0 ]]; then
    percentage=$(echo "scale=2; ${#PASSED[@]}*100/$TOTAL" | bc)
    echo | tee -a $LOG
    printf "[${YELLOW}TEST REPORT${NC}]: Passed ${#PASSED[@]} tests | Failed ${#FAILED[@]} tests | Percent: ${percentage}%%" | tee -a $LOG
    echo | tee -a $LOG
else
    echo "Did not run any tests." | tee -a $LOG
fi

rm -f $JSON

# regression
if [[ ! -z $REGRESSION && $REGRESSION -eq 1 ]]; then
    if [[ ! -f $OLD_STATUS ]]; then
        echo "Regression reports will be effective from the next run."
        cp $NEW_STATUS $OLD_STATUS
        exit 0
    fi
    STATUS=()
    PASSED_NEW=()
    FAILED_NEW=()
    for test in $(cat $OLD_STATUS); do
        STATUS+=( $test )
    done
    for test in ${PASSED[@]}; do
        if [[ ! ${STATUS[@]} =~ $test ]]; then
            PASSED_NEW+=( $test )
        fi
    done
    for test in ${FAILED[@]}; do
        if [[ ${STATUS[@]} =~ $test ]]; then
            FAILED_NEW+=( $test )
        fi
    done
    if [[ ${#PASSED_NEW[@]} -gt 0 ]]; then
        echo | tee -a $LOG
        printf "[${GREEN}NEW PASSING${NC}]" | tee -a $LOG
        echo | tee -a $LOG
        printf '%s\n' "${PASSED_NEW[@]}" | tee -a $LOG
    fi
    if [[ ${#FAILED_NEW[@]} -gt 0 ]]; then
        echo | tee -a $LOG
        printf "[${RED}NEW FAILING${NC}]" | tee -a $LOG
        echo | tee -a $LOG
        printf '%s\n' "${FAILED_NEW[@]}" | tee -a $LOG
    fi
    echo | tee -a $LOG
    printf "[${YELLOW}REGRESSION REPORT${NC}]: Passed ${#PASSED_NEW[@]} new tests | Failed ${#FAILED_NEW[@]} new tests" | tee -a $LOG
    echo | tee -a $LOG
fi
