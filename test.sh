#!/bin/sh

orig_file=$(grep file_path calc_file_size.ini | awk '{print $3}')
#app_pid=$(ps -ef | grep ex09 | grep -v grep | awk '{print $2}')
app_pid=$(cat /tmp/calc_file_size.pid)
files=$(ls -1p | grep -v /)

last_file=${orig_file}
for cur_file in ${files}; do
    sed -i "s/${last_file}/${cur_file}/g" calc_file_size.ini
    kill -1 ${app_pid} 
    nc -U /tmp/calc_file_size.sock
    last_file=${cur_file}
done

sed -i "s/${last_file}/${orig_file}/g" calc_file_size.ini
kill ${app_pid}
