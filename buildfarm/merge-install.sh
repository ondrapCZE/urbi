#! /bin/sh

# Merge multiple (2) Urbi SDK tarballs (one debug and one release)
# and run the installer generator.

set -e
. $(dirname "$0")/common
set -x

## ----------- ##
## Functions.  ##
## ----------- ##

verb ()
{
  if $verbose; then stderr "$@"; fi
}

usage ()
{
  cat <<EOF
 usage: $0 [options]

Options:
  -i, --installprogram    name of installer generator program
                          [$installer]
  -a, --installarguments  arguments for installer
                          [$installargs]
  -s, --installscriptloc  directory of installer.nsh
                          [$installscriptloc]
  -t, --templateloc       directory with templates
                          [$templateloc]
  --vcredist              vcredist binary
                          [$vcredist]
  --compiler              version of visual studio
                          [$compiler]
  --version               version of Urbi
                          [$version]
  --gostai-console	  Gostai console installer
			  [$gostaiconsole]
  --gostai-editor	  Gostai editor installer
			  [$gostaieditor]
  -d, --debug             debug mode
  -v, --verbose           verbose mode
  -o, --output            output file
EOF
  exit 0
}

files=
verbose=false
vcredist="/home/build/share/tools/vcredist/vcredist_x86-vcxx-2008.exe"
compiler="vcxx-2008"
version=
installer="$HOME/.wine/drive_c/Program Files/NSIS/makensis.exe"
installerargs="/NOCD share/installer/installer.nsh"
# Location of installer.nsh, will create a symlink to it if set
instalscriptloc=
templateloc=
urbiconsole=
# Target
output=

while test $# -ne 0
do
  case $1 in
  (-h|--help) usage;;
  (-i|--installprogram) shift; installer=$1 ;;
  (-a|--installarguments) shift; installerargs=$1 ;;
  (-s|--installscriptloc) shift; installscriptloc=$1 ;;
  (-t|--templateloc) shift; templateloc=$1;;
  (--vcredist) shift; vcredist=$1;;
  (--comp | --compiler) shift;
     # We need vcxx-2005, not vcxx2005.
     compiler=$(echo "$1" | perl -pe 's/(vcxx)(\d+)/$1-$2/');;
  (--version) shift; version=$1;;
  (--gostai-console) shift; gostaiconsole=$1;;
  (--gostai-editor) shift; gostaieditor=$1;;
  (-d|--debug) set -x ;;
  (-v|--verbose) verbose=true ;;
  (-o|--output) shift; output=$1 ;;
  (*) files="$files $1" ;;
  esac
  shift
done

test "$(echo $files |wc -w)" -eq 2 ||
  fatal "expected exactly two files"

test -r "$vcredist" ||
  fatal "cannot read $vcredist"

verb "merging archives $files"
dir=$(mktemp -d)

if test -n "$output"; then
  trap "cd / && rm -rf $dir" 0 1 2 13 15
fi

verb "Will work in $dir"
cd $dir
mkdir temp
mkdir merge
cd temp
for f in $files; do
  verb "merging $f"
  unzip -q $f
  basedir=$(echo *)
  case $f in
    (*debug*)
    mv $basedir/bin/urbi-launch.exe $basedir/bin/urbi-launch-d.exe
    mv $basedir/bin/urbi.exe $basedir/bin/urbi-d.exe
    sed -e 's/urbi-launch/urbi-launch-d/g' $basedir/urbi.bat \
        >$basedir/urbi-d.bat
    rm $basedir/urbi.bat
    ;;
  esac
  cp -a */* ../merge
  #safer that way
  cd ..
  rm -rf temp
  mkdir temp
  cd temp
done
cd ../merge

verb "Copying vcredist from $vcredist"
cp $vcredist ./vcredist-x86.exe

if test -n "$installscriptloc"; then
  verb "Setting up symlink to $installscriptloc"
  ln -s $installscriptloc share/installer
fi

if test -n "$gostaiconsole"; then
  verb "Copying gostai-console installer from $gostaiconsole"
  cp "$gostaiconsole" ./gostai-console-installer.exe
fi

if test -n "$gostaieditor"; then
  verb "Copying gostai-editor installer from $gostaieditor"
  cp "$gostaieditor" ./gostai-editor-installer.exe
fi

if test -n "templateloc"; then
  verb "Setting up symlink to $templateloc"
  ln -s $templateloc share/templates
fi

verb "running '$installer' /D$compiler /DVERSION=$version $installerargs"
wine "$installer" "/D$compiler" "/DVERSION=$version" $installerargs

if test -n "$output"; then
  xmv "$dir/merge/gostai-engine-runtime.exe" "$output"
else
  verb "Result in $dir"
fi