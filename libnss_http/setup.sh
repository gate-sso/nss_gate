#!/bin/sh
if test "$#" -ne 2; then
  echo "Usage: setup.sh <hostname> <token>"
  echo "Not enough arguments"
  exit 1
fi
mkdir -p /etc/gate
cp nss.yml.tmpl /etc/gate/nss.yml
sed -e "s/\${hostname}/$1/" -e "s/\${token}/$2/" /etc/gate/nss.yml > /etc/gate/nss.yml.1
mv /etc/gate/nss.yml.1 /etc/gate/nss.yml

