# PC-FX_Hexview

This is a  program which allows you to page through memory on the PC-FX, browsing data in hexadecimal

## Function

This was written as a utility, which may or may not have much function by itself, but
can certainly be helpful as both (a) sample code, and (b) a tester, or a test portion
of a larger process.

## How to use

Find the most recent date in the "src/RELEASE" folder, and use the files in there.

The most common approach would be to burn this to a disc; in this case, use the *.cue and *.bin files only.

If you have a fx_uploader, you can simply deploy the 'hexview' file through the data link.


### Sample Output:

![Hexview Output](images/hexview.png)


## Development Chain & Tools

This was written using a version of gcc for V810 processor, with 'pcfxtools' which assist in
building executables for PC-FX, and 'liberis' which is a library of functions targetting the PC-FX.

These can be found here:\
![https://github.com/jbrandwood/v810-gcc](https://github.com/jbrandwood/v810-gcc)\
![https://github.com/jbrandwood/pcfxtools](https://github.com/jbrandwood/pcfxtools)\
![https://github.com/jbrandwood/liberis](https://github.com/jbrandwood/liberis)

