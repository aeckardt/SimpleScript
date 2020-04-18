#!/bin/bash
repo_path=$1
repo_name=$2
release=$3
target_dir="$2-$3"
download_url="https://codeload.github.com/$repo_path/tar.gz/$release"
target_file="$release.tar.gz"
curl -o $target_file $download_url
result=$?
if [[ $result -ne 0 ]]
then
  exit $result
fi
tar -xvf $target_file
result=$?
if [[ $result -ne 0 ]]
then
  exit $result
fi
if [[ ! -d "external" ]]
then
  mkdir external
fi
mkdir external/$repo_name
mv $target_dir/* external/$repo_name
rm -rf $target_dir
exit $result
