get_i2c_control()
{
    if [ "$#" -eq 0 ]; then
        exit 1
    fi

    retries=10
    change=1
    reg_value=$(i2cget -f -y $1 $2 1)
    reg_int=$((reg_value))

    #last 4-bits of master selector control register
    value=$((reg_int & 0x0f))
    a=0

    while [ $a -le "$retries" ]
    do
        a=$(($a+1))
        if [ "$value" -eq 0 ] || [ "$value" -eq 1 ] ||[ "$value" -eq 5 ]; then
            val=4
        elif [ "$value" -eq 6 ] || [ "$value" -eq 3 ] || [ "$value" -eq 2 ]; then
            val=5
        elif [ "$value" -eq 12 ] || [ "$value" -eq 9 ] || [ "$value" -eq 13 ]; then
            val=0
        elif [ "$value" -eq 10 ] || [ "$value" -eq 14 ] || [ "$value" -eq 15 ]; then
            val=1
        else
            change=0
        fi

        write_val=$((val))
        if [ "$change" -eq 1 ]; then
            i2cset -y $1 $2 0x1 $write_val
        fi

        #check the master seletor control register
        reg_value1=$(i2cget -f -y $1 $2 1)
        reg_int1=$((reg_value1))

        #last 4-bits of master selector control register
        value1=$((reg_int1 & 0x0f))

        if [ "$value1" -eq 4 ] || [ "$value1" -eq 7 ] || [ "$value1" -eq 8 ] || [ "$value1" -eq 11 ]; then
            break
        fi

        udelay=$RANDOM
        usleep $udelay

    done
}
