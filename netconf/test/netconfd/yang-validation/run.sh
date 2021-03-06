#!/bin/bash

./update-module-repository.sh

if [ "$RUN_WITH_CONFD" != "" ] ; then
    # skipped test return value
    exit 77
fi

rm -rf tmp || true
mkdir tmp

OKS=0
FAILS=0

for dst in ietf ietf-draft ; do
for module in `ls ../../../modules/${dst}` ; do
  echo $module
  is_submodule="`head -n 1 ../../../modules/${dst}/${module} | grep submodule`" || true
  if [ "${is_submodule}" != "" ] ; then
    echo "Skip submodule: ${module}"
    continue
  fi
  cmd="/usr/sbin/netconfd --validate-config-only --startup-error=stop --module=../../../modules/${dst}/${module} --no-startup --modpath=../../../modules"
  echo $cmd
  $cmd
  ret=$?
  if [ "${ret}" = "0" ] ; then
    OKS=$((${OKS}+1))
    echo OK: $module
  else
    FAILS=$((${FAILS}+1))
    echo FAIL: $module
  fi
done
done
  echo OKS=$OKS
  echo FAILS=$FAILS
exit $FAILS
