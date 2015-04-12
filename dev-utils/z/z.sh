#!/bin/sh

dir_name=$1

real_dir=$(grep "^$dir_name:" ~/.z/path.db | cut -d ':' -f 2)

echo $real_dir
cd $real_dir
