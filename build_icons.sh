#!/bin/sh
mkdir -p icons
cd icons

#Donwload FontAwesome if not already present
if [ -f "fontawesome-free-6.7.2-web.zip" ]; then
    echo "FontAwesome already downloaded."
else
    echo "Downloading FontAwesome..."
    wget https://use.fontawesome.com/releases/v6.7.2/fontawesome-free-6.7.2-web.zip -O fontawesome-free-6.7.2-web.zip
fi

#Unzip FontAwesome
unzip -o fontawesome-free-6.7.2-web.zip

#Convert FontAwesome to C array for LVGL
lv_font_conv \
    --bpp 4 \
    --size 28 \
    --no-compress \
    --font ./fontawesome-free-6.7.2-web/webfonts/fa-solid-900.ttf \
    --range 61441 \
    --range 61448 \
    --range 61451 \
    --range 61452 \
    --range 61453 \
    --range 61457 \
    --range 61459 \
    --range 61461 \
    --range 61465 \
    --range 61468 \
    --range 61473 \
    --range 61478 \
    --range 61479 \
    --range 61480 \
    --range 61502 \
    --range 61507 \
    --range 61512 \
    --range 61515 \
    --range 61516 \
    --range 61517 \
    --range 61521 \
    --range 61522 \
    --range 61523 \
    --range 61524 \
    --range 61543 \
    --range 61544 \
    --range 61550 \
    --range 61552 \
    --range 61553 \
    --range 61556 \
    --range 61559 \
    --range 61560 \
    --range 61561 \
    --range 61563 \
    --range 61587 \
    --range 61589 \
    --range 61617 \
    --range 61636 \
    --range 61637 \
    --range 61639 \
    --range 61641 \
    --range 61664 \
    --range 61671 \
    --range 61674 \
    --range 61675 \
    --range 61683 \
    --range 61724 \
    --range 61732 \
    --range 61784 \
    --range 61931 \
    --range 62016 \
    --range 62017 \
    --range 62018 \
    --range 62019 \
    --range 62020 \
    --range 62153 \
    --range 62189 \
    --range 62212 \
    --range 62810 \
    --range 63426 \
    --range 63587 \
    --font ./fontawesome-free-6.7.2-web/webfonts/fa-brands-400.ttf \
    --range 62087 \
    --range 62099 \
    --format lvgl \
    -o CuckooFontAwesome.c

#Move generated font to source folder
cp CuckooFontAwesome.c ../src/fonts