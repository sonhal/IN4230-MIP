#!/bin/bash
 
### Print total arguments and their values
 
echo "Total Arguments:" $#
echo "All Arguments values:" $@
echo "[-d] <app_socket> <route_socket> <forward_socket> [MIP ADDRESSES...]"

&bin/mipd $@

args=("$@")
# Basic if statement
if [ $1 == "-d" ]
then
    unset 'args[1]'
    
else
    unset 'args[0]'
fi
&bin/routerd ${args[@]}
