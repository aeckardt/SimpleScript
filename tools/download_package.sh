#!/bin/bash
repo_name=$1
repo_path=$2
release=$3
target_dir=$4
download_url="https://codeload.github.com/$repo_path/tar.gz/$release"
echo "download_url: $download_url"
target_file="$release.tar.gz"
curl -o $target_file $download_url
tar -xvf $target_file
mkdir external/$repo_name
mv $target_dir/$5/* external/$repo_name
rm -rf $target_dir
