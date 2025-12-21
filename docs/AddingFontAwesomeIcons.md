# Adding FontAwesome Icons

This guide explains how to add new FontAwesome icons to the Cuckoo project.

## Prerequisites

- **npm** must be installed on your system
- **lv_font_conv** package should be installed globally:
  ```bash
  npm install -g lv_font_conv
  ```

## Steps to Add a New Icon

### 1. Find the Icon Code

Visit [FontAwesome's icon library](https://fontawesome.com/icons) and find the icon you want to add. Note down:
 - Its Unicode code point (the hexadecimal value, e.g., `0xF001` or `61441` in decimal).
 - The font it belongs to (e.g: `fa-brands`,`fa-solid`)

### 2. Update the build_icons.sh Script

Open [build_icons.sh](../build_icons.sh) and add a new `--range` line for your icon:

```bash
--range 61441 \    # existing icon
--range 61448 \    # existing icon
--range YOUR_CODE \ # add your new icon code here
```

**Important Notes:**
- Add the line in numerical order for better organization
- Use the decimal value of the Unicode code point
- For **solid** icons: add under the `fa-solid-900.ttf` font section
- For **brand** icons: add under the `fa-brands-400.ttf` font section
- Keep the backslash `\` at the end for line continuation

### 3. Run the Icon Build Script

Execute the script to generate the updated font file:

```bash
./build_icons.sh
```

This will:
- Download FontAwesome if not already present
- Generate a new `CuckooFontAwesome.c` file with your icon included
- Copy the generated file to `src/fonts/`

### 4. Update the Icon Definitions Header

Open [src/fonts/CuckooFontAwesomeDefs.h](../src/fonts/CuckooFontAwesomeDefs.h) and add a definition for your new icon:

```c
#define CUCKOO_SYMBOL_YOUR_ICON_NAME  "\xEF\x80\x81" /*61441, 0xF001*/
```

To get the UTF-8 encoding:
- Use an online converter to convert the Unicode code point to UTF-8 bytes
- Or reference similar icons in the file to see the pattern

**Format:**
- Name should be descriptive and in UPPERCASE
- Use the `CUCKOO_SYMBOL_` prefix
- Include a comment with both decimal and hexadecimal values

### 5. Commit Changes to Git

**Important:** The font generation is **NOT** part of the build process. You must commit all generated files:

```bash
git add build_icons.sh
git add src/fonts/CuckooFontAwesome.c
git add src/fonts/CuckooFontAwesomeDefs.h
git commit -m "Add new FontAwesome icon: YOUR_ICON_NAME"
```

## Files Modified

When adding a new icon, these files will be changed:

1. `build_icons.sh` - Icon code added to build script
2. `src/fonts/CuckooFontAwesome.c` - Generated font file (auto-generated)
3. `src/fonts/CuckooFontAwesomeDefs.h` - Icon definition macro (manual)

## Using Your New Icon in Code

Once added, you can use your icon in the code:

```c
#include "fonts/CuckooFontAwesomeDefs.h"

// Use the icon
lv_obj_set_style_text_font(label, &CuckooFontAwesome, 0);
lv_label_set_text(label, CUCKOO_SYMBOL_YOUR_ICON_NAME);
```

## Troubleshooting

### "lv_font_conv: command not found"
Install the package globally:
```bash
npm install -g lv_font_conv
```

### Icon not appearing correctly
- Verify the Unicode code point is correct
- Check that you're using the correct font (solid vs brands)
- Ensure the UTF-8 encoding in the header file is correct

### Build fails after adding icon
- Make sure the backslash `\` is present at the end of each line in build_icons.sh
- Verify the script runs successfully before committing
