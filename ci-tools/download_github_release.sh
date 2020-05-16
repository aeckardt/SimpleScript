#!/bin/bash
account=$1
project=$2
release=$3
download_url="https://codeload.github.com/$account/$project/tar.gz/$release"
target_file="$project.tar.gz"
install_dir="$project-$release"
curl "$download_url" -o "$target_file"
result=$?
if [[ $result -ne 0 ]]; then exit $result; fi;
tar -xvf $target_file
result=$?
if [[ $result -ne 0 ]]; then exit $result; fi;
if [[ ! -d "external" ]]; then mkdir external; fi;
mkdir "external/$project"
mv $install_dir/* external/$project
rm -f $target_file
rm -rf $install_dir
exit $result
