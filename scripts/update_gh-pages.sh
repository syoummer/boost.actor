#!/bin/bash

echo "check for folders ..."
if [ ! -d manual ]; then
    echo "no manual folder found"
    exit
fi

if [ ! -d ../boost.actor.gh-pages ]; then
    echo "no gh-pages folder found"
    exit
fi

echo "build documentation ..."
make doc &>/dev/null

if [ -f manual.pdf ]; then
    echo "PDF manual found ..."
else
    echo "no PDF manual found ... stop"
    exit
fi

echo "build HTML manual ..."
cd manual/
# runs hevea three times
make html &>/dev/null

echo "copy documentation into gh-pages ..."
cd ../../boost.actor.gh-pages
rm -f *.tex *.html *.css *.png *.js manual/manual.pdf manual/index.html
cp -R ../boost.actor/html/* .
mkdir manual
cp ../boost.actor/manual.pdf manual/
cp ../boost.actor/manual/manual.html manual/index.html

echo "commit ..."
git add .
git commit -a -m "documentation update"

echo "push ..."
git push

cd ../boost.actor
