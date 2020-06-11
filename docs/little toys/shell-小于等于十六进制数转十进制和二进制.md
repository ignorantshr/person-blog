`num2dec2bin.sh`

```shell
#!/bin/sh
# 小于等于十六进制数转十进制、二进制

[ -z $1 ] && echo "please provides a number in \$1." && exit 1
[ -z $2 ] || [ $2 -gt 16 ] && echo "please provides radix(<=16) in \$2." && exit 1

num=$1
num_len=${#num}
radix=$2

num=$(echo "${num}" | tr [A-F] [a-f])

h2da=10
h2db=11
h2dc=12
h2dd=13
h2de=14
h2df=15

y=0
dec_num=0
for i in $(seq ${num_len})
do
        remainder=${num:0-1:1}
        num=${num:0:-1}
        let x=(radix ** y)
        if [ "$1" -ge 0 ] 2>/dev/null
        then
                let 1
        else
                remainder=$(eval echo \${h2d${remainder}})
        fi
        let dec_num=(dec_num + remainder * x)
        # dec_num=$(expr ${dec_num} + $(expr ${remainder} \* $x))
        let y+=1
done

echo "decimal(10): ${dec_num}"

bin_num=
y=0
num_len=${#dec_num}
if [[ ${dec_num} -eq 0 ]]
then
        echo "binary(2): 0, length: ${#bin_num}"
else
        while [ ${dec_num} -ne 0 ]
        do
                remainder=$(expr $dec_num % 2)
                let dec_num/=2
                bin_num="${remainder}${bin_num}"
                let y+=1
        done
        echo "binary(2): ${bin_num}, length: ${#bin_num}"
fi
```

